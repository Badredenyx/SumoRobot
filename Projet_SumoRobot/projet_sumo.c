#include <xc.h>
#include "pragma.h"
#include <stdio.h>

#define _XTAL_FREQ 10000000 //Constante qui indique la fr�quence d'horloge CPU.
//Doit �tre d�clar�e pour utiliser les fonctions __delay_us() ...
#define FPWM                30000
#define TIMER2_PRESC        1
#define BAUD_RATE       9600

/**
 * Fonction rempla�ant printf qui ne fonctionne pas
 * @param text : cha�ne de caract�res � envoyer
 */
void print(char *text) {
    int i = 0;

    while (text[i] != '\0') { // \0 indique la fin de la chaine de caract�res
        putch(text[i]); //envoie chaque lettre du mot
        i++;
        __delay_ms(50); //bricolage : delay n�cessaire pour que �a fonctionne
    }
}

/**
 * Initialisation de l'ADC utilis� par les capteurs IR de ligne
 */
void init_adc(void) {
    TRISAbits.RA0 = 1; //init capteur IR
    TRISAbits.RA1 = 1; //init capteur IR
    TRISAbits.RA2 = 1; //init capteur IR
    TRISAbits.RA3 = 1; //init capteur IR
    TRISAbits.RA4 = 1; //init capteur IR

    ADCON2bits.ADFM = 1; //justification � droite.
    ADCON1bits.VCFG0 = 0; // VDD as ref.
    ADCON1bits.VCFG1 = 0; // VSS as Ref.
    ADCON1bits.PCFG = 0b1010; // AN0->AN4 s�lectionn�es.
    ADCON2bits.ADCS = 0b101; // Tad --> Fosc/16.
    ADCON2bits.ACQT = 0b001; // 2Tad
    ADCON0bits.ADON = 1; // ADC enabled.
}

/**
 * Initialisation des entr�es tout ou rien utilis�es par les capteurs IR de face
 */
void init_tor(void) {
    TRISDbits.RD5 = 1; //init capteur avant TOR
    TRISDbits.RD6 = 1; //init capteur avant TOR
}

/**
 * Initialisation du PWM utilis� pour faire varier la vitesse des moteurs
 */
void init_pwm(void) {
    CCP1CONbits.CCP1M = 0b1100; //Configuration en mode PWM
    CCP2CONbits.CCP2M = 0b1100; //Configuration en mode PWM
}

/**
 * Initialisation des moteurs
 */
void init_moteur(void) {
    TRISCbits.RC0 = 0; //sens moteur gauche en sortie
    TRISCbits.RC5 = 0; //sens moteur droit en sortie
    PORTCbits.RC0 = 1; //sens moteur gauche : en avant
    PORTCbits.RC1 = 1; //Moteur gauche activ�
    PORTCbits.RC5 = 1; //sens moteur droit : en avant
    PORTCbits.RC2 = 1; //Moteur droit activ� 
}

/**
 * Initilisation du Timer2 permettant de contr�ler le PWM
 * @param PR2_value
 */
void init_timer2(unsigned char PR2_value) {
    T2CONbits.TOUTPS = 0b000; //postscaler = 1:1
    T2CONbits.T2CKPS = 0b00; //precaler � 1:1

    PR2 = PR2_value;

    T2CONbits.TMR2ON = 1; // Timer2 ON
}

/**
 * Initilisation de la liaison s�rie permettant la communication entre l'ordinateur et le robot
 */
void init_LS(void) {
    TRISCbits.RC6 = 0; // TX en sortie.
    TRISCbits.RC7 = 1; // RX en entr�e.    
    TRISBbits.RB5 = 0; // RTS en sortie
    TRISBbits.RB4 = 1; // CTS en entr�e
    TRISBbits.RB3 = 0; // PD en sortie

    PORTBbits.RB5 = 0; // RTS initialis� � 0. indique au module RF que le PIC est pr�t 
    PORTBbits.RB3 = 1; // PD initialis� � 1 pour activer RF.

    for (int i = 0; i < 20; i++)
        __delay_ms(50); //attente apr�s init

    TXSTAbits.TX9 = 0; //8bits 
    TXSTAbits.SYNC = 0; //Mode asynchrone
    TXSTAbits.BRGH = 1; //High speed
    TXSTAbits.TXEN = 1; //Transmit enabled

    RCSTAbits.RX9 = 0; //8bits
    RCSTAbits.SPEN = 1; //Serial port enabled
    RCSTAbits.CREN = 1; //Continuous receive enabled
    
    //Init. de la vitesse de transmission � 9600bauds
    SPBRG = (unsigned char) (-1 + (float) _XTAL_FREQ / (16.0 * (float) BAUD_RATE));
}

/**
 * Invoqu�e par printf.
 * Envoie le caract�re pass� en param. sur la liaison s�rie.
 * @param c Caract�re envoy� sur la liaison s�rie.
 */
void putch(unsigned char c) {
    while (PORTBbits.RB4 == 1); //test si RF pr�t
    while (!TXSTAbits.TRMT); //test du bit TMRT de txstat
    TXREG = c; //Ecriture dans le registre de transmission
}

/**
 * Retourne le caract�re re�u sur la liaison s�rie.
 * Fonction bloquante.
 * @return le caract�re re�u sur Rx
 */
unsigned char getch(void) {
    char car;
    // R�ception d'un caract�re.
    while (PIR1bits.RCIF == 0); //Caract�re non re�u, attendre ?
    car = RCREG; //Ecriture dans le registre de r�ception
    PIR1bits.RCIF = 0; //remise � 0 du bit
    return car;
}

/**
 * Fonction permettant d'optenir l'�tat des capteurs infrarouges de ligne
 * @return la valeur convertie par l'ADC
 */
unsigned int* read_adc(void) {
    int val[5];
    for (int i = 0; i < 5; i++) {
        __delay_us(20);
        ADCON0bits.CHS = i; // AN0 selectionn�.
        ADCON0bits.GO = 1; //d�marre la conversion.
        while (ADCON0bits.GO != 0); // Attente fin de conversion.
        val[i] = ((unsigned int) ADRESH << 8) + (unsigned int) ADRESL; //val prend les 8 bits de poids fort de la conversion
    }
    return val;
}

/**
 * Configuration de la vitesse du moteur gauche
 * @param v vitesse voulue
 */
void vitesse_moteur_gauche(int v) {
    if (v < 0) {
        PORTCbits.RC5 = 0; //reculer
        v = -v;
    } else
        PORTCbits.RC5 = 1; //avancer

    // Init. de CCPR1L:CCP1CON<5:4>. Fixe le temps de niveau haut.
    CCP1CONbits.CCP1X = (unsigned) ((v & 0x0002) != 0); // Affectation du Bit5 de CCP1CON
    CCP1CONbits.CCP1Y = (unsigned) ((v & 0x0001) != 0); // Affectation du Bit4 de CCP1CON
    CCPR1L = (unsigned char) (v >> 2); // Affectation de CCPR1L.
}

/**
 * Configuration de la vitesse du moteur droit
 * @param v vitesse voulue
 */
void vitesse_moteur_droit(int v) {
    if (v < 0) {
        PORTCbits.RC0 = 0; //reculer
        v = -v;
    } else
        PORTCbits.RC0 = 1; //avancer
    // Init. de CCPR2L:CCP2CON<5:4>. Fixe le temps de niveau haut.
    CCP2CONbits.CCP2X = (unsigned) ((v & 0x0002) != 0); // Affectation du Bit5 de CCP2CON
    CCP2CONbits.CCP2Y = (unsigned) ((v & 0x0001) != 0); // Affectation du Bit4 de CCP2CON
    CCPR2L = (unsigned char) (v >> 2); // Affectation de CCPR2L.
}

/**
 * Fontion renvoyant l'�tat des capteurs IR TOR
 * @return true si obstacle d�tect� par les 2 capteurs
 */
int detection_obstacle() {
    return PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0;
}

/**
 * Fontion renvoyant l'�tat des capteurs IR TOR
 * @return true si obstacle d�tect� par le capteur gauche
 */
int detection_obstacle_g() {
    return PORTDbits.RD5 == 0 && PORTDbits.RD6 == 1;
}

/**
 * Fontion renvoyant l'�tat des capteurs IR TOR
 * @return true si obstacle d�tect� par le capteur droit
 */
int detection_obstacle_d() {
    return PORTDbits.RD5 == 1 && PORTDbits.RD6 == 0;
}

/**
 * Mode de course en ligne
 */
void mode_course() {
    //VALEURS DES CAPTEURS DE LIGNES mises dans un tableau
    int adcValue[5];
    int *p = read_adc();
    adcValue[0] = *(p + 0);
    adcValue[1] = *(p + 1);
    adcValue[2] = *(p + 2);
    adcValue[3] = *(p + 3);
    adcValue[4] = *(p + 4);

    //Si le capteur du milieu d�tecte du noir, le robot va tout droit
    if ((adcValue[2] > 200 )) {
        vitesse_moteur_gauche(400);
        vitesse_moteur_droit(400);
    //Sinon si les deux capteurs d'extr�mit�s d�tectent du blanc, le robot tourne dans le sens adapt�
    } else if (adcValue[0] > 200 && adcValue[1] > 200) {
        vitesse_moteur_gauche(400);
        vitesse_moteur_droit(-400);
    } else if (adcValue[3] > 200 && adcValue[4] > 200) {
        vitesse_moteur_gauche(-400);
        vitesse_moteur_droit(400);
    }
}

/**
 * Avant match du mode SUMO (le robot quitte le Dohyo puis y revient)
 */
void init_sumo() {
    for (int i = 0; i < 100; i++)
        __delay_ms(50); //Attente de 5s avant d�but
    vitesse_moteur_gauche(200);
    vitesse_moteur_droit(200);
    for (int i = 0; i < 25; i++)
        __delay_ms(50); //Avance pendant plus d'1s
    vitesse_moteur_gauche(-300);
    vitesse_moteur_droit(300);
    for (int i = 0; i < 10; i++)
        __delay_ms(50); //Demi-tour
    vitesse_moteur_gauche(200);
    vitesse_moteur_droit(200);
    for (int i = 0; i < 25; i++)
        __delay_ms(50); //Avance pour revenir dans l'ar�ne
}

/**
 * Mode SUMO
 */
void mode_sumo() {
    //VALEURS DES CAPTEURS DE LIGNES mises dans un tableau
    int adcValue[5];
    int *p = read_adc();
    adcValue[0] = *(p + 0);
    adcValue[1] = *(p + 1);
    adcValue[2] = *(p + 2);
    adcValue[3] = *(p + 3);
    adcValue[4] = *(p + 4);

    //PENDANT MATCH
    //Si le robot d�tecte la ligne blanche � droite, il effectue un demi-tour � gauche
    if (adcValue[0] < 100 || adcValue[1] < 100 || adcValue[2] < 100) {
        vitesse_moteur_gauche(-350);
        vitesse_moteur_droit(350);
        for (int i = 0; i < 8; i++)
            __delay_ms(50);
    } 
    //Si le robot d�tecte la ligne blanche � gauche, il effectue un demi-tour � droite
    else if (adcValue[3] < 100 || adcValue[4] < 100) {
        vitesse_moteur_gauche(350);
        vitesse_moteur_droit(-350);
        for (int i = 0; i < 8; i++)
            __delay_ms(50);
    //Sinon il avance dans l'ar�ne
    } else {
        if (detection_obstacle()) { //adversaire face � lui : acc�l�ration maximale
            vitesse_moteur_gauche(1000);
            vitesse_moteur_droit(1000);
        } else if (detection_obstacle_g()) { //se tourne pour se mettre face � l'adversaire
            vitesse_moteur_gauche(1000);
            vitesse_moteur_droit(-300);
        } else if (detection_obstacle_d()) { //se tourne pour se mettre face � l'adversaire
            vitesse_moteur_gauche(-300);
            vitesse_moteur_droit(1000);
        } else { //Sinon le robot parcourt l'ar�ne
            vitesse_moteur_gauche(230);
            vitesse_moteur_droit(230);
        }
    }
}

void main(void) {
    TRISC = 0;
    //INITIALISATIONS
    init_LS();
    init_tor();
    init_adc();
    init_pwm();
    init_moteur();
    
    //Calcul de la valeur � charger dans le registre PR2
    unsigned int PR2_value = ((float) _XTAL_FREQ / (4 * (float) FPWM * (float) TIMER2_PRESC)) - 1;
    init_timer2(PR2_value); //initialisation du Timer2

    vitesse_moteur_gauche(0); //initalisation des vitesses moteur � 0
    vitesse_moteur_droit(0);

    int sumo = 0; //variable permettant d'activer le mode sumo
    int course = 0; //variable permettant d'activer le mode course
    
    print("Robot PRET !\n\r"); //envoi de la ligne � afficher � putty
    
    while (!sumo && !course) { //tant qu'aucun mode n'est activ�
        char lettre = getch(); //la variable lettre r�cup�re la lettre tap�e sur Putty
        if (lettre == 'c') {
            print("Mode course active\n\r"); //envoi de la ligne � afficher � putty
            course = 1;
        } else if (lettre == 's') {
            init_sumo();
            print("Mode SUMO active !\n\r"); //envoi de la ligne � afficher � putty
            sumo = 1;
        }
    }
    while (1) {
        if (sumo)
            mode_sumo(); //lancement du mode sumo
        else if (course)
            mode_course(); //lancement du mode course en ligne
    };
    //Impossibilit� de changer de mode apr�s le lancement d'un des deux modes de jeu sans �teindre le robot
}

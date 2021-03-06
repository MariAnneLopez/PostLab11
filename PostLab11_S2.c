/* 
 * File:   PostLab11_S2.c
 * Author: Marian L?pez
 *
 * Created on 14 de mayo de 2022, 10:43 AM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 250000

// VARIABLES
int pot = 0;
int pot_old = 0;
int CCPR = 0;

// PROTORIPO DE FUNCIONES
void setup(void);
unsigned short map(int val, int i_min, int i_max, short o_min, short o_max);

// INTERRUPCIONES
void __interrupt() isr (void){
    if(PIR1bits.SSPIF){         // Fue int. de SPI?
        pot = SSPBUF;           // Guardar dato recibido
        PIR1bits.SSPIF = 0;     // Limpiar bandera de int.
    }
    return;
}

// CICLO PRINCIPAL
int main(void) {
    setup();
    while(1){   
        if (pot_old != pot){                 // Si cambia valor
            CCPR = map(pot, 0, 255, 14, 33); // Interpolaci?n de datos
            CCPR1L = (int)(CCPR>>2);         // 6 bits a reg CCPR1L
            CCP1CONbits.DC1B = CCPR & 0b11;  // 2 bits a DC1B
            pot_old = pot;                   // Actualizar valor
        }
        
    }
}

// CONFIGURACI?N
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    TRISAbits.TRISA5 = 1;       // Selector de slave
    
    // Configuraci?n reloj interno
    OSCCONbits.IRCF = 0b010;    // 250 kHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // SPI slave
    TRISCbits.TRISC3 = 1;       // SCK entrada
    TRISCbits.TRISC4 = 1;       // SDI entrada
    TRISCbits.TRISC5 = 0;       // SDO salida
    PORTC = 0;

    // SSPCON <5:0>
    SSPCONbits.SSPM = 0b0100;   // SPI slave, reloj = SCK pin, SS pin control
    SSPCONbits.CKP = 0;         // Polaridad = Reloj inactivo en 0 (coincidir con master)
    SSPCONbits.SSPEN = 1;       // Habilitar pines de SPI
    // SSPSTAT<7:6>
    SSPSTATbits.CKE = 1;        // Dato enviado a cada flanco positivo del reloj (coincidir con master)
    SSPSTATbits.SMP = 0;        // Se lee al final del periodo del reloj (modo slave siempre en 0)
        
    // Configuraci?n PWM (segun manual)
    TRISCbits.TRISC2 = 1;       // Pin CCP1 como entrada
    PR2 = 38;                   // Per?odo de 20 ms
    
    // Configuraci?n CCP
    CCP1CON = 0;                // CCP apagado
    CCP1CONbits.P1M = 0;        // Modo salida simple
    CCP1CONbits.CCP1M = 0b1100; // PWM
    
    CCPR1L = 31>>2;             // Valor inicial 2 ms ancho de pulso
    CCP1CONbits.DC1B = 31 & 0b11;
    
    // Configuraci?n TMR2
    PIR1bits.TMR2IF = 0;        // Bandera int. TMR2 apagada
    T2CONbits.T2CKPS = 0b11;    // Prescaler 1:16
    T2CONbits.TMR2ON = 1;       // TMR2 ON
    while(!PIR1bits.TMR2IF);    // Esperar un cliclo del TMR2
    PIR1bits.TMR2IF = 0;        // Bandera int. TMR2 apagada
    
    TRISCbits.TRISC2 = 0;       // CCP1 como salida
    
    // Interrupciones
    PIR1bits.SSPIF = 0;         // Bandera int. recepci?n en 0
    PIE1bits.SSPIE = 1;         // Int. de SPI
    INTCONbits.PEIE = 1;        // Int. perif?ricos
    INTCONbits.GIE = 1;         // Int. globales    
}

// Interpolaci?n de datos
unsigned short map(int x, int x0, int x1, short y0, short y1){
    return (unsigned short)(y0 + ((float)(y1-y0)/(x1-x0)) * (x-x0));
}

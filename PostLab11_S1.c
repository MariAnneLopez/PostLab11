/* 
 * File:   PostLab11_S1.c
 * Author: Marian López
 *
 * Created on 14 de mayo de 2022, 11:01 AM
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
int signal = 0;
int cont = 0;

// PROTORIPO DE FUNCIONES
void setup(void);

// INTERRUPCIONES
void __interrupt() isr (void){
    if(PIR1bits.SSPIF){         // Fue int. de SPI?
        signal = SSPBUF;        // Ya se puede transmitir información
        SSPBUF = cont;          // Transmite información
        PIR1bits.SSPIF = 0;     // Limpiar bandera de int.
    }
    return;
}

// CICLO PRINCIPAL
int main(void) {
    setup();
    while(1){   
        if (PORTBbits.RB0){
            while(PORTBbits.RB0){
            }
            cont++;
        }
        if (PORTBbits.RB1){
            while(PORTBbits.RB1){
            }
            cont--;
        }
    }
}

// CONFIGURACIÓN
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    TRISAbits.TRISA5 = 1;       // Selector de slave
    TRISBbits.TRISB0 = 1;       // Botones
    TRISBbits.TRISB1 = 1;

    // Configuración reloj interno
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
       
    // Interrupciones
    PIR1bits.SSPIF = 0;         // Bandera int. recepción en 0
    PIE1bits.SSPIE = 1;         // Int. de SPI
    INTCONbits.PEIE = 1;        // Int. periféricos
    INTCONbits.GIE = 1;         // Int. globales    
}


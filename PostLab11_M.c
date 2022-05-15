/* 
 * File:   PostLab11_M.c
 * Author: Marian López
 *
 * Created on 14 de mayo de 2022, 10:35 AM
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
int cont = 0;
int pot = 0;

// PROTORIPO DE FUNCIONES
void setup(void);
int spi_read_data();
unsigned char SPI_Read(void);
int spi_write_data(int data);
void SPI_Write(unsigned char dato);

// INTERRUPCIONES
void __interrupt() isr (void){
    if(PIR1bits.ADIF){            // Fue int. ADC?
        if (ADCON0bits.CHS == 0){ // Fue int. AN1?
            pot = ADRESH;         // Byte mas significativo
        }
        PIR1bits.ADIF = 0;        // Limpiar bandera int. ADC
    }
    return;
}

// CICLO PRINCIPAL
int main(void) {
    setup();
    while(1){
        spi_write_data(pot);    // Escribe SPI
        cont = spi_read_data(); // Lee SPI
        PORTD = cont;
        if(ADCON0bits.GO == 0){ // Si no hay proceso de conversión
            ADCON0bits.GO = 1;  // Inicia la conversión
        }
    }
}

// CONFIGURACIÓN
void setup(void){
    ANSEL = 1;                  // AN0 analógico
    ANSELH = 0;
    
    TRISAbits.TRISA0 = 1;       // Potenciómetro
    TRISD = 0;
    PORTD = 0;

    // Configuración reloj interno
    OSCCONbits.IRCF = 0b010;    // 250 kHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // SPI master
    TRISC = 0b00010000;         // SDI entrada, SCK y SD0 como salida
    PORTC = 0;
    
    // SSPCON <5:0>
    SSPCONbits.SSPM = 0b0000;   // SPI master, reloj = Fosc/4
    SSPCONbits.CKP = 0;         // Polaridad = Reloj inactivo en 0
    SSPCONbits.SSPEN = 1;       // Habilitar pines de SPI
    // SSPSTAT<7:6>
    SSPSTATbits.CKE = 1;        // Dato enviado a cada flanco positivo del reloj
    SSPSTATbits.SMP = 1;        // Se lee al final del periodo del reloj
    SSPBUF = 0;
        
    // Configuraciones ADC
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON1bits.VCFG0 = 0;       // VDD *Referencias internas
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0;         // Seleccionar AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitar modulo ADC
    __delay_us(40);             
    
    // Configuraciones de interrupciones
    PIR1bits.ADIF = 0;          // Limpiar bandera de int. ADC
    PIE1bits.ADIE = 1;          // Habilitar int. ADC
    INTCONbits.PEIE = 1;        // Habilitar int. de periféricos
    INTCONbits.GIE = 1;         // Habilitar int. globales
    
    return;
}

// Recibir datos del slave
int spi_read_data(){
    PORTCbits.RC1 = 0;          // Habilita slave
    SPI_Write(0x56);
    PORTCbits.RC1 = 1;          // Deshabilita slave
    __delay_us(100);
    
    PORTCbits.RC1 = 0;          // Habilita slave
    int data_rcv = SPI_Read();
    PORTCbits.RC1 = 1;          // Deshabilita slave
    return (data_rcv);
}

// Leer el dato del buffer
unsigned char SPI_Read(){
	while(SSPSTATbits.BF == 0); // Espera a que se llene el buffer
    return SSPBUF;
}

// Transmitir datos al slave
int spi_write_data(int data){
    PORTCbits.RC2 = 0;          // Habilita slave
    SPI_Write(data);
    PORTCbits.RC2 = 1;          // Deshabilita slave
}

// Enivar el dato al buffer
void SPI_Write(unsigned char dato){
    SSPBUF = dato;
    while(SSPSTATbits.BF == 0);
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}


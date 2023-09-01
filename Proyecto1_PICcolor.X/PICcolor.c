/*	MASTER//MASTER//MASTER//MASTER 
 * File:   main.c
 * Author: Bryan
 * Pre I2C Master
 * Created on 8 de agosto de 2023, 10:32 AM
 */
//*****************************************************************************
// Palabra de configuración
//*****************************************************************************
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
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

//*****************************************************************************
// Definición e importación de librerías
//*****************************************************************************
#include <stdint.h>
#include <pic16f887.h>
#include "I2C.h"
#include "LCD8B.h"
#include <xc.h>
#include <pic16f887.h>
//*****************************************************************************
// Definición de variables
//*****************************************************************************
#define _XTAL_FREQ 8000000
#define RS RE1
#define EN RE2
#define RGBADDR 0x52
#define ReadBit 0x01
#define WriteBit 0x00
#define CMDBit 0x80     //10000000
#define AutoInc 0x20    //00100000
#define periodoServo 470
#define _tmr0_value 236

uint8_t adc_val;
uint8_t REDL=0, REDH=0, GREENL=0, GREENH=0, BLUEL=0, BLUEH=0;
//uint8_t escritura=0, selector=0, contador=0;
uint16_t REDlong=0, GREENlong=0, BLUElong=0; //valor 0-2^16
uint8_t RED=0, GREEN=0, BLUE=0;   //valor 0-255
char CharR[3],CharG[3],CharB[3]; // de unidad hasta miles y con dos decimales 6543.21
uint16_t peso; 
uint8_t distance;         //variables de lectura del I2C
char tempChar[4]; // de unidad hasta centena y con um decimal 543.2

unsigned int S1_periodo = 10;//47max  //El periodo del servo puede ser de 10 a 53
unsigned int S2_periodo = 10;//45 max
unsigned int S3_periodo = 0;
unsigned int tmr0count = 0; //cuenta interrupciones del TMR0
uint8_t activadorS2=0, k=0, L=0;
int color;


//*****************************************************************************
// Definición de funciones para que se puedan colocar después del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************
void setup(void);
uint8_t map(uint16_t value, uint8_t fromLow, uint16_t fromHigh, uint8_t toLow, uint8_t toHigh);
//*****************************************************************************
//INTERRUPCION
void __interrupt() isr (void){
    if(T0IF)
    {
        tmr0count++; //INCREMETAR PUERTO
        if (tmr0count == S1_periodo){
            PORTAbits.RA0 = 0;
        }
        if (tmr0count == S2_periodo){
            PORTAbits.RA1 = 0;
        }
        if (tmr0count == S3_periodo){
            PORTAbits.RA2 = 0;
        }
        if (tmr0count == periodoServo){
            PORTAbits.RA0 = 1;
            PORTAbits.RA1 = 1;
            PORTAbits.RA2 = 1;
            tmr0count = 0;
        }
        
        TMR0 = _tmr0_value; // se le vuelve a cargar el valor al tmr0
        T0IF = 0; //se apaga la bandera
    }
 }
//*****************************************************************************
// Main
//*****************************************************************************
void main(void){
    setup();
    //escritura enable para tcs43725
    I2C_Master_Start();
    I2C_Master_Write(RGBADDR | WriteBit);
    I2C_Master_Write( CMDBit | 0x00);
    I2C_Master_Write(0b00010011);
    I2C_Master_Stop();
    
    __delay_ms(10);
    Lcd_Clear();
    uint16_t j=0;
    char charj=0;
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Starting");
    __delay_ms(250);
    Lcd_Clear();
    
    //Loop infinito
    while(1){
        PORTAbits.RA3=1;
        //if(activadorS2=0){
          //  S2_periodo = 10;
        //}
        I2C_Master_Start();         //Conexion con PIC ultrasonico
        I2C_Master_Write(0x21);     //Direccion y modo lectura
        distance = I2C_Master_Read(0);//lectura en variable sin aknowledge
        I2C_Master_Stop();          //Fin conexion
        
        if(distance>=160){
            S1_periodo = 47; 
            activadorS2 = 1;
            //S2_periodo = 45;
        }
        
        
        //si peso es tanto, servo 1 activado
        
        I2C_Master_Start();         //Conexion con PIC PESADOR
        I2C_Master_Write(0x51);     //Direccion y modo lectura
        peso = I2C_Master_Read(0);//lectura en variable sin aknowledge
        I2C_Master_Stop();          //Fin conexion
        //si peso cumple, puedo leer color, sino servo 3 modo descarte
        
        //Lectura de ROJO BAJO y ROJO ALTO con autoincremento
        I2C_Master_Start();         //Comienzo conexion RTC
        I2C_Master_Write(RGBADDR | WriteBit);     //Hablo a la direccion del DS34725
        I2C_Master_Write( CMDBit |AutoInc | 0x14);     //Hablo a la direccion de RED LOW
        I2C_Master_RepeatedStart();         //Retomo conexion
        I2C_Master_Write(RGBADDR | ReadBit);     //Hablo al DS34725 en modo lectura
        REDL = I2C_Master_Read(1);//Leo segundos
        REDH = I2C_Master_Read(1);//Leo segundos
        REDL = I2C_Master_Read(1);//Leo segundos
        REDH = I2C_Master_Read(0);//Leo segundos
        I2C_Master_Stop(); 
        
        //Lectura de VERDE BAJO y VERDE ALTO con autoincremento
        I2C_Master_Start();         //Comienzo conexion RTC
        I2C_Master_Write(RGBADDR | WriteBit);     //Hablo a la direccion del DS34725
        I2C_Master_Write( CMDBit |AutoInc | 0x18);     //Hablo a la direccion de RED LOW
        I2C_Master_RepeatedStart();         //Retomo conexion
        I2C_Master_Write(RGBADDR | ReadBit);     //Hablo al DS34725 en modo lectura
        GREENL = I2C_Master_Read(1);//Leo segundos
        GREENH = I2C_Master_Read(0);//Leo segundos
        I2C_Master_Stop();          //Termino conexion DS43725
        
        //Lectura de AZUL BAJO y AZUL ALTO con autoincremento
        I2C_Master_Start();         //Comienzo conexion RTC
        I2C_Master_Write(RGBADDR | WriteBit);     //Hablo a la direccion del DS34725
        I2C_Master_Write( CMDBit |AutoInc | 0x1A);     //Hablo a la direccion de RED LOW
        I2C_Master_RepeatedStart();         //Retomo conexion
        I2C_Master_Write(RGBADDR | ReadBit);     //Hablo al DS34725 en modo lectura
        BLUEL = I2C_Master_Read(1);//Leo segundos
        BLUEH = I2C_Master_Read(0);//Leo segundos
        
        
        //Separacion en caracteres para mostrar en la pantalla.
        CharR[0] = REDH / 100 + 48; // centena
        CharR[1] = (REDH / 10) % 10 + 48;   // decena
        CharR[2] = REDH % 10 + 48;          // unidad

        CharG[0] = GREENH  / 100 + 48; // centena
        CharG[1] = (GREENH / 10) % 10 + 48;   // decena
        CharG[2] = GREENH % 10 + 48;          // unidad

        CharB[0] = BLUEH / 100 + 48; // centena
        CharB[1] = (BLUEH / 10) % 10 + 48;   // decena
        CharB[2] = BLUEH % 10 + 48;          // unidad
        
        /*//Despliegue en pantalla
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String("R");
        Lcd_Set_Cursor(2,1);
        Lcd_Write_Char(CharR[0]);
        Lcd_Write_Char(CharR[1]);
        Lcd_Write_Char(CharR[2]);
        
        Lcd_Set_Cursor(1,5);
        Lcd_Write_String("G");
        Lcd_Set_Cursor(2,5);
        Lcd_Write_Char(CharG[0]);
        Lcd_Write_Char(CharG[1]);
        Lcd_Write_Char(CharG[2]);
        
        Lcd_Set_Cursor(1,10);
        Lcd_Write_String("B");
        Lcd_Set_Cursor(2,10);
        Lcd_Write_Char(CharB[0]);
        Lcd_Write_Char(CharB[1]);
        Lcd_Write_Char(CharB[2]);
        */
        
        //Lcd_Clear();
        Lcd_Set_Cursor(1,1);
        
        
        if (REDH > GREENH && REDH > BLUEH) {
            Lcd_Write_String("COLOR R");
            color=0; //rojo
            //mover servo posicion 1
        } else if (GREENH > REDH && GREENH > BLUEH) {
            Lcd_Write_String("COLOR V");
            color=1; //verde
            //mover servo posicion 2
        } else if (BLUEH > REDH && BLUEH > GREENH) {
            Lcd_Write_String("COLOR A");
            color=2;//amarillo
            //mover servo posicion 3
        }
       
        //para clasificar color
        if(color==0){ //color rojo
            S3_periodo = 15;
        }
        else if(color==1){ //color verde
            S3_periodo = 25;
        }
        else if(color==2){ //color azul
            S3_periodo = 35;
        }
        
        tempChar[0] = peso/1000 + 48;           //centena calculador como mil
        tempChar[1] = peso%1000/100 + 48;       //decena calculador como centena
        tempChar[2] = peso%100/10 + 48;         //unidad calculado como decena
        tempChar[3] = peso%10 + 48;             //decima calculado como unidad
        Lcd_Set_Cursor(2,0);
        Lcd_Write_String("P:");
        Lcd_Write_Char(tempChar[0]);
        Lcd_Write_Char(tempChar[1]);
        Lcd_Write_Char(tempChar[2]);
        Lcd_Write_Char('.');
        Lcd_Write_Char(tempChar[3]);
        
        tempChar[0] = distance/1000 + 48;       //centena calculador como mil
        tempChar[1] = distance%1000/100 + 48;   //decena calculador como centena
        tempChar[2] = distance%100/10 + 48;      //unidad calculado como decena
        tempChar[3] = (distance)%10 + 48;         //decima calculado como unidad
        Lcd_Set_Cursor(2,9);
        Lcd_Write_String("D:");
        Lcd_Write_Char(tempChar[0]);
        Lcd_Write_Char(tempChar[1]);
        Lcd_Write_Char(tempChar[2]);
        Lcd_Write_Char('.');
        Lcd_Write_Char(tempChar[3]);
        
        
        //contador solo para ver velocidad de conversion
        j++;
        L++;
        if(activadorS2==1){
            k++;
        }
        
        if(j==60){
            S1_periodo = 10; 
            //S2_periodo = 10;
            j=0;
        }
        if(k==90){
            S2_periodo = 45; 
            activadorS2=0;
            k=0;
        }
        //if(k==99){
          //  S2_periodo = 10;
            //k=0;
        //}
        if(L==60){
            S2_periodo = 10;
            L=0;
        }
        
        
        Lcd_Set_Cursor(1,13);
        charj = j+48;
        Lcd_Write_Char(charj);
        
        
    }
    return;
}
//*****************************************************************************
// Función de Inicialización
//*****************************************************************************
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    TRISA = 0x00;
    PORTA = 0x00;
    TRISB = 0x0F;
    TRISD = 0;
    TRISC = 0b00010000;
    PORTB = 0;
    PORTD = 0;
    TRISE = 0x00;
    PORTEbits.RE0=0;
    OSCCONbits.IRCF=0b111;
    OSCCONbits.SCS=1;
    Lcd_Init();
    I2C_Master_Init(100000);        // Inicializar Comuncación I2C
    
    //CONF TMR0
    OPTION_REGbits.T0CS = 0;  //bit 5 clock souce select bit FOSC/4
    OPTION_REGbits.PSA =  0;  //bit 6 prescaler is to TMR0 
    OPTION_REGbits.PS =  0b000; //prescaler 1:2
    TMR0 = _tmr0_value ; //valor de tmr0 va a ser 236
    
    //Configuracion de interrupcion del TMr0 el iocb
    INTCONbits.T0IF = 0; //BANDERA DE INTERRUPCION 
    INTCONbits.T0IE = 1;  // periferico
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;  //global
    
    //Configuracion para botones 
    INTCONbits.GIE = 1;         //Interrupcioes globales
    INTCONbits.RBIE = 1;        // Activo interrupción de botón
    INTCONbits.RBIF = 0;        // Apago bandera de interrupción
    OPTION_REGbits.nRBPU = 0;   // Activo pull ups generales
    
    //ESCRITOR
    IOCBbits.IOCB0 = 1;         // INTERRUPCIÓN PORTB activada
    WPUBbits.WPUB0 = 1;         // Activo pull up B0
    //SELECTOR
    IOCBbits.IOCB1 = 1;         // INTERRUPCIÓN PORTB activada
    WPUBbits.WPUB1 = 1;         // Activo pull up B1
    //INC
    IOCBbits.IOCB2 = 1;         // INTERRUPCIÓN PORTB activada
    WPUBbits.WPUB2 = 1;         // Activo pull up B2
    //DEC
    IOCBbits.IOCB3 = 1;         // INTERRUPCIÓN PORTB activada
    WPUBbits.WPUB3 = 1;         // Activo pull up B3
}

uint8_t map(uint16_t value, uint8_t fromLow, uint16_t fromHigh, uint8_t toLow, uint8_t toHigh) {
    return (uint8_t)((uint16_t)(value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow);
}
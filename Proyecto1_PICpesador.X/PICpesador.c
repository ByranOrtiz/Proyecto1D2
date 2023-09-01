/************************************************************************************** 
*   Balanza Digital con PIC + HX711 + Celda de Carga de 5kg                           * 
*                                                                                     *
*   by Sergio Andres Casta?o Giraldo                                                  *
*   Adapted by Bryan Estuardo Ortiz Casimiro
*   website: https://controlautomaticoeducacion.com/                                  *
*   YouTube Chanel: https://www.youtube.com/channel/UCdzSnI03LpBI_8gXJseIDuw          *
**************************************************************************************/ 
//*****************************************************************************
// Palabra de configuración
//*****************************************************************************
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
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

#include <pic16f887.h>
#include <stdint.h>
#include "LCD8B.h"  
#include "HX711.h"
#include <xc.h>
#include "I2C.h"

//*****************************************************************************
// Definición de variables
//*****************************************************************************
#define _XTAL_FREQ 8000000
#define MODO PORTBbits.RB4
#define TARA PORTBbits.RB5
#define dirEEPROM 0x05
float escala = 0;
float peso=0,factor = 1;
uint16_t PesoToGo;
float peso_conocido[4] = {202,462,153,202};
uint8_t z;
int datoX;

//*****************************************************************************
// Definición de funciones para que se puedan colocar después del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************
void setup(void);
void calibration(void);
void writeToEEPROM(uint8_t data, uint8_t address);
uint8_t readFromEEPROM(uint8_t address);
//*****************************************************************************
// Código de Interrupción 
//*****************************************************************************
void __interrupt() isr(void){
   if(PIR1bits.SSPIF == 1){ 

        SSPCONbits.CKP = 0;
       
        if ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)){
            z = SSPBUF;                 // Read the previous value to clear the buffer
            SSPCONbits.SSPOV = 0;       // Clear the overflow flag
            SSPCONbits.WCOL = 0;        // Clear the collision bit
            SSPCONbits.CKP = 1;         // Enables SCL (Clock)
        }

        if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) {             //LEO DE MASTER
            //__delay_us(7);
            z = SSPBUF;                 // Lectura del SSBUF para limpiar el buffer y la bandera BF
            //__delay_us(2);
            PIR1bits.SSPIF = 0;         // Limpia bandera de interrupción recepción/transmisión SSP
            SSPCONbits.CKP = 1;         // Habilita entrada de pulsos de reloj SCL
            while(!SSPSTATbits.BF);     // Esperar a que la recepción se complete
            datoX = SSPBUF;             // Guardar en el datoX el valor del buffer de recepción
            __delay_us(250);
            
        }else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){        //MANDO A MASTER
            z = SSPBUF;
            BF = 0;
            PesoToGo = (int)(peso*10);
            SSPBUF = PesoToGo;
            SSPCONbits.CKP = 1;
            __delay_us(250);
            while(SSPSTATbits.BF);
        }
       
        PIR1bits.SSPIF = 0;    
    }
}
//*****************************************************************************
// Main
//*****************************************************************************

void main(){  
    setup();
    init_hx(128);       //inicio sensor de peso
    Lcd_Clear();
    
    int unidad = 1;
    char tempChar[6]; // de unidad hasta miles y con dos decimales 6543.21
    int calicali=0;
    //Lee el valor de la escala en la EEPROM
    escala = readFromEEPROM(dirEEPROM);     

    //Pregunta si se entra al modo de ajuste y calibraci?n
    if(MODO == 0 && TARA == 0)   
        calibration();

    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Retire el peso");
    Lcd_Set_Cursor(2,1);      
    Lcd_Write_String("Y espere...");
    set_scale(escala);
    tare(10);
    __delay_ms(2000);
    Lcd_Clear();

    Lcd_Set_Cursor(1,1);        
    Lcd_Write_String("Listo...");
    __delay_ms(3000);
    Lcd_Clear();
    tare(10);
    
    //Loop INFINITO
    while(1){
        //Lcd_Clear();
        peso = get_units(10); //Lee el peso y hace un promedio de 10 muestras
        Lcd_Set_Cursor(1,1);         
        Lcd_Write_String("PESO:"); 

        switch (unidad) {
             case 1:        
                factor = 1;
                peso=peso/factor; //es decir peso en gramos
                //int peso_temp = (int)
                //peso = 3193.45;
                tempChar[0] = (int)(peso/1000) + 48;        //unidad de millar
                tempChar[1] = ((int)peso)%1000/100 + 48;    //centena
                tempChar[2] = ((int)peso)/10%10 + 48;       //decena
                tempChar[3] = ((int)peso)%10 + 48;          //unidad
                tempChar[4] = (int)(peso*10)%10 + 48;              //decima
                //tempChar[5] = (int)(peso*100)%10 + 48;             //centesima
                Lcd_Set_Cursor(2,1);
                Lcd_Write_Char(tempChar[0]);
                Lcd_Write_Char(tempChar[1]);
                Lcd_Write_Char(tempChar[2]);
                Lcd_Write_Char(tempChar[3]);
                Lcd_Write_Char('.');
                Lcd_Write_Char(tempChar[4]);
                //Lcd_Write_Char(tempChar[5]);
                Lcd_Write_Char('g'); 
                break;
             case 2:
                factor = 1000.0;  //conversor a kilos
                peso=peso/factor; //es decir peso en kilos

                tempChar[0] = (int)(peso/1000) + 48;        //unidad de millar
                tempChar[1] = ((int)peso)%1000/100 + 48;    //centena
                tempChar[2] = ((int)peso)/10%10 + 48;       //decena
                tempChar[3] = ((int)peso)%10 + 48;          //unidad
                tempChar[4] =   (int)peso*10%10 + 48;              //decima
                //tempChar[5] = ((int)peso)*100%10 + 48;             //centesima
                Lcd_Set_Cursor(2,1);
                Lcd_Write_Char(tempChar[0]);
                Lcd_Write_Char(tempChar[1]);
                Lcd_Write_Char(tempChar[2]);
                Lcd_Write_Char(tempChar[3]);
                Lcd_Write_Char('.');
                Lcd_Write_Char(tempChar[4]);
                //Lcd_Write_Char(tempChar[5]);
                Lcd_Write_Char('K');
                Lcd_Write_Char('g');
                break;
             case 3:
                factor = 28.35;  //conversor a onzas
                peso=peso/factor; //es decir peso en onzas

                tempChar[0] = (int)(peso/1000) + 48;        //unidad de millar
                tempChar[1] = ((int)peso)%1000/100 + 48;    //centena
                tempChar[2] = ((int)peso)/10%10 + 48;       //decena
                tempChar[3] = ((int)peso)%10 + 48;          //unidad
                tempChar[4] = (int)(peso*10)%10 + 48;              //decima
                //tempChar[5] = ((int)peso)*100%10 + 48;             //centesima
                Lcd_Set_Cursor(2,1);
                Lcd_Write_Char(tempChar[0]);
                Lcd_Write_Char(tempChar[1]);
                Lcd_Write_Char(tempChar[2]);
                Lcd_Write_Char(tempChar[3]);
                Lcd_Write_Char('.');
                Lcd_Write_Char(tempChar[4]);
                Lcd_Write_Char(tempChar[5]);
                Lcd_Write_Char('O');
                Lcd_Write_Char('z');
                break;
        }
        if(TARA==0){
             __delay_ms(200);
             tare(10);
        }
        if(MODO==0){
             __delay_ms(200);
             unidad = (unidad>2)? 1:unidad+1;
        }
        __delay_ms(100);

    }

}

//******************************************************************************
//Funciones =)
//******************************************************************************
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    TRISB = 0b00110100;
    TRISD = 0;
    PORTB = 0;
    PORTD = 0;
    TRISE = 0x00;
    PORTEbits.RE0=0;
    OSCCONbits.IRCF=0b111;
    OSCCONbits.SCS=1;
    Lcd_Init();
    I2C_Slave_Init(0x50); 
    
    
    
    //Configuracion para botones 
    INTCONbits.GIE = 1;         //Interrupcioes globales
    INTCONbits.RBIE = 1;        // Activo interrupción de botón
    INTCONbits.RBIF = 0;        // Apago bandera de interrupción
    OPTION_REGbits.nRBPU = 0;   // Activo pull ups generales
    
    //MODO
    IOCBbits.IOCB4 = 1;         // INTERRUPCIÓN PORTB activada
    WPUBbits.WPUB4 = 1;         // Activo pull up B4
    //TARA
    IOCBbits.IOCB5 = 1;         // INTERRUPCIÓN PORTB activada
    WPUBbits.WPUB5 = 1;         // Activo pull up B5
}


void writeToEEPROM(uint8_t data, uint8_t address){
    EEADR = address;
    EEDAT = data;
    
    EECON1bits.EEPGD = 0;       //Escribir a la memoria
    EECON1bits.WREN = 1;        //Habilito escritura en EEPROM WRITE ENABLE     AQUI LE DIGO, TE DOY PERMISO DE ESCRIBIR
    
    INTCONbits.GIE = 0;         //DESHABILITO INTERRUPCIONES TEMPORALMENTE
    
    EECON2 = 0x55;      //Siempre es 55, es confirmacion de escritura
    EECON2 = 0xAA;      
    EECON1bits.WR = 1;          //AQUI LE DIGO ESCRIBI YA
    
    //INTCONbits.GIE = 1;         //VUELVO A HABILITAR INTERRUPCIONES 
    EECON1bits.WREN = 0;        //Deshabilito escritur en EEPROM
}

uint8_t readFromEEPROM(uint8_t address){
    EEADR = address;
    EECON1bits.EEPGD = 0;       //Le digo que accese a la data 
    EECON1bits.RD = 1;          //lE DIGO LEE AHORA
    return EEDAT;               //Poner valor en el dato y regresarlo
}

//Funcion de calibracion y ajuste
void calibration(){
  //__delay_ms(100);
  int i = 0,cal=1;
  uint32_t adc_lecture;
  
  // Escribimos el Mensaje en el LCD
  Lcd_Set_Cursor(1,1);       
  Lcd_Write_String("Calibracion de");
  Lcd_Set_Cursor(2,1);      
  Lcd_Write_String("Balanza");
  __delay_ms(1500);
  //tare(10);  //El peso actual es considerado cero.
  
  Lcd_Clear();

  //Inicia el proceso de ajuste y calibración
  while(cal == 1){
    //__delay_ms(100);
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Peso conocido:");
    //printf(lcd_putc, "%4.0f g             ",peso_conocido[i]);
    char tempChar[6]; // de unidad hasta miles y con dos decimales 6543.21
    tempChar[0] = (int)(peso_conocido[i]/1000) + 48;        //unidad de millar
    tempChar[1] = ((int)peso_conocido[i])%1000/100 + 48;    //centena
    tempChar[2] = ((int)peso_conocido[i])/10%10 + 48;       //decena
    tempChar[3] = ((int)peso_conocido[i])%10 + 48;          //unidad
    tempChar[4] = (int)(peso_conocido[i])*10%10 + 48;              //decima
    tempChar[5] = (int)(peso_conocido[i])*100%10 + 48;             //centesima
    
    Lcd_Set_Cursor(2,1);
    Lcd_Write_Char(tempChar[0]);
    Lcd_Write_Char(tempChar[1]);
    Lcd_Write_Char(tempChar[2]);
    Lcd_Write_Char(tempChar[3]);
    Lcd_Write_Char('.');
    Lcd_Write_Char(tempChar[4]);
    Lcd_Write_Char(tempChar[5]);
    Lcd_Write_Char('g');
    
    
    //Busca el peso conocido con el boton tara   B5
    if(TARA==0){  
      __delay_ms(200); // Anti-rebote
      i =(i>2) ? 0:i+1; //if-else en una linea //verifica que no se pase de 2
      //ya que solo hay 4 casillas de peso conocido de 0 a 3
    }

    //Selecciona el peso conocido con el boton modo     B4
    if(MODO==0){
      __delay_ms(200);
      Lcd_Clear();
      Lcd_Set_Cursor(1,1);      
      Lcd_Write_String("Poner peso");
      Lcd_Set_Cursor(2,1);       
      Lcd_Write_String("Y espere...");
      
      __delay_ms(3000);

      //Lee el valor del HX711
      adc_lecture = get_value(10);      //adc_lecture es lectura del hx711

      //Calcula la escala con el valor leido dividido el peso conocido
      escala = adc_lecture / peso_conocido[i];

      //Guarda la escala en la EEPROM
      writeToEEPROM(escala, dirEEPROM);
      
      __delay_ms(100);
      cal = 0; //Cambia la bandera para salir del while
      Lcd_Clear();
    }
    
  }
}
/************************************************************************************** 
*   hx711.c - communicates with a HX711                                               * 
*                                                                                     *
*   by Sergio Andres Casta?o Giraldo                                                  *
*   Adapted by Bryan Estuardo Ortiz Casimiro                                          *
*   website: https://controlautomaticoeducacion.com/                                  *
*   YouTube Chanel: https://www.youtube.com/channel/UCdzSnI03LpBI_8gXJseIDuw          *
**************************************************************************************/ 

/*#byte porta = 0xf80 // Identificador para el puerto A. 
#byte portb = 0xf81 // Identificador para el puerto B. 
#byte portc = 0xf82 // Identificador para el puerto C. 
#byte portd = 0xf83 // Identificador para el puerto D. 
#byte porte = 0xf84 // Identificador para el puerto E.
*/
#ifndef HX711_H
#define	HX711_H


#ifndef DOUT 
   #define DOUT PORTBbits.RB2            //input - data  
#endif 

#ifndef SCK 
   #define SCK PORTBbits.RB3           // output clock
#endif

#include <xc.h> // include processor files - each processor file is guarded. 

void set_scale(float scale1);

void set_offset(uint32_t offset1);

uint32_t hx711_read(void);

void set_gain(int gain1);


void init_hx(int gain1);

int32_t get_average(int times);

float get_value(int times);

float get_units(int times);

void tare(int times);

void power_down(void);

void power_up(void);

#endif	/* HX711_H */
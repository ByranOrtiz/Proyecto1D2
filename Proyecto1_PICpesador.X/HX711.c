/************************************************************************************** 
*   hx711.c - communicates with a HX711                                               * 
*                                                                                     *
*   by Sergio Andres Casta?o Giraldo                                                  *
*   Adapted by Bryan Estuardo Ortiz Casimiro                                          *
*   website: https://controlautomaticoeducacion.com/                                  *
*   YouTube Chanel: https://www.youtube.com/channel/UCdzSnI03LpBI_8gXJseIDuw          *
**************************************************************************************/ 
#include "HX711.h"
#define _XTAL_FREQ 8000000

uint32_t OFFSET = 0;                // used for tare weight
float SCALE = 1;               // used to return weight in grams, kg, ounces
int   GAIN = 1;                //amplificacion interna del HX711

void set_scale(float scale1){
     SCALE = scale1;
}

void set_offset(uint32_t offset1) {
     OFFSET = offset1;
}

uint32_t hx711_read()              
{ 
   int i; 
   uint32_t data = 0; 

   SCK=0;  
   __delay_ms(100);
   while(DOUT == 1);
   for (i=0;i<24;i++){ 
      SCK = 1; 
      __delay_us(1); 
      data = data << 1;
      __delay_us(1);
      SCK = 0; 
      __delay_us(1);
      if(DOUT) data++;  
   } 
   
   for (i=0;i<GAIN;i++){ 
        SCK = 1;
        __delay_us(1);
        SCK = 0;
        __delay_us(1);
   }
   data = data ^ 0x800000;
   return(data);
} 

void set_gain(int gain1) {
    switch (gain1) {
        case 128:          // channel A, gain factor 128
            GAIN = 1;
            break;
        case 64:          // channel A, gain factor 64
            GAIN = 3;
            break;
        case 32:          // channel B, gain factor 32
            GAIN = 2;
            break;
     }

    SCK = 0;
    hx711_read();
}


void init_hx(int gain1) 
{ 
   SCK = 0;    //Output
   DOUT = 1;  //Input
   set_gain(gain1);
} 

int32_t get_average(int times)              
{
    uint32_t sum = 0;
    for (int i=0;i<times;i++){
         sum = sum + hx711_read();
         //delay_ms(1);
    }
    return(sum / times);
}

float get_value(int times)              
{
    float value;
    uint32_t read = get_average(times);
    if(OFFSET >= read){
         value =  (OFFSET - read);
         value = -1 * value; 
    }
    else     
         value = read - OFFSET;
    return(value);
}

float get_units(int times) {
    return get_value(times) / SCALE;
}

void tare(int times){
    uint32_t value = get_average(times);
    set_offset(value);
}

void power_down(){
    SCK = 0;
    SCK = 1;
}

void power_up(){
    SCK = 0;
}

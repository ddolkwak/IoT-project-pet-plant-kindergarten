#include <stdio.h>
#include <wiringPi.h>

#define SPI_CH 0
#define ADC_CH 0
#define ADC_CS 29
#define SPI_SPEED 500000
// RGB LED //
#define LED_R_PIN 28 // IO20
#define LED_G_PIN 27 // IO16
#define LED_B_PIN 25 // IO26

int main(void){
        int value=0, i;
        unsigned char buf[3];

        if(wiringPiSetup() == -1) return 1;

        if(wiringPiSPISetup() == -1) return -1;

        pinMode(ADC_CS,OUTPUT);
        //// RGB LED
		pinMode(LED_R_PIN, OUTPUT);
		pinMode(LED_G_PIN, OUTPUT);
		pinMode(LED_B_PIN, OUTPUT);

        buf[0] = 0x06 | ((ADC_CH & 0x04)>>2);
        buf[1] = ((ADC_CH & 0x03)<<6);
        buf[2] = 0x00;

        digitalWrite(ADC_CS,0);

        wiringPiSPIDataRW(SPI_CH,buf,3);

        buf[1]=0x0F & buf[1];

        value = (buf[1]<<8) | buf[2];

        digitalWrite(ADC_CS,1);

        printf("%d",value);
        
        if (value < 1600)
			digitalWrite(LED_R_PIN, HIGH);
		
		if (value > 2200)
			digitalWrite(LED_R_PIN, LOW);
}

#include <stdio.h>
#include <wiringPi.h>

#define SPI_CH 0
#define ADC_CH 5
#define ADC_CS 29
#define SPI_SPEED 500000
// DC Motor //
#define DC_MOTOR_A 11 // IO7
#define DC_MOTOR_B 10 // IO8

int main(void){
	int value=0, i;
	unsigned char buf[3];

	if(wiringPiSetup() == -1) return 1;

	if(wiringPiSPISetup() == -1) return -1;

	pinMode(DC_MOTOR_A, OUTPUT);
	pinMode(DC_MOTOR_B, OUTPUT);
	digitalWrite(DC_MOTOR_A, HIGH);
	digitalWrite(DC_MOTOR_B, HIGH);

	pinMode(ADC_CS,OUTPUT);

	buf[0] = 0x06 | ((ADC_CH & 0x04)>>2);
	buf[1] = ((ADC_CH & 0x03)<<6);
	buf[2] = 0x00;

	digitalWrite(ADC_CS,0);

	wiringPiSPIDataRW(SPI_CH,buf,3);

	buf[1]=0x0F & buf[1];

	value=(buf[1] << 8) | buf[2];

	digitalWrite(ADC_CS,1);

	printf("%d", value);
	
	return 0;
}

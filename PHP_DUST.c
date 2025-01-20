#include <stdio.h>
#include <wiringPi.h>

#define SPI_CH 0
#define ADC_CH 3
#define ADC_CS 29
#define SPI_SPEED 500000
#define DUST_ADC_CH 3
#define DUST_D_PIN 4 // IO2
// DC Motor //
#define DC_MOTOR_A 11 // IO7
#define DC_MOTOR_B 10 // IO8


int main(void){
    int d_value = 0, i, max = 0;
	unsigned char buf[3];

	if(wiringPiSetup() == -1) return 1;

	if(wiringPiSPISetup() == -1) return -1;
	
	pinMode(DC_MOTOR_A, OUTPUT);
	pinMode(DC_MOTOR_B, OUTPUT);
	digitalWrite(DC_MOTOR_A, HIGH);
	digitalWrite(DC_MOTOR_B, HIGH);

	//not yet
	pinMode(ADC_CS,OUTPUT);
	pinMode(DUST_D_PIN,OUTPUT);

	for (i = 0; i < 20; i++) {
		digitalWrite(DUST_D_PIN, LOW);
		delayMicroseconds(280);

		buf[0] = 0x06 | ((DUST_D_PIN & 0x04)>>2);
		buf[1] = ((DUST_D_PIN & 0x03) << 6);
		buf[2] = 0x00;

		digitalWrite(ADC_CS,0);

		wiringPiSPIDataRW(SPI_CH, buf, 3);

		buf[1]=0x0F & buf[1];

		d_value = (buf[1] << 8) | buf[2];

		digitalWrite(ADC_CS,1);

		delayMicroseconds(40);
		digitalWrite(DUST_D_PIN,HIGH);
		delayMicroseconds(9680);

		if (d_value > max) max = d_value;
		delay(50);
	}
	printf("%d", max);
	return d_value;
	delay(50);
}


#include <wiringPi.h>
#include <stdio.h>

// DC Motor //
#define DC_MOTOR_A 11 // IO7
#define DC_MOTOR_B 10 // IO8

int main(void){
        if(wiringPiSetup() == -1) return 1;

		pinMode(DC_MOTOR_A, OUTPUT);
		pinMode(DC_MOTOR_B, OUTPUT);

		digitalWrite(DC_MOTOR_A, HIGH);
		digitalWrite(DC_MOTOR_B, LOW);

	printf("1");
}

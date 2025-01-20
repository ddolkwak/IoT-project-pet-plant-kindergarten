#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h> 
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define SPI_CH 0
#define ADC_CS 29
#define SPI_SPEED 500000
#define MAX_TIME 100

/* Analog PIN */
// Dust
#define DUST_ADC_CH 3
// Soil
#define SOIL_ADC_CH 5

/* Digital PIN */
//// Sensor ////
// Dust //
#define DUST_D_PIN 4 // IO2
// DHT11
#define DHT11_PIN 3 // IO22

//// Actuator ////
// LED Module // 
#define LED_MODULE_1_PIN 9 // IO3
#define LED_MODULE_2_PIN 7 // IO4
#define LED_MODULE_3_PIN 0 // IO17
#define LED_MODULE_4_PIN 2 // IO27

// DC Motor //
#define DC_MOTOR_A 11 // IO7
#define DC_MOTOR_B 10 // IO8

typedef struct Environment
{
	int temperature;
	int heatIndexC;
	int farenheit;
	int heatIndexF;
	int humidity;
	int dust;
	int soil;
} env_struct;

int assign_pin();
void *dht(void*);
int ascending(const void*, const void*);
void *dust();
void *soil();
env_struct *set_env(void*, int, int, int, int, int, int, int);
void *led_module();

int isChecking = 0;
int isEnvironmentProper = 0;
int isHumidityNotProper = 0;
int isTemperatureCold = 0;
int isTemperatureHot = 0;
int isDustNotProper = 0;
int isSoilNotProper = 0;

// Threshold to Show
int dustThreshold = 4096;

// dht11
int dht11_val[5][5];
int dht11_temp[5] = {0,0,0,0,0};
float farenheit_temp = 0;

// dust
int dust_val = 0;

// soil
int soil_val = 0;

// env
env_struct env;

int main(void){
	// Wiring Pi Set up
	if (wiringPiSetup() == -1) return 1;
	if (wiringPiSPISetup(SPI_CH, SPI_SPEED) == -1) return -1;
	
	assign_pin();
	turn_off_all();
	
	void *threadReturns;
	
	pthread_t dhtThread;
	pthread_t dustThread;
	pthread_t soilThread;
	pthread_t ledModuleThread;
	
	set_env(&env, 0, 0, 0, 0, 0, 0, 0);
	
	isChecking = 1;
	pthread_create(&ledModuleThread, NULL, led_module, NULL);
	delay(100);
	
	// Init boolean 
	isEnvironmentProper = 1;
	isHumidityNotProper = 0;
	isTemperatureCold = 0;
	isTemperatureHot = 0;
	isDustNotProper = 0;
	isSoilNotProper = 0;
	
	// Check Env during 5 secs every 30 secs
	
	int i;
	for (i=0; i<5; i++)
	{
		pthread_create(&dhtThread, NULL, dht, (void*) &i);
		pthread_join(dhtThread, &threadReturns);
	}
	
	qsort(dht11_val, 5, sizeof(dht11_val[0]), ascending);
	delay(100);
	
	dust_val = 0;
	pthread_create(&dustThread, NULL, dust, NULL);
	pthread_join(dustThread, &threadReturns);
	soil_val = 0;
	pthread_create(&soilThread, NULL, soil, NULL);
	pthread_join(soilThread, &threadReturns);
	
	turn_off_all();
	
	set_env(&env, dht11_val[2][2], dht11_val[2][2], 0, 0, dht11_val[2][0], dust_val, soil_val);
	
	printf("%d %d %d %d", env.temperature, env.humidity, env.soil, env.dust);

	isChecking = 0;
	
	return 0;
}

int assign_pin()
{
	//// Dust Sensor 
	// result of Dust Sensor
	pinMode(DUST_D_PIN, OUTPUT);
	
	//// Soil Moisture Sensor
	
	//// DHT11
	pinMode(DHT11_PIN, OUTPUT);
	
	//// LED Module
	pinMode(LED_MODULE_1_PIN, OUTPUT);
	pinMode(LED_MODULE_2_PIN, OUTPUT);
	pinMode(LED_MODULE_3_PIN, OUTPUT);
	pinMode(LED_MODULE_4_PIN, OUTPUT);
	
	// DC Motor
	pinMode(DC_MOTOR_A, OUTPUT);
	pinMode(DC_MOTOR_B, OUTPUT);
	
	return 0;
}

int turn_off_all()
{
	digitalWrite(DC_MOTOR_A, HIGH);
	digitalWrite(DC_MOTOR_B, HIGH);
	digitalWrite(LED_MODULE_1_PIN, LOW);
	digitalWrite(LED_MODULE_2_PIN, LOW);
	digitalWrite(LED_MODULE_3_PIN, LOW);
	digitalWrite(LED_MODULE_4_PIN, LOW);
	return 0;
}

int ascending(const void* a, const void*b)
{
	if (*(int *)(a+8) > *(int *)(b+8)) return 1;
	else if (*(int *)(a+8) < *(int *)(b+8)) return -1;
	else return 0;
}

//
void *dht(void *_k)
{
	uint8_t lststate = HIGH;
	int counter = 0;
	int k = *(int*)_k;
	int j = 0, i;
	float farenheit;
	
	pinMode(DHT11_PIN, OUTPUT);
	digitalWrite(DHT11_PIN, 0);
	delay(18);
	digitalWrite(DHT11_PIN, 1);
	delayMicroseconds(40);
	pinMode(DHT11_PIN, INPUT);
	
	dht11_val[k][0] = 0;
	dht11_val[k][1] = 0;
	dht11_val[k][2] = 0;
	dht11_val[k][3] = 0;
	dht11_val[k][4] = 0;
	
	for( i=0 ; i<MAX_TIME ; i++) {
		counter =0;
		while(digitalRead(DHT11_PIN) == lststate) {
			counter++;
			delayMicroseconds(1);
			if(counter == 256)
				break;
		}
		lststate = digitalRead(DHT11_PIN);
		if(counter == 256)
			break;
		if((i>=4) && (i%2 ==0)) {
			dht11_val[k][j/8] <<= 1;
								
			if(counter > 26) {
				dht11_val[k][j/8] |= 1;
			}
			j++;
		}
	}

   if((j>=40)&&(dht11_val[k][4] == ((dht11_val[k][0] + dht11_val[k][1] + dht11_val[k][2] + dht11_val[k][3]) & 0xFF))) {
	   farenheit = dht11_val[k][2]*9./5.+32;
	   for(i=0 ; i<5; i++)
		  dht11_temp[i] = dht11_val[k][i];
	   farenheit_temp = farenheit;
   }
   else
   {
	   
   }
   delay(2000);
   pthread_exit(0);
}

//
void *dust()
{
	int value=0, max=0, i;
	unsigned char buf[3];
        
	for(i=0; i<20; i++){
  	    digitalWrite(DUST_D_PIN, LOW);
		delayMicroseconds(280);

   	    buf[0] = 0x06 | ((DUST_ADC_CH & 0x04)>>2);
     	buf[1] = ((DUST_ADC_CH & 0x03)<<6);
    	buf[2] = 0x00;

    	digitalWrite(ADC_CS,0);

		wiringPiSPIDataRW(SPI_CH,buf,3);

		buf[1]=0x0F & buf[1];

		value = (buf[1]<<8) | buf[2];

		digitalWrite(ADC_CS,1);

		delayMicroseconds(40);
		digitalWrite(DUST_D_PIN, HIGH);
		delayMicroseconds(9680);

	    if (value > max) max = value;
	    delay(50);
	}
	delay(50);
	dust_val = max;
	pthread_exit(0);
}

// 
void *soil()
{
	int value=0, sum=0, i;
	unsigned char buf[3];
	
	for(i=0; i<20; i++) {
		buf[0] = 0x06 | ((SOIL_ADC_CH & 0x04)>>2); 
		buf[1] = ((SOIL_ADC_CH & 0x03)<<6); 
		buf[2] = 0x00;

		digitalWrite(ADC_CS, 0);

		wiringPiSPIDataRW(SPI_CH, buf,3);

		buf[1]=0x0F & buf[1];

		value=(buf[1] << 8) | buf[2];

		digitalWrite(ADC_CS,1);

		sum += value;
		delay(50);
	}
	soil_val = sum / 20;
	delay(50);
	pthread_exit(0);
}

env_struct *set_env(void *arg, int _c1, int _c2, int _f1, int _f2, int _h, int _d, int _s)
{
	env_struct *_env = (env_struct*) arg;
	_env->temperature = _c1;
	_env->heatIndexC = _c2;
	_env->farenheit = _f1;
	_env->heatIndexF = _f2;
	_env->humidity = _h;
	_env->dust = _d;
	_env->soil = _s;
	
	return _env;
}

void *led_module()
{
	while (isChecking)
	{
		digitalWrite(LED_MODULE_1_PIN, HIGH);
		digitalWrite(LED_MODULE_2_PIN, LOW);
		digitalWrite(LED_MODULE_3_PIN, LOW);
		digitalWrite(LED_MODULE_4_PIN, LOW);
		delay(100);
		digitalWrite(LED_MODULE_1_PIN, LOW);
		digitalWrite(LED_MODULE_2_PIN, HIGH);
		digitalWrite(LED_MODULE_3_PIN, LOW);
		digitalWrite(LED_MODULE_4_PIN, LOW);
		delay(100);
		digitalWrite(LED_MODULE_1_PIN, LOW);
		digitalWrite(LED_MODULE_2_PIN, LOW);
		digitalWrite(LED_MODULE_3_PIN, HIGH);
		digitalWrite(LED_MODULE_4_PIN, LOW);
		delay(100);
		digitalWrite(LED_MODULE_1_PIN, LOW);
		digitalWrite(LED_MODULE_2_PIN, LOW);
		digitalWrite(LED_MODULE_3_PIN, LOW);
		digitalWrite(LED_MODULE_4_PIN, HIGH);
		delay(100);
	}
	digitalWrite(LED_MODULE_1_PIN, LOW);
	digitalWrite(LED_MODULE_2_PIN, LOW);
	digitalWrite(LED_MODULE_3_PIN, LOW);
	digitalWrite(LED_MODULE_4_PIN, LOW);
	pthread_exit(0);
}

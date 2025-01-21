// IoT project : pet-plant kindergarten(230303~230628)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> //Prettier로 format Documentation했더니 #include 오류 발생
#include <pthread.h> //Prettier로 format Documentation했더니 #include 오류 발생
#include <string.h> 
#include <wiringPi.h> //Prettier로 format Documentation했더니 #include 오류 발생
#include <wiringPiSPI.h> //Prettier로 format Documentation했더니 #include 오류 발생
#include "bt_master.h" //Prettier로 format Documentation했더니 #include 오류 발생
#include <sys/socket.h> //Prettier로 format Documentation했더니 #include 오류 발생
#include <netinet/in.h> //Prettier로 format Documentation했더니 #include 오류 발생

#define PORT 8081

#define SPI_CH 0
#define SPI_SPEED 500000
#define ADC_CS 29
#define MAX_TIME 100

/* Analog PIN */
// Dust
#define DUST_ADC_CH 3	
// Soil
#define SOIL_ADC_CH 5

/* Digital PIN */
//// Sensor ////
// PIR //
#define PIR_PIN 26 // IO12
// Dust //
#define DUST_D_PIN 4 // IO2
// DHT11
#define DHT11_PIN 3 // IO22
// Touch
#define TOUCH_PIN 1 // IO18

//// Actuator ////
// LED Module // 
#define LED_MODULE_1_PIN 9 // IO3
#define LED_MODULE_2_PIN 7 // IO4
#define LED_MODULE_3_PIN 0 // IO17
#define LED_MODULE_4_PIN 2 // IO27
// RGB LED //
#define LED_R_PIN 28 // IO20
#define LED_G_PIN 27 // IO16
#define LED_B_PIN 25 // IO26
// Buzzer //
#define BUZZER_PIN 6 // IO25
// DC Motor //
#define DC_MOTOR_A 11 // IO7
#define DC_MOTOR_B 10 // IO8
// Step Motor // 
#define STEP_MOTOR_1A_PIN 21 // IO5
#define STEP_MOTOR_1B_PIN 22 // IO6
#define STEP_MOTOR_2A_PIN 23 // IO13
#define STEP_MOTOR_2B_PIN 24 // IO19

typedef struct RGB_Color
{
	int r;
	int g;
	int b;
} rgb_Color;

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
int turn_off_all();
void *tcp_thread_communicate(void *arg);
void *bluetooth_thread_communicate();
void *socket_thread_communicate(void* _socket_addr);
void *buzzer();
void *dht(void*);
int ascending(const void*, const void*);
void *dust();
void *soil();
void *dc_motor();
rgb_Color *set_rgb(void*, int, int, int);
env_struct *set_env(void*, int, int, int, int, int, int, int);
void *led_module();
void *pir();
void *rgb_led();
void *step_motor();
void *touch();

int bluetooth_client;

int isActivated = 0;
int isChecking = 0;
// Thread exit(0) return value problems
int isMovementDetected = 0;
int isTouchDetected = 0;
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

int main(int argc, char **argv)
{
	// Wiring Pi Set up
	if (wiringPiSetup() == -1) return 1;
	if (wiringPiSPISetup(SPI_CH, SPI_SPEED) == -1) return -1;
	
	// Assign wiringPI pins with pinMode
	assign_pin();
	
	// thread_t 
	pthread_t buzzerThread;
	pthread_t dcMotorThread;
	pthread_t dhtThread;
	pthread_t dustThread;
	pthread_t soilThread;
	pthread_t ledModuleThread;
	pthread_t pirThread;
	pthread_t rgbThread;
	pthread_t stepMotorThread;
	pthread_t touchThread;
	
	pthread_t bluetooth_thread;
	pthread_t tcp_thread;
	
	// Return value of thread
	void *threadReturns;
	// after fixing parameter problems
	// int pirThreadReturns = 0;
	// int touchThreadReturns = 0;
	
	// Turn off all sensors & actuators
	turn_off_all();
	
	rgb_Color rgb;
	// env_struct env;
	set_rgb(&rgb, 0, 0, 0);
	set_env(&env, 0, 0, 0, 0, 0, 0, 0);
	pthread_create(&bluetooth_thread, NULL, bluetooth_thread_communicate, NULL);
	pthread_create(&tcp_thread, NULL, tcp_thread_communicate, NULL);
	while(1)
	{
		// When Movements are not detected
		if (!isMovementDetected)
		{
			// PIR Sensor Activate
			pthread_create(&pirThread, NULL, pir, NULL);
			pthread_join(pirThread, &threadReturns);
		}
		// When Movements are detected
		else 
		{
			// Set Red LED 
			set_rgb(&rgb, 1, 0, 0);
			pthread_create(&rgbThread, NULL, rgb_led, (void*) &rgb);
			
			write_server(bluetooth_client, "Someone is here, is that you?\n");
			
			// Touch Sensor Activate
			pthread_create(&touchThread, NULL, touch, NULL);
			pthread_join(touchThread, &threadReturns);
			
			// When Touch is detected after movement
			if (isTouchDetected)
			{
				write_server(bluetooth_client, "Authorized Complete \nDevices Activated\n");
				
				isActivated = 1;
				isMovementDetected = 0;
				isTouchDetected = 0;
				
				printf("*** Activated ***\n");
				
				// Set Green LED
				set_rgb(&rgb, 0, 1, 0);
				pthread_create(&rgbThread, NULL, rgb_led, (void*) &rgb);
				
				// Buzzer 100ms 2 times
				int *time = (int*) 100;
				pthread_create(&buzzerThread, NULL, buzzer, (void*) &time);
				pthread_join(buzzerThread, &threadReturns);
				pthread_create(&buzzerThread, NULL, buzzer, (void*) &time);
				pthread_join(buzzerThread, &threadReturns);
			}
			// When Touch is not detected after movement during 3 secs
			else
			{
				// Set Red LED 
				set_rgb(&rgb, 1, 0, 0);
				pthread_create(&rgbThread, NULL, rgb_led, (void*) &rgb);
				
				// Buzzer 50ms 3 times
				int *time = (int*) 50;
				pthread_create(&buzzerThread, NULL, buzzer, (void*) &time);
				pthread_join(buzzerThread, &threadReturns);
				pthread_create(&buzzerThread, NULL, buzzer, (void*) &time);
				pthread_join(buzzerThread, &threadReturns);
				pthread_create(&buzzerThread, NULL, buzzer, (void*) &time);
				pthread_join(buzzerThread, &threadReturns);
			}
		}
		// 
		
		while (isActivated)
		{
			// Turn off all sensors & actuators
			turn_off_all();
			set_rgb(&rgb, 1, 0, 1);
			pthread_create(&rgbThread, NULL, rgb_led, (void*) &rgb);
			delay(100);
			
			// Start checking
			printf("*** Checking Environment ***");
			isChecking = 1;
			pthread_create(&ledModuleThread, NULL, led_module, NULL);
			delay(100);
			
			// Open Door 
			pthread_create(&stepMotorThread, NULL, step_motor, NULL);
			
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
			
			set_env(&env, dht11_val[2][2], dht11_val[2][2], 0, 0, dht11_val[2][0], dust_val, soil_val);
			
			printf("\n **********************\n * temperature: %d \n * humidity: %d%% \n * soil: %d \n * dust: %d \n **********************\n", env.temperature, env.humidity, env.soil, env.dust);
			
			if (env.temperature >= 40)
			{
				isEnvironmentProper = 0;
				isTemperatureHot = 1;
			}
			else if (env.temperature <= 5)
			{
				isEnvironmentProper = 0;
				isTemperatureCold = 1;
			}
			
			if (env.humidity <= 30 || env.humidity >= 60)
			{
				isEnvironmentProper = 0;
				isHumidityNotProper = 1;
			}
			
			if (env.soil <= 1000 || env.soil >= 10000) // temp
			{
				isEnvironmentProper = 0;
				isSoilNotProper = 1;
			}
			
			if (env.dust >= dustThreshold)
			{
				isEnvironmentProper = 0;
				isDustNotProper = 1;
				printf("*** Fan On ***\n");
				pthread_create(&dcMotorThread, NULL, dc_motor, NULL);
				pthread_join(dcMotorThread, &threadReturns);
				printf("*** Fan Off ***\n");
			}
			
			// When Environment is not good
			if (!isEnvironmentProper)
			{
				printf("\n Not good ! \n");
				set_rgb(&rgb, 1, 0, 0);
				pthread_create(&rgbThread, NULL, rgb_led, (void*) &rgb);
				
				// Buzzer 50ms 3 times
				int *time = (int*) 50;
				pthread_create(&buzzerThread, NULL, buzzer, (void*) &time);
				pthread_join(buzzerThread, &threadReturns);
				pthread_create(&buzzerThread, NULL, buzzer, (void*) &time);
				pthread_join(buzzerThread, &threadReturns);
				pthread_create(&buzzerThread, NULL, buzzer, (void*) &time);
				pthread_join(buzzerThread, &threadReturns);
				
				isActivated = 0;
				isMovementDetected = 0;
				isTouchDetected = 0;
				delay(1000);
				break;
			}
			// When Environment is good
			else
			{
				printf("\n Good ! \n");
				// Set Blue LED during 3 secs
				set_rgb(&rgb, 0, 0, 1);
				pthread_create(&rgbThread, NULL, rgb_led, (void*) &rgb);
				delay(3000);
				// Set Green LED
				set_rgb(&rgb, 0, 1, 0);
				pthread_create(&rgbThread, NULL, rgb_led, (void*) &rgb);
				
				isActivated = 0;
				isMovementDetected = 0;
				isTouchDetected = 0;
				delay(3000);
				break;
			}
			
			delay(1000);
		}
		delay(1000);
	}
	return 0;
}

//
int assign_pin()
{
	//// Dust Sensor 
	// result of Dust Sensor
	pinMode(DUST_D_PIN, OUTPUT);
	
	//// Soil Moisture Sensor
	
	//// PIR Sensor
	pinMode(PIR_PIN, INPUT);
	
	//// DHT11
	pinMode(DHT11_PIN, OUTPUT);
	
	//// LED Module
	pinMode(LED_MODULE_1_PIN, OUTPUT);
	pinMode(LED_MODULE_2_PIN, OUTPUT);
	pinMode(LED_MODULE_3_PIN, OUTPUT);
	pinMode(LED_MODULE_4_PIN, OUTPUT);
	
	//// RGB LED
	pinMode(LED_R_PIN, OUTPUT);
	pinMode(LED_G_PIN, OUTPUT);
	pinMode(LED_B_PIN, OUTPUT);
	
	// Buzzer
	pinMode(BUZZER_PIN, OUTPUT);
	
	// DC Motor
	pinMode(DC_MOTOR_A, OUTPUT);
	pinMode(DC_MOTOR_B, OUTPUT);
	
	// Step Motor
	pinMode(STEP_MOTOR_1A_PIN, OUTPUT);
	pinMode(STEP_MOTOR_1B_PIN, OUTPUT);
	pinMode(STEP_MOTOR_2A_PIN, OUTPUT);
	pinMode(STEP_MOTOR_2B_PIN, OUTPUT);
	
	pinMode(TOUCH_PIN, INPUT);
	return 0;
}

int pc_socket;
//
int turn_each(void* _buffer)
{
	if (strcmp(_buffer, "Idle") == 10) {
		isMovementDetected = 0;
	}
	else if (strcmp(_buffer, "Check") == 10) {
		isActivated = 1;
	}
	else if (strcmp(_buffer, "Current") == 10) {
		char buffer[1024] = {0}; 
		//memset(buffer, 0, sizeof(buffer)); // 0 is unsigned char, so any kind of string could be okay
		sprintf(buffer,"\n **********************\n * temperature: %d \n * humidity: %d%% \n * soil: %d \n * dust: %d \n **********************\n \0", env.temperature, env.humidity, env.soil, env.dust);
		
		//valread = read(_socket , buffer, 1024);
		//printf("Socket %d Client : %s\n", _socket, buffer);
		write(pc_socket, buffer, sizeof(buffer));
	}
	
	
	return 0;
}

//
int turn_off_all()
{
	digitalWrite(BUZZER_PIN, LOW);
	digitalWrite(DC_MOTOR_A, HIGH);
	digitalWrite(DC_MOTOR_B, HIGH);
	digitalWrite(LED_MODULE_1_PIN, LOW);
	digitalWrite(LED_MODULE_2_PIN, LOW);
	digitalWrite(LED_MODULE_3_PIN, LOW);
	digitalWrite(LED_MODULE_4_PIN, LOW);
	digitalWrite(LED_R_PIN, LOW);
	digitalWrite(LED_G_PIN, LOW);
	digitalWrite(LED_B_PIN, LOW);
	digitalWrite(STEP_MOTOR_1A_PIN, LOW);
	digitalWrite(STEP_MOTOR_1B_PIN, LOW);
	digitalWrite(STEP_MOTOR_2A_PIN, LOW);
	digitalWrite(STEP_MOTOR_2B_PIN, LOW);
	return 0;
}

void *tcp_thread_communicate(void *arg)
{
	int server_fd, new_socket; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	
	// Create socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY -> automatically use any ip address of LAN card available  
	address.sin_port = htons(PORT); // PORT = uint16_t hostshort, 2bytes port number 
 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}
	
	while (1)
	{
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		
		pc_socket = new_socket;
		
		printf("Socket %d is Connected\n", new_socket);

		pthread_t client_thread;
		pthread_create(&client_thread, NULL, socket_thread_communicate, &new_socket);
		
		delay(1000);
	}
}

void *bluetooth_thread_communicate(void *arg)
{
	bluetooth_client = init_server();
	
	char *recv_message;
	char c0[13] = "\n **********\0";
	char c1[20] = "\n * temperature : \0";
	char c2[16] = "\n * humidity : \0";
	char c3[12] = "\n * soil : \0";
	char c4[12] = "\n * dust : \0";
	//env_struct *_env = (env_struct*) arg;
	while(1){
		
		recv_message = read_server(bluetooth_client);
		if ( recv_message == NULL ){
			printf("client disconnected\n");
			break;
		}
		else {
			//printf("%d\n",strcmp(recv_message, "current"));
			//write_server(bluetooth_client, recv_message);
			if (strcmp(recv_message, "current") == 13)
			{
				char buffer[10];
				sprintf(buffer, "%d", env.temperature);
				printf("%s", buffer);
				char msg[100], tmp[10] = {0};
				memset((void*) msg, NULL, sizeof(msg));
				strcat(msg, c0);
				strcat(msg, c1);
				sprintf(tmp, "%d", env.temperature);
				strcat(msg, tmp);
				strcat(msg, c2);
				sprintf(tmp, "%d", env.humidity);
				strcat(msg, tmp);
				strcat(msg, c3);
				sprintf(tmp, "%d", env.soil);
				strcat(msg, tmp);
				strcat(msg, c4);
				sprintf(tmp, "%d", env.dust);
				strcat(msg, tmp);
				strcat(msg, c0);
				write_server(bluetooth_client, msg);
			}
			else
			{
				printf("not yet");
			}
		}
	}
	
	pthread_exit(0);
}

// Make thread communicating function to make client and server communicate
void *socket_thread_communicate(void* _socket_addr) // receive address of new_socket value as void* type 
{	
	// cast into int* type, and take the value off with outer *
	// and store it in _socket
	int _socket = * (int*) _socket_addr;							
	int valread;
	char buffer[1024] = {0}; 

	// set memory and print via buffer
	while (1)
	{
		memset(buffer, 0, sizeof(buffer)); // 0 is unsigned char, so any kind of string could be okay
		valread = read(_socket , buffer, 1024);
		printf("Socket %d Client : %s\n", _socket, buffer);
		write(_socket, buffer, valread); // valread -> read(), strlen(buffer) -> getline()
		turn_each(buffer);
	}
}

//
void *buzzer(int *_time)
{
	digitalWrite(BUZZER_PIN, HIGH);
	delay(*_time);
	digitalWrite(BUZZER_PIN, LOW);
	delay(100);
	pthread_exit(0);
}

//
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
	   printf("\nHumidity = %d.%d %% Temperature = %d.%d *C (%.1f *F)\n", dht11_val[k][0], dht11_val[k][1], dht11_val[k][2], dht11_val[k][3], farenheit);
	   for(i=0 ; i<5; i++)
		  dht11_temp[i] = dht11_val[k][i];
	   farenheit_temp = farenheit;
   }
   else
   {
	   printf("\nHumidity = %d.%d %% Temperature = %d.%d *C (%.1f *F)\n", dht11_val[k][0], dht11_val[k][1], dht11_val[k][2], dht11_val[k][3], dht11_val[k][2]*9./5.+32);
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

//
void *dc_motor()
{
	digitalWrite(DC_MOTOR_A, HIGH);
	digitalWrite(DC_MOTOR_B, LOW);
	delay(5000);
	digitalWrite(DC_MOTOR_A, LOW);
	digitalWrite(DC_MOTOR_B, LOW);
	pthread_exit(0);
}

//
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

//
void *pir()
{
	int _pir, i;
	for(i=0; i<100; i++){
		_pir = digitalRead(PIR_PIN);
		delay(100);
		if (_pir)
		{
			printf("*** movement detected ***\n");
			break;
		}
	}
	delay(100);
	if (i == 100) isMovementDetected = 0;
	else isMovementDetected = _pir;
	pthread_exit((void*)&_pir);
}

rgb_Color *set_rgb(void *arg, int _r, int _g, int _b)
{
	rgb_Color *_rgb = (rgb_Color*)arg;
	_rgb->r = _r;
	_rgb->g = _g;
	_rgb->b = _b;
	return _rgb;
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

//
void *rgb_led(void *arg)
{
	rgb_Color *_rgb = (rgb_Color*)arg;
	if (_rgb->r) digitalWrite(LED_R_PIN, HIGH);
	else digitalWrite(LED_R_PIN, LOW);
	if (_rgb->g) digitalWrite(LED_G_PIN, HIGH);
	else digitalWrite(LED_G_PIN, LOW);
	if (_rgb->b) digitalWrite(LED_B_PIN, HIGH);
	else digitalWrite(LED_B_PIN, LOW);
	delay(100);
	pthread_exit(0);
}

//
void *step_motor()
{
	digitalWrite(STEP_MOTOR_1A_PIN, HIGH);
	digitalWrite(STEP_MOTOR_1B_PIN, LOW);
	digitalWrite(STEP_MOTOR_2A_PIN, LOW);
	digitalWrite(STEP_MOTOR_2B_PIN, LOW);
	delay(2000);
	digitalWrite(STEP_MOTOR_1A_PIN, LOW);
	digitalWrite(STEP_MOTOR_1B_PIN, HIGH);
	digitalWrite(STEP_MOTOR_2A_PIN, LOW);
	digitalWrite(STEP_MOTOR_2B_PIN, LOW);
	delay(2000);
	digitalWrite(STEP_MOTOR_1A_PIN, LOW);
	digitalWrite(STEP_MOTOR_1B_PIN, LOW);
	digitalWrite(STEP_MOTOR_2A_PIN, HIGH);
	digitalWrite(STEP_MOTOR_2B_PIN, LOW);
	delay(2000);
	digitalWrite(STEP_MOTOR_1A_PIN, LOW);
	digitalWrite(STEP_MOTOR_1B_PIN, LOW);
	digitalWrite(STEP_MOTOR_2A_PIN, LOW);
	digitalWrite(STEP_MOTOR_2B_PIN, HIGH);
	delay(2000);
	pthread_exit(0);
}

//
void *touch()
{
	int _touch, i;
	for(i=0; i<30; i++){
		_touch = digitalRead(TOUCH_PIN);
		delay(100);
		if (_touch)
		{
			printf("*** touch detected ***\n");
			break;
		}
	}
	if (i == 30) isTouchDetected = 0;
	else isTouchDetected = _touch;
	pthread_exit((void*) &_touch);
}
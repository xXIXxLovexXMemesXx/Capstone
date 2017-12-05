#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "net.h"
#include "sensor.h"
#include "mraa.hpp"
//#define _CRT_SECURE_NO_WARNINGS 

//hold the data to send to the server
server_data myData;
pthread_mutex_t myData_m;

int main()
{
  ////init shared stuff
  pthread_mutex_init(&myData_m, NULL);
  //init data structure
  myData.picOnline = false; //true if pic is online
  myData.adcData = 0; //value of PIC adc
  strncpy(myData.fileName, DEFAULT_FILENAME, MAX_FILENAME); //filename of last 

  ////create threads
  pthread_t command_thread;
  pthread_t sensor_control_thread;
  pthread_t net_thread;

  ////run threads
  pthread_create(&command_thread, NULL,commandLoop, NULL);
  //pthread_create(&sensor_control_thread, NULL, /*tbd*/, NULL);
  pthread_create(&net_thread, NULL, serverPostLoop, NULL);
 
  //jump out
  pthread_exit(NULL);
}

void* commandLoop(void * x)
{
	//initialize the I2C communication with the PIC
	using namespace mraa;
	I2c i2c(0);
	i2c.address(xxx/*undefined as of now*/);

	char word[20];
	int cmd;
	unsigned int value_ADC;
	printf("Enter the command to be used (1-4)\n");
	scanf("%d", &cmd);
	if (cmd == 1)
	{
		strcpy(word, "dingo");
	}
	else if (cmd == 2)
	{
		sensorPing();
	}
	else if (cmd == 3)
	{
		value_ADC = sendADCresults();

	}
}

//returns a copy of the current state
server_data getCurrentState()
{
  server_data s;

  pthread_mutex_lock(&myData_m);
  memcpy(&s, &myData, sizeof(server_data));
  pthread_mutex_unlock(&myData_m);

  return s;
}
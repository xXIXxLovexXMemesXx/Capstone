#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "net.h"
//#define _CRT_SECURE_NO_WARNINGS 

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
  //pthread_create(&command_thread, NULL, /*tbd*/, NULL);
  //pthread_create(&sensor_control_thread, NULL, /*tbd*/, NULL);
  pthread_create(&net_thread, NULL, serverPostLoop, NULL);
  //
  //jump out
  pthread_exit(NULL);
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
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "common.h"
//#define _CRT_SECURE_NO_WARNINGS 

server_data myData;
pthread_mutex_t myData_m;


int main()
{
  ////init
  pthread_mutex_init(&myData_m, NULL);
  //init data structure
  myData.picOnline = false; //true if pic is online
  myData.adcData = 0; //value of PIC adc
  strncpy(myData.fileName, DEFAULT_FILENAME, MAX_FILENAME); //filename of last 

  ////start threads
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
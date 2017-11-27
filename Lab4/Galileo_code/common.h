#ifndef COMMON_H
#define COMMON_H

#include <pthreads.h>

//constants
const int MAX_FILENAME = 256;

//structure to hold data that is shared between threads
typedef struct {
  bool picOnline; //true if pic is online
  int adcData; //value of PIC adc
  char fileName[MAX_FILENAME]; //filename of last image taken
}server_data;


//shared data variable name + mutex
extern server_data myData;
extern mutex_t myData_m;

#endif
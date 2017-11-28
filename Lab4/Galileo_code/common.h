#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

//constants
#define MAX_FILENAME            256
const char* DEFAULT_FILENAME = "No face detected";

//structure to hold data that is shared between threads
typedef struct {
  bool picOnline; //true if pic is online
  int adcData; //value of PIC adc
  char fileName[MAX_FILENAME]; //filename of last image taken
}server_data;

//gets the current global myData object
server_data getCurrentState();
#endif
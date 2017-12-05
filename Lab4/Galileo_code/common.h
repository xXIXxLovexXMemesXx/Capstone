#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

//constants
#define MAX_FILENAME            256
#define DEFAULT_FILENAME "No_face_detected"

//structure to hold data that is shared between threads
typedef struct {
  bool picOnline; //true if pic is online
  unsigned int adcData; //value of PIC adc
  char fileName[MAX_FILENAME]; //filename of last image taken
}server_data;

//gets the current global myData object
server_data getCurrentState();

#endif
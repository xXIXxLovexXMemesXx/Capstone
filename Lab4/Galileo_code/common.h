#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include "mraa.hpp" 
#include "stdint.h"

//constants
#define MAX_FILENAME            256
#define DEFAULT_FILENAME "No_face_detected"
#define TEMPERATURE_REGISTER 0x00	//According to table 1 in the TMP102 Datasheet
#define CONFIG_REGISTER 0x01	  	//These values will determine what register we are
#define T_LOW_REGISTER 0x02			//communicating with..not sure if we them.....
#define T_HIGH_REGISTER 0x03

#define TMP102Address 0x48

//structure to hold data that is going to the server
typedef struct {
  bool picOnline; //true if pic is online
  unsigned int adcData; //value of PIC adc
  char fileName[MAX_FILENAME]; //filename of last image taken
}server_data;

//gets the current global myData object
server_data getCurrentState();

#endif
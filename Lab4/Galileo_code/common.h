#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

//constants
#define MAX_FILENAME            256
#define DEFAULT_FILENAME "No_face_detected"
#define TEMPERATURE_REGISTER 0x00	//According to table 1 in the TMP102 Datasheet
#define CONFIG_REGISTER 0x01	  	//These values will determine what register we are
#define T_LOW_REGISTER 0x02			//communicating with..not sure if we them.....
#define T_HIGH_REGISTER 0x03

#define TMP102Address 0x48
using namespace mraa;

double temperature_threshold = 30;
double get_temp(); // temperature in C

double get_temp() {
	I2c i2c(0);
	i2c.address(TMP102Address);

	uint8_t dataReg[2];

	int buffer = i2c.read(dataReg, 2); // read two bytes from the registers

	int temperature = ((dataReg[0] << 8 | dataReg[1]) >> 4);

	return temperature*0.0625;
}

//structure to hold data that is shared between threads
typedef struct {
  bool picOnline; //true if pic is online
  unsigned int adcData; //value of PIC adc
  char fileName[MAX_FILENAME]; //filename of last image taken
}server_data;

//gets the current global myData object
server_data getCurrentState();

#endif
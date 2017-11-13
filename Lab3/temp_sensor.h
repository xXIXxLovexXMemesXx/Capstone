
#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H


#include "mraa.hpp"

#define TEMPERATURE_REGISTER 0x00	//According to table 1 in the TMP102 Datasheet
#define CONFIG_REGISTER 0x01	  	//These values will determine what register we are
#define T_LOW_REGISTER 0x02			//communicating with..not sure if we them.....
#define T_HIGH_REGISTER 0x03

#define TMP102Address 0x48
using namespace mraa;


double get_temp(); // temperature in C

double get_temp(){
	I2c i2c(0);
	i2c.address(TMP102Address);

uint8_t dataReg [2];

int buffer = i2c.read(dataReg,2); // read two bytes from the registers

int temperature = ((dataReg[0]<<8 | dataReg[1]) >> 4);

return temperature*0.0625;
}
#endif



/*/Config register kinda confused on if we need this defined or if a single write using mraa write/read function will work
os = 0;
r1 = 1;r0 = 1; //these are the power up value they are read-only anyways
f1 = 0; f0 = 1; //sets the device so that two consecutive faults (temp measured reads above t_high and t_low user defined values)
cr1 = 1;cr0 = 0; // for a 4Hz conversion rate 
pol = 0;
sd = 0; //shutdown bit when set = 0 device operates in continous conversion
al = 1; //power up value. will read 1 until temp measured >= t_high
em = 0; normal operation --12 bit temperature data format
*/


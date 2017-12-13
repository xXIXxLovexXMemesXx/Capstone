
#ifndef SENSOR_H
#define SENSOR_H



//temp sensor stuff
#define TEMPERATURE_REGISTER 0x00 //According to table 1 in the TMP102 Datasheet
#define CONFIG_REGISTER 0x01      //These values will determine what register we are
#define T_LOW_REGISTER 0x02     //communicating with..not sure if we them.....
#define T_HIGH_REGISTER 0x03

#define TMP102Address 0x48

//sensor access functions
bool capture_and_save_image(char* filename);

double get_temp(); // temperature in C


#endif
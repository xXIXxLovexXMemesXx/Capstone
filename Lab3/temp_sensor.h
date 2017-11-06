
#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

bool init_temp_sensor();
double get_temp_in_f();


//Config register
os = 0;
r1 = 1;r0 = 1; //these are the power up value they are read-only anyways
f1 = 0; f0 = 1; //sets the device so that two consecutive faults (temp measured reads above t_high and t_low user defined values)
cr1 = 1;cr0 = 0; // for a 4Hz conversion rate 
pol = 0;
sd = 0; //shutdown bit when set = 0 device operates in continous conversion
al = 1; //power up value. will read 1 until temp measured >= t_high
em = 0; normal operation --12 bit temperature data format
#endif
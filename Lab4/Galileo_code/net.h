#ifndef NET_H
#define NET_H

#include "common.h"

//Constants


//static data that won't change
const int GROUP_ID = 1;
const char* PASSWORD = "password";
const char* STUDENT_NAME = "The Rough Riders";
const char* DEFAULT_FILENAME = "No face detected";
const char* PIC_ONLINE = "Online";
const char* PIC_ERROR = "Error";

//server address information
const char* SERVER_HOSTNAME = "ec2-54-202-113-131.us-west-2.compute.amazonaws.com";
const int SERVER_PORTNUMBER = 8000;

//buffer lengths
const int MAX_TIMESTAMP = 64;
const int MAX_POST = 1024;


//functions to keep track of the data to send to the server.
void init();
void setDynamicData(bool picStatusOnline, int adcData, char* fn);
char* getPostRequest();

#endif 
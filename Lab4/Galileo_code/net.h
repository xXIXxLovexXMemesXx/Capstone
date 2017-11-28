#ifndef NET_H
#define NET_H

////Constants
//static data that won't change
const int GROUP_ID = 1;
const char* PASSWORD = "password";
const char* STUDENT_NAME = "The Rough Riders";
const char* PIC_ONLINE = "Online";
const char* PIC_ERROR = "Error";

//server address information
const char* SERVER_HOSTNAME = "ec2-54-202-113-131.us-west-2.compute.amazonaws.com";
const int SERVER_PORTNUMBER = 8000;

//buffer lengths
const int MAX_TIMESTAMP = 64;
const int MAX_POST = 1024;


////functions
//thread to send post updates every 2 seconds
void* serverPostLoop(void * x);

#endif 
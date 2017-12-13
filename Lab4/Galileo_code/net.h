#ifndef NET_H
#define NET_H


////Constants
//static data that won't change
#define GROUP_ID 1
#define PASSWORD  "password"
#define STUDENT_NAME "The_Rough_Riders"
#define PIC_ONLINE "Online"
#define PIC_ERROR "Error"

//server address information
#define SERVER_HOSTNAME  "ec2-54-202-113-131.us-west-2.compute.amazonaws.com"
#define SERVER_PORTNUMBER 8000

//buffer lengths
#define MAX_TIMESTAMP 64
#define MAX_POST 1024


////functions
//thread to send post updates every 2 seconds
void* serverPostLoop(void * x);

#endif 
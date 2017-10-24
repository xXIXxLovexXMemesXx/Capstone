/*
 * File:   PIC and Galileo communication          
 *         
 * 
 * simple PIC program example
 * for UMass Lowell 16.480/552
 * 
 * Author: Ioannis
 *
 * Created on 2017/9/21
 */


#include <pic16f18857.h>
#include "mcc_generated_files/mcc.h" //default library 

#define MSG_RESET   0x0
#define MSG_PING    0x1
#define MSG_GET     0x2
#define MSG_TURN30  0x3
#define MSG_TURN90  0x4
#define MSG_TURN120 0x5
#define MSG_ACK     0xE
#define MSG_NOTHING 0xF

/* Circuit Connections
   Signal STROBE   RC6
   Signal D0       RC2
   Signal D1       RC3
   Signal D2       RC4
   Signal D3       RC5
 */

void servoRotate0() //0 Degree -> reset servo position
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = 1;
    __delay_ms(1);
    PORTB = 0;
    __delay_ms(19);
  }
}

void servoRotate30() //30 Degree
{
  while(PORTCbits.RC6 == 1)
  {
        
  }  
  TRISC = 0b01000000;
  //send the ACK message to the galileo
  PORTCbits.RC2 = 0;
  PORTCbits.RC3 = 1;
  PORTCbits.RC4 = 1;
  PORTCbits.RC5 = 1;
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = 1;
    __delay_ms(1.18);
    PORTB = 0;
    __delay_ms(18.82);
  }
}

void servoRotate90() //90 Degree
{
  while(PORTCbits.RC6 == 1)
  {
        
  } 
  TRISC = 0b01000000;
  //send the ACK message to the galileo
  PORTCbits.RC2 = 0;
  PORTCbits.RC3 = 1;
  PORTCbits.RC4 = 1;
  PORTCbits.RC5 = 1;
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = 1;
    __delay_ms(1.5);
    PORTB = 0;
    __delay_ms(18.5);
  }
}

void servoRotate120() //120 Degree
{
  while(PORTCbits.RC6 == 1)
  {
        
  }
  TRISC = 0b01000000;
  //send the ACK message to the galileo
  PORTCbits.RC2 = 0;
  PORTCbits.RC3 = 1;
  PORTCbits.RC4 = 1;
  PORTCbits.RC5 = 1;
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = 1;
    __delay_ms(1.68);
    PORTB = 0;
    __delay_ms(18.32);
  }
  
}

void set_receive()
{  
  /*
   1.set RC6 as digital input
   2.set RC2, RC3, RC4 and RC5 as digital inputs
  */
    ANSELC = 0; //set portc to digital
    PORTC = 0; //clear portc 
    TRISC = 0b01111100; //setting RC2-6 as digital inputs
}

unsigned char receive_msg()
{
    set_receive();
    unsigned char results;
    while(PORTCbits.RC6 == 0)
    {
        
    }
    While(PORTCbits.RC6 == 1)
    {
        if(PORTCbits.RC5 == 0 && 
           PORTCbits.RC4 == 0 &&
           PORTCbits.RC3 == 0 &&
           PORTCbits.RC2 == 0)
        {
            results = 0x0;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC4 == 0 &&
                PORTCbits.RC3 == 0 &&
                PORTCbits.RC2 == 1)
        {
            results = 0x1;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC4 == 0 &&
                PORTCbits.RC3 == 1 &&
                PORTCbits.RC2 == 0)
        {
            results = 0x2;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC4 == 0 &&
                PORTCbits.RC3 == 1 &&
                PORTCbits.RC2 == 1)
        {
            results = 0x3;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC4 == 1 &&
                PORTCbits.RC3 == 0 &&
                PORTCbits.RC2 == 0)
        {
            results = 0x4;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC4 == 1 &&
                PORTCbits.RC3 == 0 &&
                PORTCbits.RC2 == 1)
        {
            results = 0x5;
        }
    }
 /* 1.wait strobe high
    2.wait strobe low
    3.read the data
    4.wait strobe high
    5.return the data
    */
    
}

void sensorReset()
{
    printf("REEEEEEEEEEE");
    while(PORTCbits.RC6 == 1)
    {
        
    }
    TRISC = 0b01000000;
    //send the ACK message to the galileo
    PORTCbits.RC2 = 0;
    PORTCbits.RC3 = 1;
    PORTCbits.RC4 = 1;
    PORTCbits.RC5 = 1;
    //reset the servo position
    servoRotate0();
}

void sensorPing();
{
    printf("to be replaced for the ping sensoring");
}


// Main program
void main (void)
{
    unsigned char msg;    
    while(1)
    {  
    msg=receive_msg();
    if(msg == MSG_RESET)
   	    sensorReset();
    else if (msg == MSG_PING)
        sensorPing();
    else if (msg == MSG_GET)
    {
        
    }
    else if (msg == MSG_TURN30)
        servoRotate30();   
    else if (msg == MSG_TURN90)
        servoRotate90();
    else if (msg == MSG_TURN120)
        servoRotate120();
    else
        PORTCbits.RC0 = 0;
    } 
}



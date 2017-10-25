
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
   Signal D1       RC1
   Signal D2       RC7
   Signal D3       RC5
 * 
 * Analog - RA1
 */

void servoRotate0() //0 Degree -> reset servo position
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = 1;
    __delay_ms(1.5);
    PORTB = 0;
    __delay_ms(18.5);
  }
}

void servoRotate30() //30 Degree
{
  printf("Delay me just a little bit\n");
  while(PORTCbits.RC6 == 1)
  {
        
  }  
  unsigned int i;
  TRISC = 0b01000000;
    //send the ACK message to the galileo
    PORTCbits.RC2 = 0;
    PORTCbits.RC1 = 1;
    PORTCbits.RC7 = 1;
    PORTCbits.RC5 = 1;
  for(i=0;i<50;i++)
  {
    PORTB = 1;
    __delay_ms(1.67);
    PORTB = 0;
    __delay_ms(18.33);
  }
}

void servoRotate90() //90 Degree
{
  printf("REEEEEEEEEEEEEEEEEEEEEEE\n");
  while(PORTCbits.RC6 == 1)
  {
        
  } 
  unsigned int i;
  TRISC = 0b01000000;
    //send the ACK message to the galileo
    PORTCbits.RC2 = 0;
    PORTCbits.RC1 = 1;
    PORTCbits.RC7 = 1;
    PORTCbits.RC5 = 1;
  for(i=0;i<50;i++)
  {
    PORTB = 1;
    __delay_ms(2);
    PORTB = 0;
    __delay_ms(18);
  }
}

void servoRotate120() //120 Degree
{
  printf("Delay me just a little bit\n");
  while(PORTCbits.RC6 == 1)
  {
        
  }
  unsigned int i;
  TRISC = 0b01000000;
    //send the ACK message to the galileo
    PORTCbits.RC2 = 0;
    PORTCbits.RC1 = 1;
    PORTCbits.RC7 = 1;
    PORTCbits.RC5 = 1;
  for(i=0;i<50;i++)
  {
    PORTB = 1;
    __delay_ms(1.33);
    PORTB = 0;
    __delay_ms(18.67);
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
    TRISC = 0b11101100; //setting RC2-6 as digital inputs
}

unsigned char receive_msg()
{
    set_receive();
    unsigned char results;
    while(PORTCbits.RC6 == 1)
    {
        
    }
    
    while(PORTCbits.RC6 == 0)
    {
        if(PORTCbits.RC5 == 0 && 
           PORTCbits.RC7 == 0 &&
           PORTCbits.RC1 == 0 &&
           PORTCbits.RC2 == 0)
        {
            results = 0x0;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC7 == 0 &&
                PORTCbits.RC1 == 0 &&
                PORTCbits.RC2 == 1)
        {
            results = 0x1;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC7 == 0 &&
                PORTCbits.RC1 == 1 &&
                PORTCbits.RC2 == 0)
        {
            results = 0x2;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC7 == 0 &&
                PORTCbits.RC1 == 1 &&
                PORTCbits.RC2 == 1)
        {
            results = 0x3;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC7 == 1 &&
                PORTCbits.RC1 == 0 &&
                PORTCbits.RC2 == 0)
        {
            results = 0x4;
        }
        else if(PORTCbits.RC5 == 0 && 
                PORTCbits.RC7 == 1 &&
                PORTCbits.RC1 == 0 &&
                PORTCbits.RC2 == 1)
        {
            results = 0x5;
        }
        else
            results = 0xF;
    }
    return results;
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
    PORTCbits.RC1 = 1;
    PORTCbits.RC7 = 1;
    PORTCbits.RC5 = 1;
    //reset the servo position
    servoRotate0();
}
void ADC_Init(void)  {
 //  Configure ADC module  
 //---- Set the Registers below::
 // 1. Set ADC CONTROL REGISTER 1 to 0 
 // 2. Set ADC CONTROL REGISTER 2 to 0 
 // 3. Set ADC THRESHOLD REGISTER to 0
 // 4. Disable ADC auto conversion trigger control register
 // 5. Disable ADACT  
 // 6. Clear ADAOV ACC or ADERR not Overflowed  related register
 // 7. Disable ADC Capacitors
 // 8. Set ADC Precharge time control to 0 
 // 9. Set ADC Clock 
 // 10 Set ADC positive and negative references
 // 11. ADC channel - Analog Input
 // 12. Set ADC result alignment, Enable ADC module, Clock Selection Bit, Disable ADC Continuous Operation, Keep ADC inactive
  
    
    TRISA = 0b11111110;   //set PORTA to input except for pin0
    TRISAbits.TRISA1 = 1;   //set pin A1 to input
    ANSELAbits.ANSA1 = 1;   //set as analog input
    ADCON1 = 0;
    ADCON2 = 0;
    ADCON3 = 0;
    ADACT = 0;
    ADSTAT = 0;
    ADCAP = 0;
    ADPRE = 0;
    ADCON0 = 0b10000100; // bit7 = enabled; bit 2 = 1: right justified
    ADREF = 0;
    ADPCH = 0b00000001; //set input to A1
    //UART initialization
    TX1STA = 0b00100000;
    RC1STA = 0b10000000;   
    printf("Initialized ADC\n");
} 

unsigned int ADC_conversion_results() {  
    ADPCH = 1; 
           
    ADCON0 |= 1;           //Initializes A/D conversion
    
    while(ADCON0 & 1);             //Waiting for conversion to complete
    unsigned result = (unsigned)((ADRESH << 8) + ADRESL); //0bXXXXXXHHLLLLLLLL
    return result;    
}

void sensorPing()
{
    printf("PING\n");
    PORTA ^= 1;
    while(PORTCbits.RC6 == 1)
    {
        
    }
    TRISC = 0b01000000;
    //send the ACK message to the galileo
    PORTCbits.RC2 = 0;
    PORTCbits.RC1 = 1;
    PORTCbits.RC7 = 1;
    PORTCbits.RC5 = 1;
    //PORTA = 0;
}

void sendADCResults()
{
    printf(";)\n");
    unsigned results;
    unsigned char nib1, nib2, nib3;
    //get the ADC value and break it into 3 nibbles
    results = ADC_conversion_results();
    nib1 = (results & 0x300) >> 8;
    nib2 = (results & 0x0F0) >> 4;
    nib3 = (results & 0x00F);
    
    //waits for the bus to go low
    while(PORTCbits.RC6 == 1)
    {
        
    } 
    
    
    //send the first nibble
    PORTCbits.RC2 = (nib1 & 0x1);
    PORTCbits.RC1 = (nib1 & 0x2) >>1;
    PORTCbits.RC7 = 0;
    PORTCbits.RC5 = 0;
    
    //wait for the bus to go high again
    while(PORTCbits.RC6 == 0)
    {
        
    } 
    
    //wait for strobe to go low again
    while(PORTCbits.RC6 == 1)
    {
        
    } 
    //send the second nibble.
    PORTCbits.RC2 = (nib2 & 0x1);
    PORTCbits.RC1 = (nib2 & 0x2) >>1;
    PORTCbits.RC7 = (nib2 & 0x4) >> 2;
    PORTCbits.RC5 = (nib2 & 0x8) >> 3;
    
    //wait for the bus to go high again
    while(PORTCbits.RC6 == 0)
    {
        
    } 
    
    //wait for strobe to go low again
    while(PORTCbits.RC6 == 1)
    {
        
    } 
    //send the third nibble.
    PORTCbits.RC2 = (nib3 & 0x1);
    PORTCbits.RC1 = (nib3 & 0x2) >>1;
    PORTCbits.RC7 = (nib3 & 0x4) >> 2;
    PORTCbits.RC5 = (nib3 & 0x8) >> 3;
    
    //wait for the bus to go high again
    while(PORTCbits.RC6 == 0)
    {
        
    } 
    
    //wait for strobe to go low again
    while(PORTCbits.RC6 == 1)
    {
        
    }
    PORTCbits.RC2 = 0;
    PORTCbits.RC1 = 1;
    PORTCbits.RC7 = 1;
    PORTCbits.RC5 = 1;
}

// Main program
#define ADC_THRESHOLD 0x0380
void main (void)
{
    SYSTEM_Initialize();
    unsigned results;
    
    unsigned char msg; 
    TRISB = 0;
    ADC_Init();
    printf("Starting main\n");
    TRISAbits.TRISA0 = 0; //make sure portA0 is ouput for the LED
    while(1)
    {  
    results = ADC_conversion_results();
    if(results > ADC_THRESHOLD)
            PORTA |= 0x01; //turn on LEd
    else
            PORTA &= !0x01; //turn off LED
    msg=receive_msg();
    if(msg == MSG_RESET)
   	    sensorReset();
    else if (msg == MSG_PING)
        sensorPing();
    else if (msg == MSG_GET)
    {
        sendADCResults();
    }
    else if (msg == MSG_TURN30)
        servoRotate30();   
    else if (msg == MSG_TURN90)
        servoRotate90();
    else if (msg == MSG_TURN120)
        servoRotate120();
    else
        (void) 0;
    } 
}


/**
 End of File
*/
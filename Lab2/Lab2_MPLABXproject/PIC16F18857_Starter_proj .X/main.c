

#include <pic16f18857.h>
#include "mcc_generated_files/mcc.h" //default library 

//messages 
#define MSG_RESET   0x0
#define MSG_PING    0x1
#define MSG_GET     0x2
#define MSG_TURN30  0x3
#define MSG_TURN90  0x4
#define MSG_TURN120 0x5
#define MSG_ACK     0xE
#define MSG_NOTHING 0xF

//constants
#define INPUT       1
#define OUTPUT      0

#define HIGH        1
#define LOW         0

#define no_op       (void) 0 //an instruction that does nothing

// Circuit Connections
// ADC input - A1
// Signal STROBE    RC6
#define STROBE_VAL  PORTCbits.RC6
#define STROBE_TRI  TRISCbits.RC6
// Signal D0        RC2
#define D0_VAL      PORTCbits.RC2
#define D0_TRI      TRISCbits.RC2
// Signal D1        RC1
#define D1_VAL      PORTCbits.RC1
#define D1_TRI      TRISCbits.RC1
// Signal D2        RC7
#define D2_VAL      PORTCbits.RC7
#define D2_TRI      TRISCbits.RC7
// Signal D3        RC5
#define D3_VAL      PORTCbits.RC5
#define D3_TRI      TRISCbits.RC5
  
  
//Rotate the servo to 0 degrees
void servoRotate0() 
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = HIGH;
    __delay_ms(1.5);
    PORTB = LOW;
    __delay_ms(18.5);
  }
}

//Rotate the servo to 30 degrees
void servoRotate30() 
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = HIGH;
    __delay_ms(1.68);
    PORTB = LOW;
    __delay_ms(18.32);
  }
}

//Rotate the servo to 90 degrees
void servoRotate90() 
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = HIGH;
    __delay_ms(2);
    PORTB = LOW;
    __delay_ms(18);
  }
}

//Rotate the servo to 120 degrees
void servoRotate120() 
{
  unsigned int i;
  for(i=0;i<50;i++)
  {
    PORTB = HIGH;
    __delay_ms(1.32);
    PORTB = LOW;
    __delay_ms(18.68);
  }
  
}

//Get prepared to receive a message
void set_receive()
{  
    //Set strobe and d0-3 as digital input
    ANSELC = 0; 
    TRISC = 0b11100110;
    // TRISC |= INPUT << 2;
    // TRISC |= INPUT << 1;
    // TRISC |= INPUT << 7;
    // TRISC |= INPUT << 6;
    // TRISC |= INPUT << 5;
}

//Get prepared to send a message
void set_send()
{  
    //Set strobe as input, and d0-3 as ouptut
    ANSELC = 0; 
    TRISC |= INPUT << 6;
    TRISC = 0b01000000;
    // TRISC &= !(INPUT << 2);
    // TRISC &= !(INPUT << 1);
    // TRISC &= !(INPUT << 7);
    // TRISC &= !(INPUT << 5);
}

unsigned char get_msg_on_line()
{
  unsigned char data;
  data += D0_VAL;
  data += D1_VAL << 1;
  data += D2_VAL << 2;
  data += D3_VAL << 3;
  return data;
}

void put_msg_on_line(unsigned char data)
{
  D0_VAL = (data & 0b0001);
  D1_VAL = (data & 0b0010) >> 1;
  D2_VAL = (data & 0b0100) >> 2;
  D3_VAL = (data & 0b1000) >> 3;
}

//perform a read operation, start to finish in the 
unsigned char receive_msg()
{
  unsigned char data;
  //get ready to receive
  set_receive();

  //1 wait for galileo to pull strobe low
  while(STROBE_VAL == HIGH) 
  { no_op; }

  //2 computer outputs a commmand to the bus
  
  //3 computer raises strobe.... when it does, PIC reads the value
  while(STROBE_VAL == LOW)
  { no_op;} //mill until the strobe raises
  
  data = get_msg_on_line();

  while(STROBE_VAL == HIGH)
  { 
    no_op;
  } 

  //4. Galileo ends write by pulling strobe low again (for 10 ms)
  //wait until the bus goes back high.
  while(STROBE_VAL == LOW)
  { 
    no_op;
  }

  //5. galileo stops putting the command on the bus, and strobe floats high
  //   write is concluded
  return data;
}

void send_msg(unsigned char data)
{
    //get ready to receive
  set_send();

  //1 wait for galileo to pull strobe low
  while(STROBE_VAL == HIGH) 
  { 
    no_op; 
  }

  //2 pic outputs a commmand to the bus
  put_msg_on_line(data);

  //3 computer raises strobe and starts reading the value
  while(STROBE_VAL == LOW)
  { 
    //mill until the strobe raises
    no_op;
  } 
  
  //PIC learns the computer reads the value

  //The galileo pulls the strobe signal low to indicate the value has been read
  while(STROBE_VAL == HIGH)
  { 
    no_op;
  } 

  //5. pic sees the strobe pulled low and stops outputting the 4 bit value
  put_msg_on_line(0x00);

  //the galileo waits a little bit and puts the bus back high
  while(STROBE_VAL == LOW)
  { 
    no_op;
  }

  //once strobe goes back high, the write is completed
}

void sensorReset()
{
    printf("REEEEEEEEEEE");
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
    ADCON0 = 0b10000100; 
    ADREF = 0;
    ADPCH = 0b00000001;
} 

unsigned int ADC_conversion_results() {  
    ADPCH = 1; 
           
    ADCON0 |= 1;           //Initializes A/D conversion
    
    while(ADCON0 & 1);             //Waiting for conversion to complete
    unsigned result = (unsigned)((ADRESH << 8) + ADRESL);
    return result;    
}

void sensorPing()
{
  printf("Pong");
}

void sendADCResults()
{
  printf("todo");
}

// Main program
void main (void)
{
  unsigned char msg;
  SYSTEM_Initialize();
  ADC_Init();
  TRISA = OUTPUT; //make sure portA0 is ouput for the LED
  while(1)
  {  
    //receive the message
    msg=receive_msg();

    //does something based on that message
    if(msg == MSG_RESET)
    {
      send_msg(MSG_ACK);
      sensorReset();
    }
    else if (msg == MSG_PING)
    {
      send_msg(MSG_ACK);
      sensorPing();
    }
    else if (msg == MSG_GET)
    {
      send_msg(MSG_ACK);
      sendADCResults();
      send_msg(0x6); //fake data 4 now
      send_msg(0x6);
      send_msg(0x6);
    }
    else if (msg == MSG_TURN30)
    {
      send_msg(MSG_ACK);
      servoRotate30();
    }
    else if (msg == MSG_TURN90)
    {
        send_msg(MSG_ACK);
        servoRotate90();
    }
    else if (msg == MSG_TURN120)
    {
        send_msg(MSG_ACK);
        servoRotate120();
    }
    else
    {
        no_op;
    }

  } // \while
}



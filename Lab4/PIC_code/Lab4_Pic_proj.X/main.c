
#include <pic16f18857.h>
#include "mcc_generated_files/mcc.h" //default library 

#define MSG_RESET   0x0
#define MSG_PING    0x1
#define MSG_GET     0x2
#define MSG_TURN30  0x3
#define MSG_TURN90  0x4
#define MSG_TURN120 0x5
#define MSG_LED_ON  0x6
#define MSG_LED_OFF 0x7
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
    printf("Servo Rotate 0\n");
    for(i=0;i<50;i++)
    {
        PORTB = 1;
        __delay_ms(1.4);
        PORTB = 0;
        __delay_ms(18.6);
    }
}

void servoRotate30() //30 Degree
{
    unsigned int i;
    printf("Servo Rotate 30\n");
    for(i=0;i<50;i++)
    {
        PORTB = 1;
        __delay_ms(1.75);
        PORTB = 0;
        __delay_ms(18.33);
    }
}

void servoRotate90() //90 Degree
{
    unsigned int i;
    printf("Rotate90\n");
    for(i=0;i<50;i++)
    {
        PORTB = 1;
        __delay_ms(2.1);
        PORTB = 0;
        __delay_ms(17.9);
    }
}

void servoRotate120() //120 Degree
{
    unsigned i;
    printf("Servo rotate 120\n");
    for(i=0;i<50;i++)
    {
        PORTB = 1;
        __delay_ms(1.1);
        PORTB = 0;
        __delay_ms(18.9);
    }
}

unsigned char receive_msg()
{
    unsigned char results;
   
}

void sensorReset()
{
    printf("REEEEEEEEEEE-set");

    servoRotate0();
}

void i2cInit(void)
{
    //put in Slave receive mode
        //SSPxCON1 register
    //set address
        //SSPxADD
}

void ADCInit(void)  {
    //  Configure ADC module  
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
    
    printf("Initialized ADC\n");
} 

void sensorPing()
{
    printf("PING\n");
}

unsigned int ADC_conversion_results() {  
    ADPCH = 1; 
           
    ADCON0 |= 1;           //Initializes A/D conversion
    
    while(ADCON0 & 1);             //Waiting for conversion to complete
    unsigned result = (unsigned)((ADRESH << 8) + ADRESL); //0bXXXXXXHHLLLLLLLL
    return result;    
}

void sendADCResults()
{
    printf("Sending ADC results \n");
    unsigned results;
    unsigned char nib1, nib2, nib3;
    //get the ADC value and break it into 3 nibbles
    results = ADC_conversion_results();
    nib1 = (results & 0x300) >> 8;
    nib2 = (results & 0x0F0) >> 4;
    nib3 = (results & 0x00F);
    
}

void initModules(void)
{
    ADCInit();
    i2cInit();
}

// Main program
void main (void)
{
    SYSTEM_Initialize();
    unsigned results;
    
    unsigned char msg; 
    TRISB = 0;
    initModules();
    printf("Starting main\n");
    
    while(1)
    {  
        //wait for ADC interrupts
    } 
}


/**
 End of File
*/
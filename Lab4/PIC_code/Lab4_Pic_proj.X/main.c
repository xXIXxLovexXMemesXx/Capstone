
#include <pic16f18857.h>
#include "mcc_generated_files/mcc.h" //default library 

/////messages
#define MSG_RESET       0x0
#define MSG_PING        0x1
#define MSG_GET         0x2
#define MSG_TURN30      0x3
#define MSG_TURN90      0x4
#define MSG_TURN120     0x5
#define MSG_ROTATE90_E  0x6
#define MSG_ROTATE180_E 0x7
#define MSG_ROTATE90_D  0x8
#define MSG_ROTATE180_D 0x9

#define MSG_ACK         0xE
#define MSG_NOTHING     0xF

/* Circuit Connections
   Signal STROBE    RC6
   Signal D0        RC3
   Signal D1        RC1
   Signal D2        RC7
   Signal D3        RC5
 * ////just don't use C4 
 * Analog           RA1
 * Servo            RC2
 */

////Global state variables.
//1 if in scan mode. 0 if not
int isScanning;

int scanCounter; //current duty cycle in the scan
int isScanningUp; //direction of scanCounter increment. 0 = down, 1 = up
int scanUpperLimit; //upper % scan will go to
int scanLowerLimit; //lower % scan limit

//fwd declare tests
void test_main();

void adc_init(void)  {
    //  Configure ADC module  
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

//where duty is from 0 to 100 %
void set_pwm_duty(unsigned duty)
{
    unsigned pr;
    //pulse width = CCPR4H:CCPR4L * 64 / FOSC
    //duty = 0 -> PW = 1ms, PR = 15
    //duty = 100 -> PW = 2ms, PR = 31
    pr = 15 + duty*16/100; //turn % duty into PR setting
    CCPR1H = 0;
    CCPR1L = pr;
}

//init PWM module then ADC module
void everything_init()
{
    ////init PWM module
    //Set whole port B to output
    TRISC = 0; 
    
    //set frquency of PWM
    //PWM period = 20ms = [(PR2) + 1]] * 4* _XTAL_FREQ * TMR2 prescale
    // PR2 = 20ms / (4 * 1/_XTAL_FREQ * 64) ~= 77
    PR2 =  77; //set timer2 period 
    
    //use CCP1 because it goes to RC1
    CCP1CON = 0x8F; //set enabled.. in PWM mode
    
    //load CCPRxL and CCPRxH
    set_pwm_duty(50);
    
    //point timer2 at CCP1
    CCPTMRS0 = 0x01;
    
    //set RxyPPS to let CCP1 take over C2
    RC2PPS = 0x09;
    
    //configure + start TIMER2
    T2CLKCON = 0x01; // set timer source to FOSC/4
    T2CON = 0xE0; //set timer2 on; prescaler = 64
     
    //clear TMR2IF of PIR4
    PIR4 = 0;
    
    T2CON |= 0x80;
    
    ////init timer0 for the scan interval -- shoot for 2 deg/s
    T0CON0 = 0x90;//enable TMR0 in 8 bit mode w/ postscaler = 0
    T0CON1 = 0x41; //fSource to FOSC/4 with prescale 1
    T0CON0 |= 0x80;
    
    ////init LED
    TRISA = 0;
    
    ////Init ADC
    adc_init();
}


//do nothing while the strobe bus is in the current state
void wait_while_strobe(int state)
{
    while(PORTCbits.RC6 == state) {}
}

//puts a nibble on the bus
//only sends the lower 4 bits of whatever you pass
void send_nibble(unsigned char nibble)
{
    TRISC = 0b01000000;
    PORTCbits.RC3 = (nibble) & 1;
    PORTCbits.RC1 = (nibble >> 1) & 1 ;
    PORTCbits.RC7 = (nibble >> 2) & 1 ;
    PORTCbits.RC5 = (nibble >> 3) & 1;
}
//Does nothing but return an ACK on the bus
//following the strobe bus protocl
void sensor_ping()
{
    printf("PING\n");
    wait_while_strobe(1);
    
    //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

//resets state variables and sets servo position to 0
void sensor_reset()
{
    printf("REEEEEEEEEEEEEEEEEEEEEEEset\n");
    wait_while_strobe(1);
    
    //action here
    set_pwm_duty(50);
    isScanning = 0;
    scanCounter = 50;
    scanUpperLimit = 75;
    scanLowerLimit = 25;
    //
    
   //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

//sends ADC results in 3 nibbles
void send_adc_results()
{
    printf("Sending ADC results \n");
    unsigned results;
    unsigned char nib1, nib2, nib3;
    
    //get the ADC value and break it into 3 nibbles
    ADPCH = 1;
    ADCON0 |= 1; //init A/D conversion
    while(ADCON0 & 1); //wait for conversion
    results = (unsigned) ((ADRESH << 8) + ADRESL);
    
    nib1 = (results & 0x300) >> 8;
    nib2 = (results & 0x0F0) >> 4;
    nib3 = (results & 0x00F);
    
    wait_while_strobe(0); //only start when the bus is high
    wait_while_strobe(1); //wait for the bus to go low
    
    //send the first nibble
    send_nibble(nib1);
    
    wait_while_strobe(0); //wait for bus to go high again
    wait_while_strobe(1); //then for strobe to go low again
    wait_while_strobe(0); //then high.. end the write
    wait_while_strobe(1); //then low
    
    //send the second nibble
    send_nibble(nib2);
    
    wait_while_strobe(0); //wait for bus to go high again
    wait_while_strobe(1); //then for strobe to go low again
    wait_while_strobe(0); //then high.. end the write
    wait_while_strobe(1); //then low
    
    //send the third nibble
    send_nibble(nib3);
    
    wait_while_strobe(0); //wait for bus to go high again
    wait_while_strobe(1); //then for strobe to go low again
    wait_while_strobe(0); //then high.. end the write
    wait_while_strobe(1); //then low
    
    //send ACK
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0); //G reads ack
    wait_while_strobe(1); //G is done reading
    wait_while_strobe(0); //G is done on the bus
}

//turn the servo to a position that may be 30 degrees
void turn_30()
{
    printf("turn 30\n");
    wait_while_strobe(1);
    
    //action here
    set_pwm_duty(75);
    isScanning = 0;
    //
    
   //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

//turn the servo to a position that may be 90 degrees
void turn_90()
{
    printf("servo 90\n");
    wait_while_strobe(1);
    
    //action here
    set_pwm_duty(100);
    isScanning = 0;
    //
    
   //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

//turn the servo to a position that may be 120 degrees
void turn_120()
{
    printf("scan 120\n");
    wait_while_strobe(1);
    
    //action here
    set_pwm_duty(0);
    isScanning = 0;
    //
    
   //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

//enter scan mode
void enter_scan_mode()
{
    printf("enter scan\n");
    wait_while_strobe(1);
    
    //action here
    isScanning = 1;
    //
    
   //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

void rotate_90_enable()
{
    printf("set scanning to 90degrees\n");
    wait_while_strobe(1);
    
    //action here
    isScanning = 1;
    scanUpperLimit = 75;
    scanLowerLimit = 25;
    //
    
   //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

void rotate_180_enable()
{
    printf("set scanning to 18 0degrees\n");
    wait_while_strobe(1);
    
    //action here
    isScanning = 1;
    scanUpperLimit = 100;
    scanLowerLimit = 0;
    //
    
   //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

void disable_rotate()
{
    printf("set scanning to 18 0degrees\n");
    wait_while_strobe(1);
    
    //action here
    isScanning = 0;
    //
    
   //send the ACK message to the galileo
    send_nibble(MSG_ACK);
    
    wait_while_strobe(0);
    wait_while_strobe(1);
    wait_while_strobe(0);
}

//returns whatever nibble was on the bus
unsigned char return_bus_nibble()
{
    return (PORTCbits.RC5 << 3) &
           (PORTCbits.RC7 << 2) &
           (PORTCbits.RC1 << 1) &
           PORTCbits.RC3;
}
//expect to receive a message and return it
unsigned char receive_msg()
{
    unsigned results;
    
    //clear output Latch
    PORTC = 0;
    //Set strobe and the data pins as digital inputs //C 13567
    TRISC = 0b11101010;
    
    wait_while_strobe(1);
    //continuously read the bus while the strobe is low
    while(PORTCbits.RC6 == 0)
    {
        results = return_bus_nibble();
    }
    
    wait_while_strobe(1); //finish the bus cycle
    wait_while_strobe(0);
    
    return results;
}

//adjust the scan counter and bounce off of limits 
void handle_scan()
{
    //if timer 0 overflows
    if(PIR0bits.TMR0IF)
    {
       PIR0bits.TMR0IF = 0; //reset the overflow flag
       
       //adust scan counter
       if(isScanningUp)
           ++scanCounter;
       else
           --scanCounter;
       
       //set new duty
       set_pwm_duty(scanCounter);
       
       //bounce of boundaries
       if(scanCounter >= scanUpperLimit)
           isScanningUp = 0;
       else if(scanCounter <= scanLowerLimit)
           isScanningUp = 1;
    }
}

// Main program
void main (void)
{
    //init modules
    SYSTEM_Initialize();
    everything_init();

    //init state
    unsigned char msg = MSG_NOTHING;
    set_pwm_duty(50);
    isScanning = 0;
    scanCounter = 50;
    scanUpperLimit = 75;
    scanLowerLimit = 25;
    
    //test_scan();
    
    while(1)
    {  
        //handle Scanning
        if(isScanning)
        {
            PORTAbits.RA0 = 1;//light on if scanning
            handle_scan();
        } 
        else 
        {
           PORTAbits.RA0 = 0;
        }
        
        //receive a message and do something about it
        msg = receive_msg();
        
        if(msg == MSG_RESET) {
            sensor_reset();
        } else if(msg == MSG_PING) {
            sensor_ping();
        } else if(msg == MSG_GET) {
            send_adc_results();
        } else if(msg == MSG_TURN30) {
            turn_30();
        } else if(msg == MSG_TURN90) {
            turn_90();
        } else if(msg == MSG_TURN120) {
            turn_120();
        } else if(msg == MSG_ROTATE90_E) {
            rotate_90_enable();
        } else if(msg == MSG_ROTATE180_E) {
            rotate_180_enable();
        } else if(msg == MSG_ROTATE90_D) {
            disable_rotate();
        } else if(msg == MSG_ROTATE180_D) {
            disable_rotate();
        } else {
            //do nothing;
        }
        
    } // end inf loop
}

void test_scan()
{
    //init state
    unsigned char msg = MSG_NOTHING;
    set_pwm_duty(50);
    isScanning = 0;
    scanCounter = 50;
    scanUpperLimit = 75;
    scanLowerLimit = 25;
    
    int oldScanCounter = 50;
    
    while(1)
    {
        oldScanCounter = scanCounter;
        handle_scan();
        
        //if the scan counter gets updated, pulse A0
        if(scanCounter != oldScanCounter)
        {
            PORTAbits.RA0 = 1;
            __delay_ms(200);
        } else {
            PORTAbits.RA0 = 0;
        }
    }
}
/**
 End of File
*/
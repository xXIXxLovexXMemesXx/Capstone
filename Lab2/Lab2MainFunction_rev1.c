/*
 * File:   PIC and Galileo communication               
 * 
 * simple Galileo program example for main function
 * for UMass Lowell 16.480/552
 * 
 * Author: Jose Velis, Andy MacGregor,Grayson Colwell
 * Lab 2 main function Rev_1
 *
 * Created on 10/17/2017
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


//Linux GPIO number // Shield Pin Name
#define Strobe                  (40) // 8
#define GP_4                    (48) // A0
#define GP_5                    (50) // A1
#define GP_6                    (52) // A2
#define GP_7                    (54) // A3

#define GPIO_DIRECTION_IN       (1)//Go HIGH (acoording to handout timing diagram))
#define GPIO_DIRECTION_OUT      (0)//Go LOW (acoording to handout timing diagram))
#define HIGH                    (1)
#define LOW                     (0)

#define SUCCESS                 (0)
#define ERROR                   (-1)

/*User Commands*/
#define MSG_RESET   0x1
#define MSG_PING    0x2
#define MSG_GET     0x3
#define MSG_TURN30  0x4
#define MSG_TURN90  0x5
#define MSG_TURN120 0x6
#define MSG_ACK     0x7

//Constants
#define BUFFER_SIZE             (256)
#define EXPORT_FILE             "/sys/class/gpio/export"
/************************/

//open GPIO and set the direction
//returns pointer to string containing the gpio pin directory if successful
//  this needs to get freed after you're done with it
//returns null ptr on error
char* openGPIO(int gpio_handle, int direction )
{
    int n;
    //   1. export GPIO
    FILE* exportFh = fopen(EXPORT_FILE, "w");
    if(exportFh== NULL)
    {
        printf("Couldn't not open export file\n");
        fclose(exportFh);
        return NULL;
    }
    char numBuffer[5];
    snprintf(numBuffer, 5, "%d", gpio_handle);
    n = fputs(numBuffer, exportFh);
    fclose(exportFh);
    if(n < 0)
    {
        printf("error writing to export file\n");
        return NULL;
    }
    
    //form the file name of the newly created gpio directory
    char *gpioDirectory = malloc(BUFFER_SIZE);

    n = snprintf(gpioDirectory, BUFFER_SIZE, "/sys/class/gpio/gpio%d/", gpio_handle);
    if(n >= BUFFER_SIZE)
    {
        printf("Buffer overflow when creating directory name\n");
        free(gpioDirectory);
        return NULL;
    }

    //    2.set the direction
    char gpioDirection[BUFFER_SIZE];
    strcpy(gpioDirection, gpioDirectory);
    strcat(gpioDirection, "direction");
    FILE* directionFh = fopen(gpioDirection, "w");

    if(directionFh == NULL)
    {
        printf("gpio direction file is null %s \n", gpioDirection);
        free(gpioDirectory);
        fclose(directionFh);
        return NULL;
    }

    if(direction == GPIO_DIRECTION_OUT)
    {
        n = fputs("out", directionFh);
    }
    else if(direction == GPIO_DIRECTION_IN)
    {
        n = fputs("in", directionFh);
    }
    if(n < 0)
    {
        printf("Error writing to gpio direction file\n");
        free(gpioDirectory);
        fclose(directionFh);
        return NULL;
    }
    fclose(directionFh);

    //    3.set the votage
    char gpioValue[BUFFER_SIZE];
    strcpy(gpioValue, gpioDirectory);
    strcat(gpioValue, "value");
    FILE* valueFh = fopen(gpioValue, "w");
    n = fputs("1", valueFh);
    fclose(valueFh);
    if(n < 0)
    {
        printf("Error writing to gpio value file\n");
        free(gpioDirectory);
        return NULL;
    }

    //return the new gpio directory
    return gpioDirectory;
}

//write value (HIGH or LOW) to port specified
//returns value written on success
//returns ERROR (negative) on failure
int writeGPIO(char* gpioDirectory, int value)
{
    char gpioValue[BUFFER_SIZE];
    strcpy(gpioValue, gpioDirectory);
    strcat(gpioValue, "value");
    FILE* valueFh = fopen(gpioValue, "w");

    char numBuffer[5];
    snprintf(numBuffer, 5, "%d", value);

    int n = fputs(numBuffer, valueFh);
    fclose(valueFh);
    if(n < 0)
    {
        printf("Error writin to gpio value file in %s", gpioDirectory);
       
        return ERROR;
    }

    return value;
}

//Sets the GPIO pin specified to a new direction
//returns the direction set on success
//returns ERROR(negative) on failure
int setGPIODirection(char* gpioDirectory, int direction)
{
    char gpioDirection[BUFFER_SIZE];
    strcpy(gpioDirection, gpioDirectory);
    strcat(gpioDirection, "direction");
    FILE* directionFh = fopen(gpioDirection, "w");

    int n = -1;
    if(direction == GPIO_DIRECTION_IN)
    {
        n = fputs("in", directionFh);
    }
    else if(direction == GPIO_DIRECTION_OUT)
    {
        n = fputs("out", directionFh);
    }

    fclose(directionFh);
    if(n < 0)
    {
        printf("Error writin to gpio value file in %s", gpioDirectory);
        return ERROR;
    }

    return direction;
}

//read value (HIGH or LOW) from port specified
//port direction must be set to input.
//returns port value on success
//returns ERROR (negative) on failure
int readGPIO(char* gpioDirectory)
{
    char gpioValue[BUFFER_SIZE];
    strcpy(gpioValue, gpioDirectory);
    strcat(gpioValue, "value");
    FILE* valueFh = fopen(gpioValue, "r");

    char numBuffer[5];
    char* test = fgets(numBuffer, 5, valueFh);
    fclose(valueFh);
    if(test == NULL)
    {
        printf("Error reading from gpio value %s", gpioDirectory);
        return ERROR;
    }

    return atoi(numBuffer);
}

//Sends a nibble(4 bytes) along the bus following the Bus Protocol.
//does not wait for an ACK.
void writeNibble(unsigned char data,
    char* d0,
    char* d1,
    char* d2,
    char* d3,
    char* strobe)
{
    //set all the ports to output
    setGPIODirection(d0, GPIO_DIRECTION_OUT);
    setGPIODirection(d1, GPIO_DIRECTION_OUT);
    setGPIODirection(d2, GPIO_DIRECTION_OUT);
    setGPIODirection(d3, GPIO_DIRECTION_OUT);
    setGPIODirection(strobe, GPIO_DIRECTION_OUT);

    //start the bus protocol
    //1: pull strobe low
    writeGPIO(strobe, LOW);

    //2: output the nibble to the bus
    writeGPIO(d0, data & 1);
    writeGPIO(d1, (data & 2) >> 1);
    writeGPIO(d2, (data & 4) >> 2);
    writeGPIO(d3, (data & 8) >> 3);

    //3: raise strobe and wait at least 10ms
    writeGPIO(strobe, HIGH);
    usleep(10);

    //4: Pull strobe low again
    writeGPIO(strobe, LOW);

    //5: clear the bus
    writeGPIO(d0, LOW);
    writeGPIO(d1, LOW);
    writeGPIO(d2, LOW);
    writeGPIO(d3, LOW);
}

//Reads a 4 bit nibble from the bus following the protocol
//returns the nibble in the lower 4 bits of the return value
//returns a negative on error
int readNibble(char* d0,
    char* d1,
    char* d2,
    char* d3,
    char* strobe)
{
    unsigned char data = 0x00;
    //set all the data ports to input, but the strobe to output
    setGPIODirection(d0, GPIO_DIRECTION_IN);
    setGPIODirection(d1, GPIO_DIRECTION_IN);
    setGPIODirection(d2, GPIO_DIRECTION_IN);
    setGPIODirection(d3, GPIO_DIRECTION_IN);
    setGPIODirection(strobe, GPIO_DIRECTION_OUT);

    //start the bus protocol
    //1: pull strobe low to signal the start of the read
    writeGPIO(strobe, LOW);

    //2: the PIC should output to the bus now. We give it 10ms
    usleep(10);

    //3: raise strobe and start reading the value from the data bus
    writeGPIO(strobe, HIGH);
    data += readGPIO(d0);
    data += readGPIO(d1) << 1;
    data += readGPIO(d2) << 2;
    data += readGPIO(d3) << 3;

    if(data > 0xF)
    {
        printf("Error reading nibble from the bus");
        return ERROR;
    }
    //4: Pull strobe low again to signal that data has been read
    writeGPIO(strobe, LOW);

    //5: the PIC will clear the bus
    return (int)data;
}

// tests the GPIO write and exits
// connect a scope or something to the strobe port and watch for output
void testGPIOWrite(char * fh)
{
    int i;
    //test read and write.
    for(i =0; i <1000; i++)
    {
        int w = writeGPIO(fh, HIGH);
        assert(w == HIGH && "Write high");
        usleep(10);
        w = writeGPIO(fh, LOW);
        assert(w == LOW && "Write Low");
        usleep(10);
    }
    exit(0);
}

// tests the GPIO writenibble and exits
//repeatedly sends nibbles from 0x0 to 0xF
//How to test: Connect to logic analyzer, watch values
void testGPIOWriteNibble(char* strobe_fh,
                            char* d4, //48
                            char* d5, //50
                            char* d6, //52
                            char* d7) //54 
{
    unsigned i;
    //test writeNibble
    for( i =0; i <1000; i++)
    {
        printf("%X\n",i % 0xF);
        writeNibble( i %0xF, d4, d5, d6, d7, strobe_fh);
        usleep(20);
    }
    exit(0);
}

//Functions definitions - for commands
void reset();
void ping();
void adc_value();
void servo_30();
void servo_90();
void servo_120();


//File handles for the pins
char* fileHandleGPIO_4;
char* fileHandleGPIO_5;
char* fileHandleGPIO_6;
char* fileHandleGPIO_7;
char* fileHandleGPIO_S;  //Should these 5 variables be used globally? (jk you're right they should be global)- 

int main(void)
{

    fileHandleGPIO_4 = openGPIO(GP_4, GPIO_DIRECTION_OUT);
    fileHandleGPIO_5 = openGPIO(GP_5, GPIO_DIRECTION_OUT);
    fileHandleGPIO_6 = openGPIO(GP_6, GPIO_DIRECTION_OUT);
    fileHandleGPIO_7 = openGPIO(GP_7, GPIO_DIRECTION_OUT);
    fileHandleGPIO_S = openGPIO(Strobe, GPIO_DIRECTION_OUT);
    /*Line 92 sets the strobe to LOW as shown in figure 1 of the lab manual. 
    The data lines are also set LOW (lines 99-102) to assure that
    they aren't sending any data */ 
    
    while(1)
    {
    /*For line 106, I'm not sure what the best way to handle 
    the write to of the GPIO is since a #define directive was 
    used (lines 28 & 29 ) to pick high or low. Can you send a
    #define variable and compare it to a normal variable? Even 
    if you can do that, how to physically change direction?
    
    2.write data... (where the menu selection is used? and a bunch of 
    other stuff?))*/

        int input;
        int scanf_test;
        do{
            printf("Select a number for desired action: \n\n");
            printf("1. Reset\n");
            printf("2. Ping\n");
            printf("3. Get ADC value");
            printf("4. Turn Servo 30 degrees\n");
            printf("5. Turn Servo 90 degrees\n");
            printf("6. Turn Servo 120 degrees\n");
            printf("7. Exit\n");

            //check for input. If ipmroperly formatted,
            //  set input to 0 to prompt user to input again
            scanf_test = scanf("%d", &input);
            if(scanf_test == 0)
                input = 0;

            switch (input)
            {
                case 1 :
                    reset();
                    break;
                case 2  :
                    ping();
                    break;
                case 3  :
                    adc_value();
                    break;
                case 4  :
                    servo_30();
                    break;
                case 5  :
                    servo_90();
                    break;
                case 6  :
                    servo_120();
                    break;
                default :
                    printf("Please enter a valid number (1 - 6)\n");
                    break;
            }
        }while(input != 7); 
    }
}


//stubs for the command functions
void reset()
{
    int receive_msg = 0;
    while(receive_msg != MSG_ACK)
    {
        writeNibble(MSG_RESET, 
                    fileHandleGPIO_4,
                    fileHandleGPIO_5,
                    fileHandleGPIO_6,
                    fileHandleGPIO_7,
                    fileHandleGPIO_S);
        usleep(30);
        receive_msg = readNibble(fileHandleGPIO_4,
                                fileHandleGPIO_5,
                                fileHandleGPIO_6,
                                fileHandleGPIO_7,
                                fileHandleGPIO_S);
    }
    printf("Reset message sent");
}

void ping()
{
    int receive_msg = 0;
    while(receive_msg != MSG_ACK)
    {
        writeNibble(MSG_PING, 
                    fileHandleGPIO_4,
                    fileHandleGPIO_5,
                    fileHandleGPIO_6,
                    fileHandleGPIO_7,
                    fileHandleGPIO_S);
        usleep(30);
        receive_msg = readNibble(fileHandleGPIO_4,
                                fileHandleGPIO_5,
                                fileHandleGPIO_6,
                                fileHandleGPIO_7,
                                fileHandleGPIO_S);
    }
    printf("Ping message sent");
}

//requests the PIC to send its current adc value MSN (most significant nibble) first
void adc_value()
{
    int i;
    int receive_msg = 0;
    int adc_value = 0;
    while(receive_msg != MSG_ACK)
    {
        writeNibble(MSG_GET, 
                    fileHandleGPIO_4,
                    fileHandleGPIO_5,
                    fileHandleGPIO_6,
                    fileHandleGPIO_7,
                    fileHandleGPIO_S);
        //expect to receive 3 messages containing ADC values, msn first
        usleep(30);
        for(i = 8; i >= 0; i -= 4)
        {
            receive_msg = readNibble(fileHandleGPIO_4,
                                    fileHandleGPIO_5,
                                    fileHandleGPIO_6,
                                    fileHandleGPIO_7,
                                    fileHandleGPIO_S);
            adc_value += ((unsigned) receive_msg) << i;
            usleep(30);
        }
        //expect one last ACK message
        receive_msg = readNibble(fileHandleGPIO_4,
                                fileHandleGPIO_5,
                                fileHandleGPIO_6,
                                fileHandleGPIO_7,
                                fileHandleGPIO_S);
    }
    printf("adc message received successfully: %x", adc_value);
}

void servo_30()
{
    int receive_msg = 0;
    while(receive_msg != MSG_ACK)
    {
        writeNibble(MSG_TURN30, 
                    fileHandleGPIO_4,
                    fileHandleGPIO_5,
                    fileHandleGPIO_6,
                    fileHandleGPIO_7,
                    fileHandleGPIO_S);
        usleep(30);
        receive_msg = readNibble(fileHandleGPIO_4,
                                fileHandleGPIO_5,
                                fileHandleGPIO_6,
                                fileHandleGPIO_7,
                                fileHandleGPIO_S);
    }
    printf("servo_30 message sent");
}

void servo_90()
{
    int receive_msg = 0;
    while(receive_msg != MSG_ACK)
    {
        writeNibble(MSG_TURN90, 
                    fileHandleGPIO_4,
                    fileHandleGPIO_5,
                    fileHandleGPIO_6,
                    fileHandleGPIO_7,
                    fileHandleGPIO_S);
        usleep(30);
        receive_msg = readNibble(fileHandleGPIO_4,
                                fileHandleGPIO_5,
                                fileHandleGPIO_6,
                                fileHandleGPIO_7,
                                fileHandleGPIO_S);
    }
    printf("Servo_90 message sent");
}

void servo_120()
{
    int receive_msg = 0;
    while(receive_msg != MSG_ACK)
    {
        writeNibble(MSG_TURN120, 
                    fileHandleGPIO_4,
                    fileHandleGPIO_5,
                    fileHandleGPIO_6,
                    fileHandleGPIO_7,
                    fileHandleGPIO_S);
        usleep(30);
        receive_msg = readNibble(fileHandleGPIO_4,
                                fileHandleGPIO_5,
                                fileHandleGPIO_6,
                                fileHandleGPIO_7,
                                fileHandleGPIO_S);
    }
    printf("Servo_120 message sent");
}
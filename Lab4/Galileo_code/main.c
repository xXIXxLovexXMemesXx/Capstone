#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "net.h"
#include "sensor.h"
#include "mraa.hpp"
#include "camera.h"
#include "camera.cpp"
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>


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

#define GPIO_MODE_PULLUP        (1)
#define GPIO_MODE_PULLDOWN      (2)
#define GPIO_MODE_HIZ           (3)
#define GPIO_MODE_STRONG        (4)

#define SUCCESS                 (0)
#define ERROR                   (-1)

/*User Commands*/
#define MSG_RESET   0x0
#define MSG_PING    0x1
#define MSG_GET     0x2
#define MSG_TURN30  0x3
#define MSG_TURN90  0x4
#define MSG_TURN120 0x5
#define MSG_ROTATE90_E 0x6
#define MSG_Rotate90_D 0x8
#define MSG_ROTATE180_E 0x7
#define MSG_ROTATE180_D 0x9

#define MSG_ACK     0xE
#define MSG_NOTHING 0xF

//Constants
#define BUFFER_SIZE             (256)
#define EXPORT_FILE             "/sys/class/gpio/export"
#define STROBE_DELAY            (1000*20) //10 ms in us
#define _CRT_SECURE_NO_WARNINGS 

//hold the data to send to the server
server_data myData;
pthread_mutex_t myData_m;
const int addr = 0x32;

//Functions definitions - for commands
void reset();
void ping();
void adc_value();
void servo_30();
void servo_90();
void servo_120();
void rotate_servo_90_E();
void rotate_servo_180_E();
void rotate_servo_90_D();
void rotate_servo_180_D();
void commandloop();
void sensor_control();

//File handles for the pins
char* fileHandleGPIO_4;
char* fileHandleGPIO_5;
char* fileHandleGPIO_6;
char* fileHandleGPIO_7;
char* fileHandleGPIO_S;

//variable for scan mode
int scan_solo;


int main()
{
  ////init shared stuff
  pthread_mutex_init(&myData_m, NULL);
  //init data structure
  myData.picOnline = false; //true if pic is online
  myData.adcData = 0; //value of PIC adc
  strncpy(myData.fileName, DEFAULT_FILENAME, MAX_FILENAME); //filename of last 

  ////create threads
  pthread_t command_thread;
  pthread_t sensor_control_thread;
  pthread_t net_thread;

  ////run threads
  pthread_create(&command_thread, NULL,commandLoop, NULL);
  pthread_create(&sensor_control_thread, NULL, sensor_control, NULL);
  pthread_create(&net_thread, NULL, serverPostLoop, NULL);
 
  //jump out
  pthread_exit(NULL);
}



void commandloop()
{

	fileHandleGPIO_4 = openGPIO(GP_4, GPIO_DIRECTION_OUT);
	fileHandleGPIO_5 = openGPIO(GP_5, GPIO_DIRECTION_OUT);
	fileHandleGPIO_6 = openGPIO(GP_6, GPIO_DIRECTION_OUT);
	fileHandleGPIO_7 = openGPIO(GP_7, GPIO_DIRECTION_OUT);
	fileHandleGPIO_S = openGPIO(Strobe, GPIO_DIRECTION_OUT);

	int input;
	int temp_thresh_input;
	int scanf_test;
	double temp;

	do {

		printf("Select a number for desired action: \n\n");
		printf("1. Reset\n");
		printf("2. Ping\n");
		printf("3. Get ADC value\n");
		printf("4. Turn Servo 30 degrees\n");
		printf("5. Turn Servo 90 degrees\n");
		printf("6. Turn Servo 120 degrees\n");
		printf("7. Rotate the Servo continuously 90 degrees enable\n");
		printf("8. Rotate the Servo continously 180 degrees enable\n");
		printf("9. Rotate the Servo continuously 90 degrees disable\n");
		printf("10. Rotate the Servo continuously 180 degrees disable\n");
		printf("11. Exit\n");
		temp = get_temp();
		printf("Current Temperature: %lf", temp);
		//check for input. 
		input = 0;
		scanf_test = scanf("%d", &input);
		//If ipmroperly formatted,
		//  set input to 0 to prompt user to input again
		if (scanf_test == 0)
			input = 0;
		switch (input)
		{
		case 1:
			reset();
			break;
		case 2:
			ping();
			break;
		case 3:
			pthread_mutex_lock(&myData_m);
			printf("LDR Value = %d", LDR_Value);
			pthread_mutex_unlock(&myData_m);
			break;
		case 4:
			servo_30();
			break;
		case 5:
			servo_90();
			break;
		case 6:
			servo_120();
			break;
		case 7:
			rotate_servo_90_e();
			pthread_mutex_lock(&myData_m);
			scan_solo = 1;
			pthread_mutex_unlock(&myData_m);
			break;
		case 8:
			rotate_servo_180_e();
			pthread_mutex_lock(&myData_m);
			scan_solo = 1;
			pthread_mutex_unlock(&myData_m);
			break;
		case 9:
			rotate_servo_90_d;
			pthread_mutex_lock(&myData_m);
			scan_solo = 0;
			pthread_mutex_unlock(&myData_m);
			break;
		case 10:
			rotate_servo_180_d;
			pthread_mutex_lock(&myData_m);
			scan_solo = 0;
			pthread_mutex_unlock(&myData_m);
			break;
		default:
			printf("Please enter a valid number (1 - 10)\n");
			break;
		}
		//print the current temperature reading
		pthread_mutex_lock(&myData_m);
		printf("Current Temperature reading: %lf", cur_temp);
		pthread_mutex_unlock(&myData_m);
		//ask the user to change the temp threshold
		printf("Would you like to change the Temperature Threshold?\n (1 = yes)\n");
		scanf(" %d", &temp_thresh_input);
		if (temp_thresh_input == 1)
		{
			pthread_mutex_lock(&myData_m);
			printf("Enter the new Temperature Threshold:\n");
			scanf("%lf", &temperature_threshold);
			pthread_mutex_unlock(&myData_m);
		}
	} while (input != 11);
}

void sensor_control()
{
	while (1)
	{
		time_t date = time(NULL);
		char* cdate;
		pthread_mutex_lock(&mydata_m);
		LDR_Value = adc_value();
		cur_temp = get_temp();
		if((cur_temp > temperature_threshold) && (scan_solo == 1)
		{
			date = time(NULL);
			cdate = asctime(localtime(&date));
			for(int i = 0; i < 25; i++)
			{
				if (cdate[i] == 32)
					{
						cdate[i] = '_';
					}
					else if (cdate[i] == 58)
					{
						cdate[i] = '_';
					}
					else if (cdate[i] == 10)
					{
						cdate[i] = '_';
					}
			}
			printf("Taking picture: %s \n", cdate);
			capture_and_save_image(cdate);
		}	
		pthread_mutex_unlock(&mydata_m);
		sleep(2)


	}
}

//returns a copy of the current state
server_data getCurrentState()
{
  server_data s;

  pthread_mutex_lock(&myData_m);
  memcpy(&s, &myData, sizeof(server_data));
  pthread_mutex_unlock(&myData_m);

  return s;
}

//returns the mode set on success
//returns ERROR(negative) on failure
int setGPIOMode(char* gpioDirectory, int mode)
{
	//find Drive file
	char gpioDrive[BUFFER_SIZE];
	strcpy(gpioDrive, gpioDirectory);
	strcat(gpioDrive, "drive");
	FILE* driveFh = fopen(gpioDrive, "w");

	int n = -1;
	if (mode == GPIO_MODE_HIZ)
	{
		n = fputs("hiz", driveFh);
	}
	else if (mode == GPIO_MODE_STRONG)
	{
		n = fputs("strong", driveFh);
	}
	else if (mode == GPIO_MODE_PULLUP)
	{
		n = fputs("pullup", driveFh);
	}
	else if (mode == GPIO_MODE_PULLDOWN)
	{
		n = fputs("pulldown", driveFh);
	}

	fclose(driveFh);
	if (n < 0)
	{
		printf("Error writing to gpio drive file in %s", gpioDirectory);
		return ERROR;
	}

	return mode;
}
//Sets the GPIO pin specified to a new direction
// ALSO sets the mode of the pin.
//returns the direction set on success
//returns ERROR(negative) on failure
int setGPIODirection(char* gpioDirectory, int direction)
{
	//find direciton file
	char gpioDirection[BUFFER_SIZE];
	strcpy(gpioDirection, gpioDirectory);
	strcat(gpioDirection, "direction");
	FILE* directionFh = fopen(gpioDirection, "w");

	//find Drive file
	char gpioDrive[BUFFER_SIZE];
	strcpy(gpioDrive, gpioDirectory);
	strcat(gpioDrive, "drive");
	FILE* driveFh = fopen(gpioDrive, "w");

	int n = -1;
	int m = -1;
	if (direction == GPIO_DIRECTION_IN)
	{
		n = fputs("in", directionFh);
		m = fputs("hiz", driveFh);
	}
	else if (direction == GPIO_DIRECTION_OUT)
	{
		n = fputs("out", directionFh);
		m = fputs("strong", driveFh);
	}

	fclose(driveFh);
	fclose(directionFh);
	if (n < 0 || m < 0)
	{
		printf("Error writing to gpio value file in %s", gpioDirectory);
		return ERROR;
	}

	return direction;
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
	if (n < 0)
	{
		printf("Error writin to gpio value file in %s", gpioDirectory);

		return ERROR;
	}

	return value;
}

//open GPIO and set the direction
//returns pointer to string containing the gpio pin directory if successful
//  this needs to get freed after you're done with it
//returns null ptr on error
char* openGPIO(int gpio_handle, int direction)
{
	int n;
	//   1. export GPIO
	FILE* exportFh = fopen(EXPORT_FILE, "w");
	if (exportFh == NULL)
	{
		printf("Couldn't open export file\n");
		fclose(exportFh);
		return NULL;
	}
	char numBuffer[5];
	snprintf(numBuffer, 5, "%d", gpio_handle);
	n = fputs(numBuffer, exportFh);
	fclose(exportFh);
	if (n < 0)
	{
		printf("error writing to export file\n");
		return NULL;
	}

	//form the file name of the newly created gpio directory
	char *gpioDirectory = malloc(BUFFER_SIZE);

	n = snprintf(gpioDirectory, BUFFER_SIZE, "/sys/class/gpio/gpio%d/", gpio_handle);
	if (n >= BUFFER_SIZE)
	{
		printf("Buffer overflow when creating directory name\n");
		free(gpioDirectory);
		return NULL;
	}

	//    2.set the direction
	setGPIODirection(gpioDirectory, direction);

	//    3.set the voltage
	writeGPIO(gpioDirectory, HIGH);

	//return the new gpio directory
	return gpioDirectory;
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
	if (test == NULL)
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
	usleep(STROBE_DELAY);

	//4: Pull strobe low again
	writeGPIO(strobe, LOW);
	usleep(STROBE_DELAY); //and delay a little bit

						  //5: clear the bus
	writeGPIO(d0, LOW);
	writeGPIO(d1, LOW);
	writeGPIO(d2, LOW);
	writeGPIO(d3, LOW);

	//....let the bus float high again
	writeGPIO(strobe, HIGH);
	usleep(STROBE_DELAY); //and delay a little bit
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
	int test = 1;
	//set all the data ports to input, but the strobe to output
	setGPIODirection(d0, GPIO_DIRECTION_IN);
	setGPIODirection(d1, GPIO_DIRECTION_IN);
	setGPIODirection(d2, GPIO_DIRECTION_IN);
	setGPIODirection(d3, GPIO_DIRECTION_IN);

	//start the bus protocol
	//1: pull strobe low to signal the start of the read
	writeGPIO(strobe, LOW);

	//2: the PIC should output to the bus now. 

	//3: We give it 10ms
	usleep(STROBE_DELAY);

	//4: raise strobe and start reading the value from the data bus
	writeGPIO(strobe, HIGH);
	test = readGPIO(d0);
	if (test == ERROR)
		return ERROR;
	data += test;

	test = readGPIO(d1);
	if (test == ERROR)
		return ERROR;
	data += test << 1;

	test = readGPIO(d2);
	if (test == ERROR)
		return ERROR;
	data += test << 2;

	test = readGPIO(d3);
	if (test == ERROR)
		return ERROR;

	data += test << 3;

	if (data > 0xF)
	{
		printf("Uncaught error reading nibble from the bus");
		return ERROR;
	}
	//leave strobe high for a bit
	usleep(STROBE_DELAY);

	//4: Pull strobe low again to signal that data has been read
	writeGPIO(strobe, LOW);
	usleep(STROBE_DELAY);
	//5: the PIC will clear the bus

	//....let the bus float high again
	writeGPIO(strobe, HIGH);
	usleep(STROBE_DELAY); //and delay a little bit
	return (int)data;
}

// tests the GPIO write and exits
// connect a scope or something to the strobe port and watch for output
void testGPIOWrite(char * fh)
{
	int i;
	//test read and write.
	for (i = 0; i <1000; i++)
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
	for (i = 0; i <1000; i++)
	{
		printf("%X\n", i % 0xF);
		writeNibble(i % 0xF, d4, d5, d6, d7, strobe_fh);
		usleep(20);
	}
	exit(0);
}

// tests the GPIO readnibble
void testGPIOReadNibble(char* strobe_fh,
	char* d4, //48
	char* d5, //50
	char* d6, //52
	char* d7) //54 
{
	unsigned i;
	unsigned data;
	//test writeNibble
	for (i = 0; i <1000; i++)
	{
		data = readNibble(d4, d5, d6, d7, strobe_fh);
		printf("read: %X\n", data);

		usleep(STROBE_DELAY);
	}
	exit(0);
}



//stubs for the command functions
void reset()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		printf("Starting to send reset\n");
		writeNibble(MSG_RESET,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote Nibble to line\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("Reset message sent\n");
}

void ping()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		printf("Starting to send ping\n");
		writeNibble(MSG_PING,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote ping to bus\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("Ping message sent\n");
}

//requests the PIC to send its current adc value MSN (most significant nibble) first
int adc_value()
{
	int i;
	int receive_msg = 0;
	int adc_value = 0;
	while (receive_msg != MSG_ACK)
	{
		adc_value = 0;
		receive_msg = 0;
		printf("Starting to send ADC_request\n");
		writeNibble(MSG_GET,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);

		//expect to receive 3 messages containing ADC values, msn first
		usleep(STROBE_DELAY);
		for (i = 8; i >= 0; i -= 4)
		{
			receive_msg = readNibble(fileHandleGPIO_4,
				fileHandleGPIO_5,
				fileHandleGPIO_6,
				fileHandleGPIO_7,
				fileHandleGPIO_S);
			adc_value += ((unsigned)receive_msg) << i;
			printf("Received ADC Nibble: 0x%x\n", receive_msg);
			usleep(STROBE_DELAY);
		}
		//expect one last ACK message
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received ADC ACK(?): 0x%x\n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("adc message received successfully: 0x%x\n", adc_value);
	return adc_value;
}

void servo_30()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		printf("Starting to send Servo30\n");
		writeNibble(MSG_TURN30,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote Nibble to Line Servo30\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("servo_30 message sent\n");
}


void servo_90()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		printf("Starting to send Servo90\n");
		writeNibble(MSG_TURN90,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote Nibble to bus\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("Servo_90 message sent\n");
}

void servo_120()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		writeNibble(MSG_TURN120,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote Nibble to bus\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("Servo_120 message sent\n");
}

void rotate_servo_90_E()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		printf("Starting to send Rotate Servo90\n");
		writeNibble(MSG_ROTATE90,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote Nibble to Line Rotate Servo90\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("rotate_servo_90 message sent\n");
}

void rotate_servo_180_E()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		printf("Starting to send Rotate Servo180\n");
		writeNibble(MSG_ROTATE180,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote Nibble to Line Rotate Servo180\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("rotate_servo_180 message sent\n");
}

void rotate_servo_90_D()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		printf("Starting to send Rotate Servo90 disable\n");
		writeNibble(MSG_ROTATE90,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote Nibble to Line Rotate Servo90 disable\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("rotate_servo_90 disable message sent\n");
}

void rotate_servo_180_D()
{
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		printf("Starting to send Rotate Servo180 disable\n");
		writeNibble(MSG_ROTATE180,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Wrote Nibble to Line Rotate Servo180 disable \n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("rotate_servo_180 disable message sent\n");
}
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "net.h"
#include "sensor.h"
#include "mraa.hpp"
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>


//Linux GPIO number // Shield Pin Name
#define Strobe                  (40) // 8		white
#define GP_4                    (48) // A0	yellow
#define GP_5                    (50) // A1	green
#define GP_6                    (52) // A2	blue
#define GP_7                    (54) // A3	red

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
#define MSG_ROTATE90_D 0x8
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
//protect all data with myData_m
server_data myData;
pthread_mutex_t myData_m;
double temperature_threshold = 30;
double cur_temp = -100; //set to invalid value

//Functions definitions - for commands
void reset();
int ping(unsigned timeout);
int adc_value();
void servo_30();
void servo_90();
void servo_120();
void rotate_servo_90_E();
void rotate_servo_180_E();
void rotate_servo_90_D();
void rotate_servo_180_D();
void* commandLoop(void *);
void* sensor_control(void *);

//bus output functions
int setGPIOMode(char* gpioDirectory, int mode);
int setGPIODirection(char* gpioDirectory, int direction);
int writeGPIO(char* gpioDirectory, int value);
char* openGPIO(int gpio_handle, int direction);
int readGPIO(char* gpioDirectory);
void writeNibble(unsigned char data, char* d0, char* d1, char* d2, char* d3, char* strobe);
int readNibble(char* d0, char* d1, char* d2,char* d3, char* strobe);

//File handles for the pins
char* fileHandleGPIO_4;
char* fileHandleGPIO_5;
char* fileHandleGPIO_6;
char* fileHandleGPIO_7;
char* fileHandleGPIO_S;
pthread_mutex_t gpio_m;

//variable for scan mode
int scan_solo;

//tests
//pass
void testMyBus()
{
	int input = 0;
	int scanf_test = 0;
	while(input != 1)
	{
		printf("Ping? or not. 1 = quit\n");
		input = 0;
		scanf_test = scanf("%d", &input);

		if (scanf_test == 0)
			input = 0;

		if(input == 1)
			return;
		int i = ping(100);
		printf("Ping return: %x\n", i);
	}
}

int main()
{
  ////init shared stuff
  pthread_mutex_init(&myData_m, NULL);
  pthread_mutex_init(&gpio_m, NULL);

  //init data structure
  myData.picOnline = false; //true if pic is online
  myData.adcData = 0; //value of PIC adc
  strncpy(myData.fileName, DEFAULT_FILENAME, MAX_FILENAME); //filename of last 

  //init GPIO pins
  fileHandleGPIO_4 = openGPIO(GP_4, GPIO_DIRECTION_OUT);
	fileHandleGPIO_5 = openGPIO(GP_5, GPIO_DIRECTION_OUT);
	fileHandleGPIO_6 = openGPIO(GP_6, GPIO_DIRECTION_OUT);
	fileHandleGPIO_7 = openGPIO(GP_7, GPIO_DIRECTION_OUT);
	fileHandleGPIO_S = openGPIO(Strobe, GPIO_DIRECTION_OUT);

	//test
	//testMyBus();

  ////create threads
  pthread_t command_thread;
  pthread_t sensor_control_thread;
  pthread_t net_thread;

  ////run threads
  pthread_create(&command_thread, NULL,commandLoop, NULL);
  pthread_create(&sensor_control_thread, NULL, sensor_control, NULL);
  pthread_create(&net_thread, NULL, serverPostLoop, NULL);
 

  pthread_join(command_thread, NULL);

  //jump out if the command loop exits
  printf("Main thread stopped\n");
  pthread_exit(NULL);
}



void* commandLoop(void*)
{
	int input;
	int temp_thresh_input;
	int scanf_test;
	double temp;
	int ping_ret; //return value of ping function

	do {
		//print the main menu
		printf("(1)Select a number for desired action: \n\n");
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

		//print the temp if a valid reading is obtained
		pthread_mutex_lock(&myData_m);
		if(cur_temp > -50) 
			printf("(1)Current Temperature: %lf\n", cur_temp);
		pthread_mutex_unlock(&myData_m);
		
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
			ping_ret = ping(10);

			//update pic status 
			pthread_mutex_lock(&myData_m);
			if(ping_ret == MSG_ACK)
				myData.picOnline = true;
			else
				myData.picOnline = false;
			pthread_mutex_unlock(&myData_m);

			break;
		case 3:
			//print adc value collected from sensor loop
			pthread_mutex_lock(&myData_m);
			printf("LDR Value = %d\n", myData.adcData);
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
			rotate_servo_90_E();
			pthread_mutex_lock(&myData_m);
			scan_solo = 1;
			pthread_mutex_unlock(&myData_m);
			break;
		case 8:
			rotate_servo_180_E();
			pthread_mutex_lock(&myData_m);
			scan_solo = 1;
			pthread_mutex_unlock(&myData_m);
			break;
		case 9:
			rotate_servo_90_D();
			pthread_mutex_lock(&myData_m);
			scan_solo = 0;
			pthread_mutex_unlock(&myData_m);
			break;
		case 10:
			rotate_servo_180_D();
			pthread_mutex_lock(&myData_m);
			scan_solo = 0;
			pthread_mutex_unlock(&myData_m);
			break;
		case 11:
			return NULL;
		default:
			printf("Please enter a valid number (1 - 10)\n");
			break;
		}
		//print the current temperature reading
		pthread_mutex_lock(&myData_m);
		printf("Current Temperature settings:\n\ttemp = %lf\n\tthreshold = %lf\n", cur_temp, temperature_threshold);
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


//thread 2 of the system.
//Every two seconds, it probes the LDR via custom bus and temp sensor via i2c.
//	and updates the shared variables.
//if the temp sensor is greater than the set temp threshold and you're in scan mode...
//	take a picture and save it to file.
//	save the filename in shared memory to upload to server
void* sensor_control(void *)
{
	while (1)
	{
		time_t date = time(NULL);
		char* cdate;

		//probe the buses for sensor data. Use temp variables to avoid hold and wait
		//adc_value is mutex protected itself
		//printf("(2)Calling adc_value\n");
		int tmpLdr = adc_value();

		//probe temp sensor on the i2c bus
		//printf("(2) Calling get Temp.");
		double tmpTemp = get_temp();

		//printf("(2)Read temp: %lf, and ADC: %d\n", tmpTemp, tmpLdr);
		//update shared state variables
		pthread_mutex_lock(&myData_m);
		myData.adcData = tmpLdr;
		cur_temp = tmpTemp;
		pthread_mutex_unlock(&myData_m);

		//take a picture if above the temp threshold and in scan mode
		if((cur_temp > temperature_threshold) && (scan_solo == 1))
		{
			//form the filename. Filter out spaces, colons and newlines
			date = time(NULL);
			cdate = asctime(localtime(&date));
			for(int i = 0; i < 25; i++)
			{
				if (cdate[i] == ' ')
					{
						cdate[i] = '_';
					}
					else if (cdate[i] == ':')
					{
						cdate[i] = '_';
					}
					else if (cdate[i] == '\n')
					{
						cdate[i] = '_';
					}
			}
			printf("(2)Taking picture: %s \n", cdate);
			capture_and_save_image(cdate);

			//update latest filename
			pthread_mutex_lock(&myData_m);
			strcpy(myData.fileName, cdate);
			pthread_mutex_unlock(&myData_m);
		}	
		
		sleep(2);

	}
}

//returns a copy of the current state to upload to server
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
	char *gpioDirectory = (char *) malloc(BUFFER_SIZE);

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
	usleep(STROBE_DELAY);

	//3: raise strobe and wait at least 10ms for PIC to read it
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
	{
		return ERROR;
	}
	data += test;

	test = readGPIO(d1);
	if (test == ERROR)
	{
		return ERROR;
	}
	data += test << 1;

	test = readGPIO(d2);
	if (test == ERROR)
	{
		return ERROR;
	}
	data += test << 2;

	test = readGPIO(d3);
	if (test == ERROR)
	{
		return ERROR;
	}

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


//command functions
void reset()
{
	//printf("Waiting for custBUs mutex for reset\n");
	//pthread_mutex_lock(&gpio_m);
	int receive_msg = 0;
	while (receive_msg != MSG_ACK)
	{
		//printf("Starting to send reset\n");
		writeNibble(MSG_RESET,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote Nibble to line\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	printf("Reset message sent\n");
	pthread_mutex_unlock(&gpio_m);
}

//tries pinging the bus timeout-times.
//if successful returns MSG_ACK.
int ping(unsigned timeout)
{
	//printf("Waiting for custBUs mutex for ping\n");
	pthread_mutex_lock(&gpio_m);
	//printf("Aquired the mutex for ping\n");
	int receive_msg = MSG_NOTHING;

	//timeout after 10 attempts
	for (int i = 0; (i < 10) && (receive_msg != MSG_ACK); i++)
	{
		//printf("Writing ping\n");
		writeNibble(MSG_PING,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote ping to bus\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY *2); //wait a little extra before trying again
	}
	pthread_mutex_unlock(&gpio_m);
	if(receive_msg == MSG_ACK)
	{
		printf("Ping message sent successfully\n");
	}
	else
	{
		printf("No response from bus\n");
		receive_msg = MSG_NOTHING;
	}
	return receive_msg;
}

//requests the PIC to send its current adc value MSN (most significant nibble) first
int adc_value()
{
	int i;
	int receive_msg = 0;
	int adc_value = 0;

	//printf("Waiting for custBUs mutex for adcVal\n");
	pthread_mutex_lock(&gpio_m);
	//printf("Got mutex for ADC\n");
	while (receive_msg != MSG_ACK)
	{
		adc_value = 0;
		receive_msg = 0;
		//printf("Starting to send ADC_request\n");
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
			//printf("Received ADC Nibble: 0x%x\n", receive_msg);
			usleep(STROBE_DELAY);
		}
		//expect one last ACK message
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received ADC ACK(?): 0x%x\n", receive_msg);
		usleep(STROBE_DELAY);
	}
	//printf("adc message received successfully: 0x%x\n", adc_value);
	pthread_mutex_unlock(&gpio_m);
	return adc_value;
}

void servo_30()
{
	int receive_msg = 0;

	//printf("Waiting for custBUs mutex for servo 30\n");
	pthread_mutex_lock(&gpio_m);
	while (receive_msg != MSG_ACK)
	{
		//printf("Starting to send Servo30\n");
		writeNibble(MSG_TURN30,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote Nibble to Line Servo30\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	pthread_mutex_unlock(&gpio_m);
	printf("servo_30 message sent\n");
}


void servo_90()
{
	int receive_msg = 0;
	//printf("Waiting for custBUs mutex for Servo_90\n");
	pthread_mutex_lock(&gpio_m);
	while (receive_msg != MSG_ACK)
	{
		//printf("Starting to send Servo90\n");
		writeNibble(MSG_TURN90,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote Nibble to bus\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	pthread_mutex_unlock(&gpio_m);
	printf("Servo_90 message sent\n");
}

void servo_120()
{
	int receive_msg = 0;

	//printf("Waiting for custBUs mutex for servo 120\n");
	pthread_mutex_lock(&gpio_m);
	while (receive_msg != MSG_ACK)
	{
		writeNibble(MSG_TURN120,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote Nibble to bus\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	pthread_mutex_unlock(&gpio_m);
	printf("Servo_120 message sent\n");
}

void rotate_servo_90_E()
{
	int receive_msg = 0;

	//printf("Waiting for custBUs mutex for S90 Enable\n");
	pthread_mutex_lock(&gpio_m);
	while (receive_msg != MSG_ACK)
	{
		//printf("Starting to send Rotate Servo90\n");
		writeNibble(MSG_ROTATE90_E,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote Nibble to Line Rotate Servo90\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	pthread_mutex_unlock(&gpio_m);
	printf("rotate_servo_90 message sent\n");
}

void rotate_servo_180_E()
{
	int receive_msg = 0;

	//printf("Waiting for custBUs mutex for servo 180 enable\n");
	pthread_mutex_lock(&gpio_m);
	while (receive_msg != MSG_ACK)
	{
		//printf("Starting to send Rotate Servo180\n");
		writeNibble(MSG_ROTATE180_E,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote Nibble to Line Rotate Servo180\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	pthread_mutex_unlock(&gpio_m);
	printf("rotate_servo_180 message sent\n");
}

void rotate_servo_90_D()
{
	int receive_msg = 0;

	//printf("Waiting for custBUs mutex for Servo 90 D\n");
	pthread_mutex_lock(&gpio_m);
	while (receive_msg != MSG_ACK)
	{
		//printf("Starting to send Rotate Servo90 disable\n");
		writeNibble(MSG_ROTATE90_D,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote Nibble to Line Rotate Servo90 disable\n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	pthread_mutex_unlock(&gpio_m);
	printf("rotate_servo_90 disable message sent\n");
}

void rotate_servo_180_D()
{
	int receive_msg = 0;

	//printf("Waiting for custBUs mutex for rotate 180 D\n");
	pthread_mutex_lock(&gpio_m);
	while (receive_msg != MSG_ACK)
	{
		//printf("Starting to send Rotate Servo180 disable\n");
		writeNibble(MSG_ROTATE180_D,
			fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Wrote Nibble to Line Rotate Servo180 disable \n");
		usleep(STROBE_DELAY);
		receive_msg = readNibble(fileHandleGPIO_4,
			fileHandleGPIO_5,
			fileHandleGPIO_6,
			fileHandleGPIO_7,
			fileHandleGPIO_S);
		//printf("Received message from PIC: %x \n", receive_msg);
		usleep(STROBE_DELAY);
	}
	pthread_mutex_unlock(&gpio_m);
	printf("rotate_servo_180 disable message sent\n");
}
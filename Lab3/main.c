// Micro2 Main Module.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include "camera.h"
#include "temp_sensor.h"
#include <time.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS 

int main()
{
	char smd[20];
	char smd2[20];
	int cmd;
	//init_temp_sensor();
	time_t date = time(NULL);
	char* cdate;
	cdate = asctime(localtime(&date));
	printf("%s", cdate);
	for (int i = 0; i < 25; i++)
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
	printf("%s", cdate);
	while (1)
	{
		printf("\nEnter the Command you would like to do (1 temp sensor, 2 take a pic, 3 exit)\n");
		scanf("%d", &cmd);
		if (cmd == 1)
		{
			//get_temp_in_f();
			printf("fuck this shit TURN UP THE HEAT\n");
		}
		else if (cmd == 2)
		{
			date = time(NULL);
			cdate = asctime(localtime(&date));
			printf("%s", cdate);
			for (int i = 0; i < 25; i++)
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
			capture_and_save_image(cdate);
		}
		else if (cmd == 3)
		{
			return 0;
		}
		else
		{
			printf("Improper input please enter a number 1-3\n");
		}
	}

}


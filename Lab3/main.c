// Micro2 Main Module.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include "camera.h"
#include "temp_sensor.h"
#include <time.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS 
void temp_test();
int main()
{

	char smd[20];
	char smd2[20];
	int cmd;
	double temp_Reading;
	double temp_Thresh = 30;
	//init_temp_sensor();
	time_t date = time(NULL);
	char* cdate;
	cdate = asctime(localtime(&date));

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
		printf("\nEnter the Command you would like to do (1 temp sensor-triggered pic, 2 take a pic, 3 exit)\n");
		scanf("%d", &cmd);
		if (cmd == 1)
		{
			temp_Reading = get_temp();
			printf("Temperature:%lf degrees C\n", temp_Reading);
			date = time(NULL);
			cdate = asctime(localtime(&date));
			if (temp_Reading > temp_Thresh)
			{
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
        printf("Taking picture: %s \n", cdate);
				capture_and_save_image(cdate);
			} else {
        printf("Not hot enough for picture -- \n\tneeds to be greater than %f\n", temp_Thresh);
      }
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
      break;
    }
  }
}

void temp_test()
{
	double d = get_temp();
d = d *0.0625;
	printf("%f C\n", d);
	//exit();
}
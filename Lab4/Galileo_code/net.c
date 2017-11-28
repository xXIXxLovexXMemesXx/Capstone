
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "net.h"


//format the data to be sent to the server into the buffer
//length is the length of the length of the buffer
void getPostRequest(char* buffer, int length, int picAdcValue, char* picStatus, char* timeStamp, char* filename)
{

  //get timestamp and set member
  //YYYY-MM-DD_HH:MM:SS
  time_t date = time(NULL);
  strftime(timeStamp, MAX_TIMESTAMP, "%F_%T", localtime(&date)); //For format string details: http://man7.org/linux/man-pages/man3/strftime.3.html

  //format the whole url
  int ret = snprintf(buffer,length, 
    "http://%s:%d/update?/id=%d&password=%s&name=%s&data=%d&status=%s&timestamp=%s&filename=%s",
    SERVER_HOSTNAME, SERVER_PORTNUMBER, GROUP_ID, PASSWORD, STUDENT_NAME, picAdcValue,
    picStatus, timeStamp, filename);
  
  //one way to report an error
  if(ret >= length)
    strcpy(buffer, "POST msg length limit exceded");
}

//modified from test_client.c
//all it does is use curl to post a URL
void HTTP_POST(const char* url){ //} const char* image, int size){
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl)
  {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,(long) size);
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, image);
    
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
    curl_easy_cleanup(curl);
  }
}


//Main thread loop
void* serverPostLoop(void * x)
{
  char picStatus[10];
  strcpy(picStatus, PIC_ERROR);
  char filename[MAX_FILENAME];
  strncpy(filename, DEFAULT_FILENAME, MAX_FILENAME);
  char timeStamp[MAX_TIMESTAMP];
  int picAdcValue = 0;

  char postBuffer[MAX_POST];
  
  while(true)
  {
    sleep(2);

    //update data
    server_data currentState = getCurrentState();
    strncpy(filename, currentState.fileName, MAX_FILENAME);
    if(currentState.picOnline)
    {
      strncpy(picStatus,PIC_ONLINE, 10);
    }
    else
    {
      strncpy(picStatus, PIC_ERROR, 10);
    }
    picAdcValue = currentState.adcData;
    
    //format post string 
    getPostRequest(postBuffer, MAX_POST, picAdcValue, picStatus, timeStamp, filename);

    //send it
    HTTP_POST(postBuffer);
  }
}
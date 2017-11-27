
#include <pthreads.h>
#include <time.h>

#include "net.h"

static char m_picStatus[10];
static int m_picAdcValue = 0;
static char m_filename[MAX_FILENAME];
static char m_timeStamp[MAX_TIMESTAMP];


void init()
{
  strcpy(m_picStatus, PIC_ERROR)
  strncpy(m_filename, DEFAULT_FILENAME, MAX_FILENAME);
}

//Loads the dynamic server data.
//if fn is unknown or unavailable, use a empty string
void setDynamicData(bool picStatusOnline, int adcData, char* fn)
{
  //set pic status 
  if(picStatusOnline)
    strcpy(m_picStatus, PIC_ONLINE);
  else
    strcpy(m_picStatus, PIC_ERROR);

  //set adc value directly
  m_picAdcValue = adcData;

  //set filename if its there. Empty filename gets default filename
  if(!fn)
  {
    strcpy(m_filename, DEFAULT_FILENAME);
  }
  else
  {
    if(strlen(fn) > MAX_FILENAME)
      strcpy(m_filename, "Filename limit exceded");
    else
      strncpy(m_filename, fn, MAX_FILENAME);
  }

}
  
//format the data to be sent to the server into a URL and return it.
//need to free when done
char* getPostRequest()
{
  char* url = malloc(MAX_POST);

  //get timestamp and set member
  //YYYY-MM-DD_HH:MM:SS
  time_t date = time(NULL);
  strftime(m_timeStamp, MAX_TIMESTAMP, "%F_%T", localtime(&date)); //For format string details: http://man7.org/linux/man-pages/man3/strftime.3.html


  //format the whole url
  int ret = snprintf(url, MAX_POST, 
    "http://%s:%d/update?/id=%d&password=%s&name=%s&data=%d&status=%s&timestamp=%s&filename=%s",
    SERVER_HOSTNAME, SERVER_PORTNUMBER, GROUP_ID, PASSWORD, STUDENT_NAME, m_picAdcValue,
    m_picStatus, m_timeStamp, m_filename);
  
  if(ret >= MAX_POST)
    strcpy(url, "POST msg length limit exceded");

  return url;
}

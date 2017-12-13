#include "sensor.h"
#include "common.h"

#include "opencv2/opencv.hpp"
#include "mraa.hpp"

using namespace std;


//read image from filename
//if successful, save as a file -- .png and retrun true
//if not, return false.
bool capture_and_save_image(char* filename)
{
  using namespace cv;

  //init webcam on video0 interface
  VideoCapture ourCam;
  ourCam.open(0);

  Mat image;
  bool camIsThere = false;

  //read a frame from the vid camera into image
  camIsThere = ourCam.read(image); 

  //test if camera not connected
  if(!camIsThere)
    return false; 
  //test if a blank image was grabbed
  if(image.empty())
    return false; 

  //make jpeg format parameters
  vector<int> compression_params;
  compression_params.push_back(IMWRITE_JPEG_QUALITY);
  compression_params.push_back(95);

  //make filename
  string fn = string(filename) + string(".jpeg");
  
  //Code from inWrite() example in OpenCV docs
  //try to save to file
  try{
    imwrite(fn, image, compression_params);
  }
  catch (Exception& ex) {
    cout << "Issue saving file: " << ex.what() << endl;
    return false;
  }

  return true;
}

//Ask the temp sensor to give you the temp in C
double get_temp()
{
  using namespace mraa; 
  int temperature;

  //printf("About to probe I2C temp sensor\n");

  try{
    I2c i2c(0);
    i2c.address(TMP102Address);

    uint8_t dataReg [2];

    int buffer = i2c.read(dataReg,2); // read two bytes from the registers
    temperature = ((dataReg[0]<<8 | dataReg[1]) >> 4);
  } 
  catch(const std::invalid_argument& e) //catch an error from no device on bus
  {
    temperature = -10000;
    printf("Temp sensor read error. Sensor not connected\n");
  }

  return ((double)temperature)*0.0625;
}
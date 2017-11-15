#include "camera.h"

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

//read image from filename
//if successful, save as a file -- .png and retrun true
//if not, return false.
bool capture_and_save_image(char* filename)
{
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

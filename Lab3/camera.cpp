#include "camera.h"

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

//read image from filename
//if successful, save as a file -- .png and retrun true
//if not, return false.
bool capture_and_save_image(char* filename)
{
  VideoCapture ourCam;
  ourCam.open(0);

  Mat image;
  bool camIsThere = false;

  camIsThere = ourCam.read(image); //read a frame from the vid camera into image

  if(!camIsThere)
    return false; //camera not connected somehow
  if(image.empty())
    return false; //blank frame grabbed

  //make jpeg format parameters
  vector<int> compression_params;
  compression_params.push_back(IMWRITE_JPEG_QUALITY);
  compression_params.push_back(95);

  string fn = string(filename) + string(".jpeg");
  
  //Code from inWrite() example in OpenCV docs
  try{
    imwrite(fn, image, compression_params);
  }
  catch (Exception& ex) {
    cout << "Issue converting to PNG format: " << ex.what() << endl;
    return false;
  }

  return true;
}

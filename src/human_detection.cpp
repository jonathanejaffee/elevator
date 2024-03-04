
#include "human_detection.h"
using namespace cv;
using namespace cv::ml;
using namespace hd;
void HumanDetector::loadImage(string imgPath)
{
    Mat img = imread(imgPath);
}

//void HumanDetector::detectHumans()
//{
//
//}
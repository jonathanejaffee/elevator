#pragma once

#include <opencv2/ml.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
using namespace cv;
using namespace cv::ml;
using namespace std;
namespace hd
{
class HumanDetector
{
public:
    HumanDetector(){};
    ~HumanDetector(){};
    int detectHumans(string imgPath);
    //void detectHumans();
        

};
}
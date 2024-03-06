#pragma once
// Includes
#include <opencv2/ml.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>
using namespace cv;
using namespace cv::ml;
using namespace std;
namespace hd
{
// This class looks at images (say from CCTV) and detects the number of people in the image.
class HumanDetector
{
public:
    HumanDetector();
    ~HumanDetector(){};
    int detectHumans(const string& imgPath); // Detects number of images in the image, pass param by const ref
private:
    HOGDescriptor mHog;
        
};
}

#include "human_detection.h"
#include <vector>
using namespace cv;
using namespace cv::ml;
using namespace hd;
void HumanDetector::loadImage(string imgPath)
{
    Mat img = imread(imgPath);
    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    vector<Rect> found;
    vector<double> weights;

    hog.detectMultiScale(img, found, weights);
    std::cout << "FOUND: " << found.size() << std::endl;
}

//void HumanDetector::detectHumans()
//{
//
//}
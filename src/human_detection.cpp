
#include "human_detection.h"
#include <vector>
using namespace cv;
using namespace cv::ml;
using namespace hd;
int HumanDetector::loadImage(string imgPath)
{
    Mat img = imread(imgPath);
    resize(img,img,Size(img.cols*2, img.rows*2));
    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    vector<Rect> found;
    vector<double> weights;

    hog.detectMultiScale(img, found, weights);
    //std::cout << "FOUND: " << found.size() << ", For img: " << imgPath << std::endl;
    /// draw detections and store location
    vector<Point> track;
    for( size_t i = 0; i < found.size(); i++ )
    {
        Rect r = found[i];
        rectangle(img, found[i], cv::Scalar(0,0,255), 3);
        stringstream temp;
        temp << weights[i];
        putText(img, temp.str(),Point(found[i].x,found[i].y+50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255));
        track.push_back(Point(found[i].x+found[i].width/2,found[i].y+found[i].height/2));
    }

    /// plot the track so far
    for(size_t i = 1; i < track.size(); i++){
        line(img, track[i-1], track[i], Scalar(255,255,0), 2);
    }
    /// Show
    imshow("detected person", img);
    waitKey(100);
    return found.size();
}

//void HumanDetector::detectHumans()
//{
//
//}
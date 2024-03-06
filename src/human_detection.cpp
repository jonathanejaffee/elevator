
#include "human_detection.h"
#include <vector>
using namespace cv;
using namespace cv::ml;
using namespace hd;

// Constructor
HumanDetector::HumanDetector()
{
    mHog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector()); // Create the model
}

/**
 * This cass detects humans in an image using openCV's histogram of gradients (HOG) pre-trained people detector model.
 * Default hyper-parameter used.
*/
int HumanDetector::detectHumans(const string& imgPath) // pass by const ref, dont have to copy.
{
    Mat img = imread(imgPath);
    resize(img,img,Size(1280, 960)); // Resize
    vector<Rect> found;
    vector<double> weights;

    mHog.detectMultiScale(img, found, weights); // Find detections

    // save detection drawings
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

    // Create box around detect
    for(size_t i = 1; i < track.size(); i++){
        line(img, track[i-1], track[i], Scalar(255,255,0), 2);
    }
    // Show the people detected
    imshow("detected people", img);
    waitKey(500);
    destroyAllWindows();
    return found.size();
}


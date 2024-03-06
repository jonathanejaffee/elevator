#pragma once

#include <mutex>
#include <vector>
#include <deque>
#include <thread>
#include "floor_request.h"
#include "human_detection.h"

using namespace std;

namespace ele
{
class Elevator
{

public:

    Elevator(unsigned int speed, unsigned int waitTime, int maxCap, int startingFloor);
    Elevator() = delete;
    ~Elevator();
    //void setCurrentFloor(int floor);
    int incCurrentFloor(); //int inc);
    int getCurrentFloor();
    //int getTargetFloor();
    void addRequest(flreq::floorRequest floor);

    bool done();

private:
    void runElevator();
    deque<flreq::floorRequest>::iterator searchReqList(int diff);
    bool checkTargMatch();
    chrono::duration<unsigned int, milli> mElevatorSpeed;
    chrono::duration<unsigned int, milli> mElevatorStopTime;
    int mLobbyCap;
    int mCurrentFloor;
    bool mRun;
    int mDirect;
    //mutex mMutexCurrent;
    mutex mMutexFloors;
    deque<flreq::floorRequest> mFloors;
    hd::HumanDetector mHumanDetect;

};
}
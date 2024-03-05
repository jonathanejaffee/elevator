#pragma once

#include <mutex>
#include <vector>
#include <deque>
#include <thread>
#include "floor_request.h"

using namespace std;

namespace ele
{
class Elevator
{

public:

    Elevator(unsigned int speed, int startingFloor);
    ~Elevator();
    //void setCurrentFloor(int floor);
    int incCurrentFloor(); //int inc);
    int getCurrentFloor();
    //int getTargetFloor();
    void addRequest(flreq::floorRequest floor);

private:
    void runElevator();
    vector<deque<flreq::floorRequest>::iterator> searchReqList();
    int checkTargMatch(int curr);
    unsigned int mElevatorSpeed;
    int mCurrentFloor;
    bool mRun;
    int mDirect;
    //mutex mMutexCurrent;
    mutex mMutexFloors;
    deque<flreq::floorRequest> mFloors;
    unsigned int mNumPpl;

};
}
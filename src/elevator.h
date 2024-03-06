#pragma once

#include <mutex>
#include <vector>
#include <deque>
#include <thread>
#include "floor_request.h"
#include "human_detection.h"
#include <condition_variable>

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

    pair<vector<int>, unsigned int> done();

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
    std::condition_variable condV;
    bool doneProcess = false;
    std::vector<int> mFloorsVisited;
    unsigned int mTimeTaken;
    bool mStopped;

};
}
#pragma once

#include <mutex>
#include <vector>
#include <thread>

using namespace std;

namespace ele
{
class Elevator
{

public:

    Elevator(unsigned int speed, int startingFloor);
    ~Elevator();
    void setTargetFloor(int target);
    void setCurrentFloor(int floor);
    void incCurrentFloor(int inc);
    int getCurrentFloor();
    int getTargetFloor();
    void addRequest(int floor);

private:
    void runElevator();
    unsigned int mElevatorSpeed;
    int mCurrentFloor;
    int mTargetFloor;
    bool mRun;
    mutex mMutexTarget;
    mutex mMutexCurrent;
    mutex mMutexFloors;
    vector<int> mFloors;

};
}
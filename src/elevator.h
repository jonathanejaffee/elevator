#pragma once

#include <mutex>
#include <vector>
#include <deque>
#include <thread>

using namespace std;

namespace ele
{
class Elevator
{

public:

    Elevator(unsigned int speed, int startingFloor);
    ~Elevator();
    void setCurrentFloor(int floor);
    int incCurrentFloor(int inc);
    int getCurrentFloor();
    int getTargetFloor();
    void addRequest(int floor);

private:
    void runElevator();
    int checkTargMatch(int curr);
    unsigned int mElevatorSpeed;
    int mCurrentFloor;
    bool mRun;
    //mutex mMutexCurrent;
    mutex mMutexFloors;
    deque<int> mFloors;

};
}
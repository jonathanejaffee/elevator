#pragma once

#include <mutex>

using namespace std;

namespace ele
{
class Elevator
{

public:

    Elevator(unsigned int speed, int startingFloor);

    void setTargetFloor(int target);
    int getCurrentFloor();

private:
    unsigned int mElevatorSpeed;
    int mCurrentFloor;
    int mTargetFloor;
    mutex mMutexTarget;
    mutex mMutexCurrent;

};
}
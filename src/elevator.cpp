
#include "elevator.h"
#include <iostream>

using namespace std;
using namespace ele;

Elevator::Elevator(unsigned int speed, int startingFloor) : mElevatorSpeed(speed),
    mCurrentFloor(startingFloor),
    mTargetFloor(startingFloor)
{
    cout << "Creating new Elevator with speed: " << mElevatorSpeed << ", starting floor: " << startingFloor << endl;
}

void Elevator::setTargetFloor(int target)
{
    lock_guard<mutex> lock(mMutexTarget); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
    mTargetFloor = target;
}

int Elevator::getCurrentFloor()
{
    lock_guard<mutex> lock(mMutexCurrent);
    return mCurrentFloor;
}
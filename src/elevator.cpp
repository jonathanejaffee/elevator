
#include "elevator.h"
#include <iostream>

using namespace std;
using namespace ele;

Elevator::Elevator(unsigned int speed, int startingFloor) : mElevatorSpeed(speed),
    mCurrentFloor(startingFloor),
    mTargetFloor(startingFloor),
    mRun(true)
{
    cout << "Creating new Elevator with speed: " << mElevatorSpeed << ", starting floor: " << startingFloor << endl;
    // Need to spawn 2 threads - the elevator dynamics thread, which only cares about moving the elevator to a single target,
    // and the request handler thread, which dictates at any given time where the elevator should be attempting to move to.
    thread eleThread(&Elevator::runElevator, this);
    eleThread.detach();
}

Elevator::~Elevator()
{
    mRun = false;
}

void Elevator::setTargetFloor(int target)
{
    lock_guard<mutex> lock(mMutexTarget); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
    mTargetFloor = target;
    cout << "Target Floor: " << target << endl;
}

void Elevator::setCurrentFloor(int floor)
{
    lock_guard<mutex> lock(mMutexCurrent); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
    mCurrentFloor = floor;
}

void Elevator::incCurrentFloor(int inc)
{
    lock_guard<mutex> lock(mMutexCurrent); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
    mCurrentFloor += inc;
    cout << "Current Floor: " << mCurrentFloor << endl;
}
int Elevator::getCurrentFloor()
{
    lock_guard<mutex> lock(mMutexCurrent);
    return mCurrentFloor;
}

int Elevator::getTargetFloor()
{
    lock_guard<mutex> lock(mMutexTarget);
    return mTargetFloor;
}

void Elevator::addRequest(int floor)
{
    lock_guard<mutex> lock(mMutexFloors);
    mFloors.push_back(floor);

}

void Elevator::runElevator()
{
    while (mRun)
    {
        //cout << "WEEEEE" << endl;
        //this_thread::sleep_for(1s);
        int dFloors = getTargetFloor() - getCurrentFloor();
        while (dFloors != 0 && mRun)
        {
            this_thread::sleep_for(1s);
            (dFloors > 0) ? incCurrentFloor(1) : incCurrentFloor(-1);
            dFloors = getTargetFloor() - getCurrentFloor();
        }
        mMutexFloors.lock();
        if (!mFloors.empty())
        {
            int newTarg = mFloors.back();
            mFloors.pop_back();
            setTargetFloor(newTarg);
        }
        mMutexFloors.unlock();
    }
}
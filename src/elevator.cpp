
#include "elevator.h"
#include <iostream>

using namespace std;
using namespace ele;

Elevator::Elevator(unsigned int speed, int startingFloor) : mElevatorSpeed(speed),
    mCurrentFloor(startingFloor),
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

void Elevator::setCurrentFloor(int floor)
{
    lock_guard<mutex> lock(mMutexFloors); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
    mCurrentFloor = floor;
}

int Elevator::incCurrentFloor(int inc)
{
    lock_guard<mutex> lock(mMutexFloors); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
    mCurrentFloor += inc;
    cout << "Current Floor: " << mCurrentFloor << endl;
    return mCurrentFloor;
}
int Elevator::getCurrentFloor()
{
    lock_guard<mutex> lock(mMutexFloors);
    return mCurrentFloor;
}

int Elevator::getTargetFloor()
{
    lock_guard<mutex> lock(mMutexFloors);

    return (mFloors.empty() ? getCurrentFloor() : mFloors.back());
}

void Elevator::addRequest(int floor)
{
    lock_guard<mutex> lock(mMutexFloors);
    cout << "Adding Target Floor: " << floor << endl; 
    mFloors.push_front(floor);

}

int Elevator::checkTargMatch(int curr)
{
    lock_guard<mutex> lock(mMutexFloors);
    int diff = 0;
    if (!mFloors.empty())
    {
        diff = mFloors.back() - curr;
        if (diff == 0)
        {
            mFloors.pop_back();
            cout << "Elevator hit target floor: " << curr << endl;
        }
        //cout << "req sz: " << mFloors.size() << endl;
    }
    return diff;
}


void Elevator::runElevator()
{
    while (mRun)
    {
        int curr = getCurrentFloor();
        int dFloors = checkTargMatch(curr);
        int numMoves = 0;
        while ((dFloors != 0) && mRun)
        {
            this_thread::sleep_for(1s);
            curr = (dFloors > 0) ? incCurrentFloor(1) : incCurrentFloor(-1);
            //target = getTargetFloor();
            //curr = getCurrentFloor();
            dFloors = checkTargMatch(curr);
            //cout << "dfloors: " << dFloors << endl;
            //numMoves++;
        }
        //mMutexFloors.lock();
        //if (!mFloors.empty())
        //{
        //    int newTarg = mFloors.back();
        //    mFloors.pop_back();
        //    setTargetFloor(newTarg);
        //}
        //mMutexFloors.unlock();
    }
}

#include "elevator.h"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace ele;

Elevator::Elevator(unsigned int speed, int startingFloor) : mElevatorSpeed(speed),
    mCurrentFloor(startingFloor),
    mRun(true),
    mDirect(1)
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

//void Elevator::setCurrentFloor(int floor)
//{
//    lock_guard<mutex> lock(mMutexFloors); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
//    mCurrentFloor = floor;
//}

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

//int Elevator::getTargetFloor()
//{
//    lock_guard<mutex> lock(mMutexFloors);
//
//    return (mFloors.empty() ? getCurrentFloor() : mFloors.back());
//}

void Elevator::addRequest(flreq::floorRequest floor)
{
    lock_guard<mutex> lock(mMutexFloors);
    cout << "Adding Target Floor: " << floor.floor << endl; 
    mFloors.push_front(floor);
    sort(mFloors.begin(), mFloors.end());
    int currPos = mCurrentFloor + mDirect;
    const int PENALTY_FACTOR = 2;
    int distUp = max(0, (mFloors.back().floor - currPos));
    if (mFloors.back().direction == 1)
    {
        distUp += PENALTY_FACTOR;
    }
    int distDown = max(0, (currPos - mFloors.front().floor));
    if (mFloors.front().direction == -1)
    {
        distDown += PENALTY_FACTOR;
    }
    if (distUp < distDown)
    {
        mDirect = 1; // moving up
    }
    else
    {
        mDirect = -1; // Moving down
    }
}

vector<int> Elevator::searchReqList()
{
    vector<int> matches;
    //lock_guard<mutex> lock(mMutexFloors);
    if (mDirect == 1) // going up
    {
        int idx = 0;
        while ((mFloors[idx].floor <= mCurrentFloor) && (idx < mFloors.size()))
        {
            if (mFloors[idx].floor == mCurrentFloor)
            {
                if ((mFloors[idx].direction == 1) || (mFloors[idx].direction == 0))
                {
                    matches.push_back(idx);
                }
            }
            idx++;
        }
    }
    else if (mDirect == -1)
    {
        int idx = mFloors.size() - 1;
        while ((mFloors[idx].floor >= mCurrentFloor) && (idx >= 0))
        {
            if (mFloors[idx].floor == mCurrentFloor)
            {
                if ((mFloors[idx].direction == -1) || (mFloors[idx].direction == 0))
                {
                    matches.push_back(idx);
                }
            }
            idx--;
        }
    }
    else
    {
        cout << "this shouldnt be 0" << endl;
    }
    return matches;
}

int Elevator::checkTargMatch(int curr)
{
    lock_guard<mutex> lock(mMutexFloors);
    int diff = 0;
    if (!mFloors.empty())
    {
        flreq::floorRequest targFloor = (mDirect > 0) ? mFloors.back() : mFloors.front();
        diff = targFloor.floor - curr;
        if (diff == 0)
        {
            (mDirect > 0) ? mFloors.pop_back() : mFloors.pop_front();
            cout << "Elevator hit END target floor: ";
            targFloor.print();
            mDirect *= -1;
        }
        else
        {
            vector<int> hit = searchReqList();
            for (int i = 0; i < hit.size(); i++)
            {
                cout << "Elevator hit target floor: ";
                mFloors[i].print();
                mFloors.erase(mFloors.begin() + hit[i]);
            }
        }
        //cout << "req sz: " << mFloors.size() << endl;
    }
    else
    {
        mDirect = 0;
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
            this_thread::sleep_for(2s);
            curr = (dFloors > 0) ? incCurrentFloor(1) : incCurrentFloor(-1);
            //target = getTargetFloor();
            //curr = getCurrentFloor();
            dFloors = checkTargMatch(curr);
            //cout << "dfloors: " << dFloors << endl;
            //numMoves++;
        }
        // set direct to 0
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
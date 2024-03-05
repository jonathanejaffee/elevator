
#include "elevator.h"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace ele;

Elevator::Elevator(unsigned int speed, int startingFloor) : mElevatorSpeed(speed),
    mCurrentFloor(startingFloor),
    mRun(true),
    mDirect(1),
    mNumPpl(0)
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

int Elevator::incCurrentFloor() //int inc)
{
    lock_guard<mutex> lock(mMutexFloors); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
    mCurrentFloor += mDirect;
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
    //if ((floor.direction != 0) && floor.numPpl < 1)
    //{
    //    cout << "not enough people" << endl;
    //    return;
    //}
    cout << "Adding Target Floor: " << floor.floor << endl; 
    mFloors.push_front(floor);
    sort(mFloors.begin(), mFloors.end());
    int currPos = mCurrentFloor + mDirect;
    const int PENALTY_FACTOR = 2;
    int distUp = max(0, (mFloors.back().floor - currPos));
    if ((mFloors.back().direction == 1) && (distUp > 0))
    {
        distUp += PENALTY_FACTOR;
    }
    int distDown = max(0, (currPos - mFloors.front().floor));
    //cout << "Dist up: " << distUp << endl;
    //cout << "Dist down: " << distDown << endl;


   if ((mFloors.front().direction == -1) && (distDown > 0))
    {
        distDown += PENALTY_FACTOR;
    }

    if ((distUp < 1) && (distDown < 1))
    {
        mDirect = 0;
    }
    else if ((distUp > 0) && (distDown <= 0))
    {
        mDirect = 1;
    }
    else if ((distDown > 0) && (distUp <= 0))
    {
        mDirect = -1;
    }
    else if ((distUp < distDown))
    {
        mDirect = 1;
       // cout << "1" << endl;
    }
    else if ((distDown <= distUp))
    {
        mDirect = -1; // moving up
     //   cout << "-1, " << distDown << endl;
    }
   // cout << "MDIRECT: " << mDirect << endl;
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
                else if (mFloors[idx].numPpl > 2)
                {
                    matches.push_back(idx);
                    mDirect = -1;
                    cout << "Too many ppl swapping direct for: ";
                    mFloors[idx].print();
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
                else if (mFloors[idx].numPpl > 2)
                {
                    matches.push_back(idx);
                    mDirect = 1;
                    cout << "Too many ppl swapping direct for: ";
                    mFloors[idx].print();
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
                mFloors[hit[i]].print();
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
            curr = incCurrentFloor(); //(dFloors > 0) ? incCurrentFloor(1) : incCurrentFloor(-1);
            //target = getTargetFloor();
            //curr = getCurrentFloor();
            dFloors = checkTargMatch(curr);
            //cout << "dfloors: " << dFloors << endl;
            //numMoves++;
        }
        this_thread::sleep_for(1s);
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
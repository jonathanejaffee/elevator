
#include "elevator.h"
#include <iostream>
#include <utility>
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

    if ((mFloors.front().direction == -1) && (distDown > 0))
    {
        distDown += PENALTY_FACTOR;
    }

    int dDist = distUp - distDown;
    if (distUp == 0)
    {
        mDirect = -1;
    }
    else if (distDown == 0)
    {
        mDirect = 1;
    }
    else
    {
        mDirect = dDist/abs(dDist);
    }

}

vector<deque<flreq::floorRequest>::iterator> Elevator::searchReqList()
{
    vector<deque<flreq::floorRequest>::iterator> matches;

    auto match = [&cf = as_const(mCurrentFloor)](const flreq::floorRequest& fl) -> bool { return (fl.floor == cf); };

    auto it = find_if(mFloors.begin(), mFloors.end(), match);
    while (it != mFloors.end())
    {
        if ((it == mFloors.begin()) || (it == (mFloors.end() - 1)))
        {
            matches.push_back(it);
        }
        else if ((it -> direction == mDirect) || (it -> direction == 0))
        {
            matches.push_back(it);
        }
        else if (it -> numPpl > 2)
        {
            matches.push_back(it);
            bool changeDir = (it -> direction == 1) ? (mFloors.back().floor > mCurrentFloor) : (mFloors.front().floor < mCurrentFloor);
            if (changeDir)
            {
                mDirect = it -> direction; // need to make sure there is something in that direction
                cout << "Too many ppl swapping direct for: ";
                it->print();
            }
        }
        it = find_if(it+1, mFloors.end(), match);
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
            //mDirect = 0;
            //this_thread::sleep_for(1s);
            mDirect *= -1;
        }
        else
        {
            vector<deque<flreq::floorRequest>::iterator> hit = searchReqList();
            deque<flreq::floorRequest>::iterator it;
            for (int i = 0; i < hit.size(); i++)
            {
                it = hit[i];
                cout << "Elevator hit target floor: ";
                it -> print();
                mFloors.erase(it);
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

    }
}
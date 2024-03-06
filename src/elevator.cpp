
#include "elevator.h"
#include <iostream>
#include <utility>
#include <algorithm>
#include <condition_variable>
using namespace std;
using namespace ele;

condition_variable cv;
bool doneProcess = false;

Elevator::Elevator(unsigned int speed, unsigned int wait, int lobbyCap, int startingFloor) : mCurrentFloor(startingFloor),
    mRun(true),
    mDirect(0),
    mLobbyCap(lobbyCap)
{
    cout << "Creating new elevator with speed: " << speed << ", Stop wait time: " << wait << ", Lobby Capacity: " << lobbyCap << ", starting floor: " << startingFloor << endl;
    cout << "Not - the above is order of optional command line args after input file" << endl;
    mElevatorSpeed = chrono::duration<int, milli>(speed*1000);
    mElevatorStopTime = chrono::duration<int, milli>(wait*1000);
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
        if (mDirect == 0)
        {
            mDirect = dDist/abs(dDist);
        }
    }

}

deque<flreq::floorRequest>::iterator Elevator::searchReqList(int diff)
{
    mDirect = diff/abs(diff);
    auto match = [&cf = as_const(mCurrentFloor)](const flreq::floorRequest& fl) -> bool { return (fl.floor == cf); };

    auto it = find_if(mFloors.begin(), mFloors.end(), match);
    if (it != mFloors.end())
    {
        if ((it -> direction != mDirect) && ((mDirect != 0) && (it ->direction != 0)))
        {
            if (it -> numPpl > mLobbyCap)
            {
                bool changeDir = (it -> direction == 1) ? (mFloors.back().floor > mCurrentFloor) : (mFloors.front().floor < mCurrentFloor);
                if (changeDir)
                {
                    mDirect = it -> direction; // need to make sure there is something in that direction
                    cout << "Too many ppl swapping direct for: ";
                    it->print();
                }
            }
            else
            {
                it = mFloors.end();
            }
        }
    }
    return it;
}

bool Elevator::checkTargMatch()
{
    int curr = getCurrentFloor();
    lock_guard<mutex> lock(mMutexFloors);
    int diff = 0;
    bool stopped = false;
    if (!mFloors.empty())
    {
        doneProcess = false;
        auto it = (mDirect > 0) ? (mFloors.end() - 1) : mFloors.begin();
        diff = it->floor - curr;

        auto findBest = [diff, it, &mDirect = as_const(mDirect), &mCurrentFloor = as_const(mCurrentFloor)] () -> deque<flreq::floorRequest>::iterator
        {
            deque<flreq::floorRequest>::iterator bestIt = it;
            int newDiff = diff;
            while (newDiff == 0)
            {
                if ((bestIt -> direction == mDirect) || (bestIt -> direction == 0) || (mDirect == 0))
                {
                    return bestIt;
                }
                (mDirect > 0) ? bestIt-- : bestIt++;
                newDiff = bestIt->floor - mCurrentFloor;
            }
            return it;
        };

        if (diff == 0) // at one of the extremities
        {
            deque<flreq::floorRequest>::iterator bestIt = findBest();
            cout << "Elevator hit END target floor: ";
            bestIt->print();
            mFloors.erase(bestIt);
            stopped = true;
        }
        else
        {
            deque<flreq::floorRequest>::iterator hit = searchReqList(diff);
            if (hit != mFloors.end())
            {
                cout << "Elevator hit target floor: ";
                hit -> print();
                mFloors.erase(hit);
                stopped = true;
            }
        }
    }
    else
    {
        mDirect = 0;
        doneProcess = true;
        cv.notify_one();
    }
    return stopped;
}


void Elevator::runElevator()
{
    cout << "Starting Elevator" << endl;
    //this_thread::sleep_for(5s);
    bool stop = true;
    while (mRun)
    {
        this_thread::sleep_for(mElevatorSpeed);
        if (!stop)
        {
            incCurrentFloor();
        }
        else
        {
            cout << "stop" << endl;
            this_thread::sleep_for(mElevatorStopTime);
        }
        stop = checkTargMatch();
    }
}

bool Elevator::done()
{
    unique_lock<mutex> lock(mMutexFloors);
    cv.wait(lock, [] { return doneProcess; });
    return true;
}
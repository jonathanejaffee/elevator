
#include "elevator.h"
#include <iostream>
#include <utility>
#include <algorithm>
//#include <condition_variable>
using namespace std;
using namespace ele;

//std::condition_variable condV;
//bool doneProcess = false;

Elevator::Elevator(unsigned int speed, unsigned int wait, int lobbyCap, int startingFloor) : mCurrentFloor(startingFloor),
    mRun(true),
    mDirect(0),
    mLobbyCap(lobbyCap),
    mTimeTaken(0),
    mStopped(true)
{
    cout << "Creating new elevator with speed: " << speed << ", Stop wait time: " << wait << ", Lobby Capacity: " << lobbyCap << ", starting floor: " << startingFloor << endl;
    cout << "Not - the above is order of optional command line args after input file" << endl;
    mElevatorSpeed = chrono::duration<int, milli>(speed*1000);
    mElevatorStopTime = chrono::duration<int, milli>(wait*1000);
    mFloors = deque<flreq::floorRequest>();
    mHumanDetect = hd::HumanDetector();
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
    if (floor.direction != 0)
    {
        floor.numPpl = mHumanDetect.detectHumans(floor.image);
    }
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
    cout << "distUP: " << distUp << endl;
    cout << "distDown: " << distDown << endl;
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
        if ((mDirect == 0) || mStopped)
        {
            cout << "weee" << endl;
            mDirect = -dDist/abs(dDist);
        }
    }
    cout << "MDIRECT: " << mDirect << endl;
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
    //bool stopped = false;
    if (!mFloors.empty())
    {
        doneProcess = false;
        auto it = (mDirect > 0) ? (mFloors.end() - 1) : mFloors.begin();
        diff = it->floor - curr;
        cout << "Fl: " << it->floor << ", dir " << it->direction << endl;
        auto findBest = [diff, it, &mDirect = as_const(mDirect), &mCurrentFloor = as_const(mCurrentFloor), &mFloors = as_const(mFloors)] () -> deque<flreq::floorRequest>::iterator
        {
            if (mFloors.size() < 2)
            {
                return it;
            }
            deque<flreq::floorRequest>::iterator bestIt = it;
            int newDiff = diff;
            while (newDiff == 0)
            {
                if ((bestIt -> direction == mDirect) || (bestIt -> direction == 0) || (mDirect == 0))
                {
                    cout << "bestIt" << endl;
                    bestIt -> print();
                    return bestIt;
                }
                (mDirect > 0) ? bestIt-- : bestIt++;
                if ((bestIt < mFloors.begin()) || (bestIt >= mFloors.end()))
                {
                    return it;
                }
                newDiff = bestIt->floor - mCurrentFloor;
            }
            cout << "It" << endl;
            it -> print();
            return it;
        };

        if (diff == 0) // at one of the extremities
        {
            deque<flreq::floorRequest>::iterator bestIt = findBest();
            cout << "Elevator hit END target floor: ";
            bestIt->print();
            mFloorsVisited.push_back(bestIt->floor);
            mFloors.erase(bestIt);
            mStopped = true;
            //stopped = true;
        }
        else
        {
            deque<flreq::floorRequest>::iterator hit = searchReqList(diff);
            if (hit != mFloors.end())
            {
                cout << "Elevator hit target floor: ";
                hit -> print();
                mFloorsVisited.push_back(hit->floor);
                mFloors.erase(hit);
                mStopped = true;
                return mStopped;
            }
            mStopped = false;
        }
    }
    else
    {
        mDirect = 0;
        mStopped = true;
        doneProcess = true;
        condV.notify_one();
    }
    return mStopped;
}


void Elevator::runElevator()
{
    cout << "Starting Elevator" << endl;
    this_thread::sleep_for(mElevatorStopTime);
    cout << "Elevator Started" << endl;
    bool stop = true;
    while (mRun)
    {
        this_thread::sleep_for(mElevatorSpeed);
        mTimeTaken += mElevatorSpeed.count();
        if (!stop)
        {
            incCurrentFloor();
        }
        else
        {
            cout << "stop" << endl;
            this_thread::sleep_for(mElevatorStopTime);
            mTimeTaken += mElevatorStopTime.count();
        }
        stop = checkTargMatch();
    }
}

pair<vector<int>, unsigned int> Elevator::done()
{
    unique_lock<mutex> lock(mMutexFloors);
    condV.wait(lock, [&doneProcess= as_const(doneProcess)] { return doneProcess; });
    cout << "Done" << endl;
    this_thread::sleep_for(3s);
    return make_pair(mFloorsVisited, (mTimeTaken/1000));
}
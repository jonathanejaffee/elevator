// Includes
#include "elevator.h"
#include <iostream>
#include <utility>
#include <algorithm>

using namespace std;
using namespace ele;

/**
 * Create the elevator
 * @param speed = speed, in seconds, that it takes the elevator to move 1 floor
 * @param wait = time in seconds that the elevator will remain waiting when stopping on a floor
 * @param lobbyCap = capacity for elevator lobbies (on all floors), number of people
 * @param startingFloor = floor the elevator starts on
*/
Elevator::Elevator(unsigned int speed, unsigned int wait, int lobbyCap, int startingFloor) : mCurrentFloor(startingFloor),
    mRun(true),
    mDirect(0),
    mLobbyCap(lobbyCap),
    mTimeTaken(0),
    mStopped(true)
{
    cout << "Creating new elevator with speed: " << speed << ", Stop wait time: " << wait << ", Lobby Capacity: " << lobbyCap << ", starting floor: " << startingFloor << endl;
    cout << "Not - the above is order of optional command line args after input file" << endl;
    mElevatorSpeed = chrono::duration<int, milli>(speed*1000); // convert to milliseconds
    mElevatorStopTime = chrono::duration<int, milli>(wait*1000); // ^
    mFloors = deque<flreq::floorRequest>(); // Create the deque of floors
    mHumanDetect = hd::HumanDetector(); // Instantiate the human detector
    thread eleThread(&Elevator::runElevator, this); // Spawn the elevator thread, which processes mFloors
    eleThread.detach(); // Let it run independently
}

/**
 * Destructor
*/
Elevator::~Elevator()
{
    mRun = false; // This will kill the elevator run thread
}

/**
 * (Inc/Dec)rements what floor the elevator is on based on the direction it is moving
 * @return int = floor the elevator is on after incrementing or decrementing
*/
int Elevator::incCurrentFloor() 
{
    lock_guard<mutex> lock(mMutexFloors); // Locks mutex, automatic storage duration, gurantees unlock on exit (ot of scope) of this function
    mCurrentFloor += mDirect;
    cout << "Current Floor: " << mCurrentFloor << endl;
    return mCurrentFloor;
}

/**
 * Adds a floor to visit
 * @param floor = floor request to visit
*/
void Elevator::addRequest(flreq::floorRequest floor)
{
    // If the floor direction is up or down, we know it's coming from a lobby, so going to check how many people are in that lobby
    if (floor.direction != 0)
    {
        cout << "Checking number of people in lobby on floor: " << floor.floor << endl;
        floor.numPpl = mHumanDetect.detectHumans(floor.image); // Get the Number of people in the lobby
    }
    // Lock to protect shared resources
    lock_guard<mutex> lock(mMutexFloors); // Automatic storage duration, unlocks on destruction
    cout << "Adding Target Floor: " << floor.floor << ", going: " << floor.direction << endl; 
    mFloors.push_front(floor); // Add to the request list
    sort(mFloors.begin(), mFloors.end()); // Sort the request list (uses comparator override in floorRequest class)
    int currPos = mCurrentFloor + mDirect; // Assume the elevator may be moving, so the current position will be last position plus direction of motion
    const int PENALTY_FACTOR = 2; // Penalty factor, hardcoded for now
    /**
     * A note on the penalty factor ^
     * When trying to asess the extremity distances to travel up or down, if the farthest up or down request
     * want to continue going in their extermity direction (ie, if the lowest floor in the request list wnats to go down),
     * You can reasonably assume that the distance to go down is further than just current position - farthers down position,
     * since the person gettign on has requested the elevator continues down. Don't know how far, so guess, in this case, 2.
    */
    int distUp = max(0, (mFloors.back().floor - currPos)); // Get the max distance to go up
    if ((mFloors.back().direction == 1) && (distUp > 0))
    {
        distUp += PENALTY_FACTOR; // In this case, the highest request wants to keep going up, so add more margin
    }
    int distDown = max(0, (currPos - mFloors.front().floor)); // Get max distance to go down.

    if ((mFloors.front().direction == -1) && (distDown > 0))
    {
        distDown += PENALTY_FACTOR; // In this case the lowest request wants to contiune to go down so assume distance to go down is further.
    }
    //--------------------------Determine Direction to move after a new request comes in-------------------------
    int dDist = distUp - distDown; // Find the difference between going up and down

    if (distUp == 0 && distDown == 0)
    {
        mDirect = 0; // Nowhere to go either direction so stay still.
    }
    else if (distUp == 0)
    {
        mDirect = -1; // There is nowhere to go up, so go down
    }
    else if (distDown == 0)
    {
        mDirect = 1; // There is nowhere to go down, so go up
    }
    else // Can go up or down potentially (requests on both sides)
    {
        /**
         * A note on the logic below -
         * The logic below enforces that direction changes can only be made in the event the elevator is stopped.
         * The rationale behind this is STARVATION.
         * Starvation is when one set of floors, or one floor, never gets the elevator because it is too far. The elevator
         * design is such that it moves to the closer extremity (the min of {max(upDist), max(DownDist)}) first, then switches directions.
         * Consider if the elevator is on floor 5, and trying to make its way towrds the up extremity, say floor 6. The lowest floor request is say
         * floor 3. If someone pushes a button for floor 8 (while elevator is still in transit), the elevator would just turn around before
         *  ever getting to floor 6. Then if someone pushed the button for floor 1, it's once more reverse direction without ever picking anyone up.
         * The below logic helps preventing that by ignoring requests while its moving, and will wiat to picking someone up before considering changing direction.
        */
        if ((mDirect == 0) || mStopped) // Can only make direction changes if the elevator is stopped/ has no where to go. Helps with starvation.
        {
            mDirect = -dDist/abs(dDist); // Direction is the opposite of the longer distance normalized to -1 or 1.
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
    lock_guard<mutex> lock(mMutexFloors);
    int diff = 0;
    //bool stopped = false;
    if (!mFloors.empty())
    {
        mDoneProcess = false;
        auto it = (mDirect > 0) ? (mFloors.end() - 1) : mFloors.begin();
        diff = it->floor - mCurrentFloor;
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
        mDoneProcess = true;
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
    condV.wait(lock, [&mDoneProcess= as_const(mDoneProcess)] { return mDoneProcess; });
    cout << "Done" << endl;
    this_thread::sleep_for(3s);
    return make_pair(mFloorsVisited, (mTimeTaken/1000));
}
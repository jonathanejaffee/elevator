/**
 * @author Jonathan Jaffee
 * @details - Following is to maybe get me a job.
*/

// Includes
#include "elevator.h"
#include <iostream>
#include <utility>
#include <algorithm>

using namespace std;
using namespace ele;

// Note - extremity = smallest or largest floor number

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
    mFloorsVisited.push_back(startingFloor);
    thread eleThread(&Elevator::runElevator, this); // Spawn the elevator thread, which processes mFloors
    eleThread.detach(); // Let it run independently
}

/**
 * Destructor
*/
Elevator::~Elevator()
{
    mRun = false; // This will kill the elevator run thread
    mDoneProcess = true;
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
        cout << "Checking number of people in lobby on floor " << floor.floor;
        floor.numPpl = mHumanDetect.detectHumans(floor.image); // Get the Number of people in the lobby
        cout << ". " << floor.numPpl << " people detected." << endl;
    }
    // Lock to protect shared resources
    lock_guard<mutex> lock(mMutexFloors); // Automatic storage duration, unlocks on destruction
    cout << "Adding New Floor Request: " << floor.floor << ", going: " << floor.direction << endl; 
    mFloors.push_front(floor); // Add to the request list
    sort(mFloors.begin(), mFloors.end()); // Sort the request list (uses comparator override in floorRequest class)
    int currPos = mCurrentFloor + mDirect; // Assume the elevator may be moving, so the current position will be last position plus direction of motion
    const int PENALTY_FACTOR = 2; // Penalty factor, hardcoded for now

    // Not a big fan of all the ifs, but I've been working on this too long now.

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

/**
 * Following function is for finding target floors that arent extremities.
 * @param diff = difference between current floor and target extremity (top/low floor depending on direction of travel)
*/
deque<flreq::floorRequest>::iterator Elevator::searchReqList(int diff)
{
    mDirect = diff/abs(diff); // Set the elevator direction

    // Lambda function used to match a target floor to current floor in std::find_if
    auto match = [&cf = as_const(mCurrentFloor)](const flreq::floorRequest& fl) -> bool { return (fl.floor == cf); };

    auto it = find_if(mFloors.begin(), mFloors.end(), match); // Find based on the match lambda function

    if (it != mFloors.end()) // We found a match
    {
        // We found a match, but it is not in the direction the elevator is moving (someone wants to go down, but the elevator is going up)
        if ((it -> direction != mDirect) && ((mDirect != 0) && (it ->direction != 0)))
        {
            if (it -> numPpl >= mLobbyCap) // Check if the lobby is at capacity, if it is, stop anyway and swap directions of travel.
            {
                bool changeDir = (it -> direction == 1) ? (mFloors.back().floor > mCurrentFloor) : (mFloors.front().floor < mCurrentFloor); // Should we swap directions (check that we can)
                if (changeDir) // Elevator direction should be changed to help clear this overfilling lobby
                {
                    mDirect = it -> direction; // need to make sure there is something in that direction
                    cout << "Passing by a full elevator lobby that wants to go in opposite direction. Switching elevator directions to clear lobby, for floor: ";
                    it->print();
                }
            }
            else
            {
                it = mFloors.end(); // Ignore since we cannot do anything (no floors to stop at in that direction)
            }
        }
    }
    return it; // Return a match for floor that is not extremity
}

/**
 * Following function is to check if the current floor is a tagrt floor
*/
bool Elevator::checkTargMatch()
{
    lock_guard<mutex> lock(mMutexFloors); // Lock mutex, auto storage duration, unlocks on destruction
    int diff = 0; // Difference between current floor and extremity in direction we are moving.
    // Check if there are any floors to move to
    if (!mFloors.empty())
    {
        mDoneProcess = false; // Are not done processing floors
        auto it = (mDirect > 0) ? (mFloors.end() - 1) : mFloors.begin(); // Get an iterator to the extremity in the direction we are moving
        diff = it->floor - mCurrentFloor; // Difference in floors between current floor and target extremity

        /**
         * A lambda function to find best request to service if there are multiple for a given floor extremity
         * Captures some class members as constants
        */
        auto findBest = [diff, it, &mDirect = as_const(mDirect), &mCurrentFloor = as_const(mCurrentFloor), &mFloors = as_const(mFloors)] () -> deque<flreq::floorRequest>::iterator
        {
            if (mFloors.size() < 2) // There are not enough floor in the request to bother
            {
                return it; // Return
            }
            deque<flreq::floorRequest>::iterator bestIt = it; // New iterator used to return what the best match is
            int newDiff = diff; // Keep track of new differences
            while (newDiff == 0)
            {
                // A good match is one that either matches the direction of travel of the elevator, or has no direction at all (people getting off)
                // The below check looks for that condition
                if ((bestIt -> direction == mDirect) || (bestIt -> direction == 0) || (mDirect == 0))
                {
                    return bestIt; // Return the match
                }
                (mDirect > 0) ? bestIt-- : bestIt++; // Increment or decrement the iterator

                // Check if we have gone too far, in which case returnt he original
                if ((bestIt < mFloors.begin()) || (bestIt >= mFloors.end()))
                {
                    return it;
                }
                newDiff = bestIt->floor - mCurrentFloor; // Recalc the difference
            }
            return it; // No matches, return original
        };

        if (diff == 0) // at one of the extremities
        {
            deque<flreq::floorRequest>::iterator bestIt = findBest(); // Find best match for this extremity
            cout << "Elevator hit target floor: ";
            bestIt->print();
            mFloorsVisited.push_back(bestIt->floor); // Add to the list of floors visited
            mFloors.erase(bestIt); // Erase from request deque
            mStopped = true; // Stop elevator to service this floor
        }
        else
        {
            deque<flreq::floorRequest>::iterator hit = searchReqList(diff); // Search for target floors that are not extremities
            if (hit != mFloors.end()) // Found a target floor for where the elevator is at
            {
                cout << "Elevator hit target floor: ";
                hit -> print();
                mFloorsVisited.push_back(hit->floor);
                mFloors.erase(hit);
                mStopped = true;
                return mStopped; // Return now to skip lower logic, stop elevator
            }
            mStopped = false; // No match, don't stop elevator
        }
    }
    else // Nothing to do
    {
        mDirect = 0;
        mStopped = true;
        mDoneProcess = true; // All requests processed
        condV.notify_one(); // Notify the conditional variable that we finished all requests
    }
    return mStopped;
}

/**
 * The following runs as a thread and controls the elevator by working through the deque of floor requests
 * NOTE - there is no explicit mutec locks in this funtion, so during those sleep call requests can still be added
*/
void Elevator::runElevator()
{
    cout << "Starting Elevator" << endl;
    this_thread::sleep_for(mElevatorStopTime); // Idk figured it should take some time to boot an elevator system
    cout << "Elevator Started" << endl;
    bool stop = true; // Elevator is stopped to start
    while (mRun) // Thread
    {
        if (!stop)
        {
            incCurrentFloor(); // Increment the floor the elevator is on
            this_thread::sleep_for(mElevatorSpeed); // Simulate the time it takes to get there
            mTimeTaken += mElevatorSpeed.count(); // increment time taken
        }
        else
        {
            cout << "Elevator is Stopped" << endl;
            this_thread::sleep_for(mElevatorStopTime); // Simulate how long the elevator stops for
            mTimeTaken += mElevatorStopTime.count();   // Record time delta
        }
        stop = checkTargMatch(); // Check for match between target floor and current floor
    }
}

/**
 * Check if the elevator has finished processing it's requests, if so return pair of floors visited as a vector, and unsigned int of time taken
*/
pair<vector<int>, unsigned int> Elevator::done()
{
    unique_lock<mutex> lock(mMutexFloors); // create lock
    // The below line will wait, without locking the lock, until it is notified by the condition variable. 
    // Allows for waiting without resource contention
    condV.wait(lock, [&mDoneProcess= as_const(mDoneProcess)] { return mDoneProcess; });
    mRun = false; // Kill thread
    cout << "Done" << endl; // We are done!
    this_thread::sleep_for(1s); // Let threads die
    return make_pair(mFloorsVisited, (mTimeTaken/1000)); // Convert to seconds
}
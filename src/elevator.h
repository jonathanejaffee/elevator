#pragma once

// Includes
#include <mutex>
#include <vector>
#include <deque>
#include <thread>
#include "floor_request.h"
#include "human_detection.h"
#include <condition_variable>

using namespace std;

namespace ele
{

/**
 * This Elevator class simulates an elevator runnign in real time.
*/
class Elevator
{

public:

    Elevator(unsigned int speed, unsigned int waitTime, int maxCap, int startingFloor); // Constructor
    Elevator() = delete;                                            // No defualt constructor
    ~Elevator();                                                    // destructor
    void addRequest(flreq::floorRequest floor);                     // Function to add a new floor request

    pair<vector<int>, unsigned int> done();                         // Function to check if the elevator is done

private:
    void runElevator();                                             // Runs the elevator
    int incCurrentFloor();                                          // Increments the floor the elevator is on
    deque<flreq::floorRequest>::iterator searchReqList(int diff);   // Searches the deque of floor requests to see if current floor is in the deque
    bool checkTargMatch();                                          // Checks if the current floor matches a target floor
    chrono::duration<unsigned int, milli> mElevatorSpeed;           // Elevator speed, milliseconds to move 1 floor.
    chrono::duration<unsigned int, milli> mElevatorStopTime;        // Time the elevator stops at a target floor, milliseconds
    int mLobbyCap;                                                  // Capacity for an elevator lobby. If this is exceeded, the elevator will priortize that floor
    int mCurrentFloor;                                              // Current floor the elevator is on
    bool mRun;                                                      // If the elevaor thread is running
    int mDirect;                                                    // Direction the elevator is moving, or desires to move next. 0 = none, 1 = up, -1 = down.
    mutex mMutexFloors;                                             // Mutex for protecting shared resources.
    deque<flreq::floorRequest> mFloors;                             // Deque of floor requests, chosen for better insertion or popping off either end of the deque
    hd::HumanDetector mHumanDetect;                                 // Human detector class, used to check if lobbies are too full
    std::condition_variable condV;                                  // A conditional variable used to wait until elevator is done without constantly having to lock mutex
    bool mDoneProcess = false;                                      // Boolean for if the elevator is done processing all requests. (will still keep thread alive)
    std::vector<int> mFloorsVisited;                                // Vector to keep track of floors visited   
    unsigned int mTimeTaken;                                        // Total time elevator took to visit all floors, including idle/stopped time
    bool mStopped;                                                  // Boolean or if the elevator is currently stopped at a floor.

};
}
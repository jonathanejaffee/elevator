// Headers
#include <iostream>
#include <fstream>
#include "floor_request.h"
#include "elevator.h"

using namespace std;

/**
 * @param argc = number of arguments. Will be program name + input file + optional arguments
 * @param argv array of char* input arguments. index 0 is program name, 1 input data file, 2 = elevator speed, 3 = time to stop on a floor,
 * 4 = max allowable capacity for lobbies, 5 = starting floor
*/
int main(int argc, char *argv[])
{
    unsigned int speed = 10;
    unsigned int stopTime = 2;
    int maxLobbyCap = 4;
    int startFloor = 1;
    string inputData;
    // Parse command line arguments
    if (argc > 5)
    {
        startFloor = atoi(argv[5]);
        maxLobbyCap = atoi(argv[4]);
        stopTime = atoi(argv[3]);
        speed = atoi(argv[2]);
    }
    else if (argc > 4)
    {
        maxLobbyCap = atoi(argv[4]);
        stopTime = atoi(argv[3]);
        speed = atoi(argv[2]);
    }
    else if (argc > 3)
    {
        stopTime = atoi(argv[3]);
        speed = atoi(argv[2]);
    }
    else if (argc > 2)
    {
        speed = atoi(argv[2]);
    }
    else if (argc < 2) // Make sure number of input arguments makes sense
    {
        cout << "ERROR -> Not enough input arguments. Please pass path to input data, and optionally after elevator speed, wait time, lobby capacity, and start floor." << endl;
        exit(-1);
    }

    // Parse the input csv
    inputData.assign(argv[1]);
    cout << "Input data: " << inputData << endl;
    ifstream inputFile(inputData);
    if (!inputFile.is_open()) throw runtime_error("Could not open input file, exiting.");
    string line;
    getline(inputFile, line);
    cout << "Parsing: " << line << endl;
    ele::Elevator elevator(speed, stopTime, maxLobbyCap, startFloor);
    while (getline(inputFile, line))
    {
        flreq::floorRequest floorData(line);
        elevator.addRequest(floorData);
        chrono::duration<int, milli> dt(floorData.time*1000); // convert to seconds
        this_thread::sleep_for(dt); // Meter the requests being sent to mimick people pushing elevator buttons at different times.
    }
    cout << "All requests sent to elevator" << endl;
    pair<vector<int>, unsigned int> retPair = elevator.done();
    cout << "Elevator sim complete. Total time: " << retPair.second << endl;
    cout << "Floors visited (in order): " << endl;
    for (int i = 0; i < retPair.first.size(); i++)
    {
        cout << retPair.first[i] << endl;
    }
    return 0;
}
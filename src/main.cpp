// Headers
#include <iostream>
#include <fstream>
#include <algorithm> // Used for sort
#include "floor_request.h"
#include "human_detection.h" // Temp
#include "elevator.h"
using namespace std;

/**
 * @param argc = number of arguments. Will be program name + the floor requests
 * @param argv array of char* input arguments. index 0 is program name, 1 is starting floor, rest are floors to visit
*/
int main(int argc, char *argv[])
{
    int speed = 10;
    string inputData;
    if (argc > 2)
    {
        speed = atoi(argv[2]);
    }
    else if (argc < 2) // Make sure number of input arguments makes sense
    {
        cout << "ERROR -> Not enough input arguments. Please pass path to input data, and optionally elevator speed." << endl;
        exit(-1);
    }
    inputData.assign(argv[1]);
    cout << "Input data: " << inputData << endl;
    ifstream inputFile(inputData);
    if (!inputFile.is_open()) throw runtime_error("Could not open input file, exiting.");
    string line;
    getline(inputFile, line);
    cout << "Parsing: " << line << endl;
    ele::Elevator elevator(10, 8);
    while (getline(inputFile, line))
    {
        flreq::floorRequest floorData(line);
        hd::HumanDetector hd;
        int numPpl = hd.loadImage(floorData.image);
        floorData.numPpl = numPpl;
        elevator.addRequest(floorData);
        chrono::duration<int, milli> dt(floorData.time*1000);
        this_thread::sleep_for(dt);
    }
    cout << "All requests sent" << endl;
    elevator.done();
    //this_thread::sleep_for(120s);
    return 0;
}
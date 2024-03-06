#pragma once
// Includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

/**
 * Following struct is used to hold the data for a floor request
*/
namespace flreq
{
struct floorRequest
{
    int floor; // floor to visit
    int direction; // direction requested - 0 means none (passenger is on the elevator), 1 = up, -1 = down
    string image; // CCTV image of the elevator lobby
    int time; // Time to stagger the request (used only to simulate someone pushing buttom async)
    int numPpl; // Number of people in the elevator lobby
    floorRequest(){};

    // Ths constructor sets the imformation from a line in the input csv
    floorRequest(string line, char delim = ',')
    {
        const int EXPECTED_SZ = 4;
        stringstream ss(line);
        vector<string> splits;
        splits.reserve(EXPECTED_SZ);
        string split;
        while (getline(ss, split, delim))
        {
            splits.push_back(split);
        }
        if (splits.size() != EXPECTED_SZ) // that aint good
        {
            cout << "Warning - Could not parse line for floor request data." << endl;
            floor = -1;
            direction = 0;
            image = "";
            time = 0;
        }
        else
        {
            floor = stoi(splits[0]);
            direction = stoi(splits[1]);
            image = splits[2];
            time = stoi(splits[3]);
        }
        numPpl = 0; // For now, default this to 0. Only run image recognition on an as needed basis.
    }

    // Override the < operator ffor comparisons. Use floor.
    bool operator<(const floorRequest& other) const { return floor < other.floor; }

    // Print the floor request
    void print()
    {
        cout << "Floor: " << floor << ", Direction: " << direction << endl;
    }
};
}
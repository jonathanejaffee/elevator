#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

namespace flreq
{
struct floorRequest
{
    int floor;
    int direction;
    string image;
    int time;
    floorRequest(){};

    floorRequest(string line, char delim = ',')
    {
        const int expectedSz = 4;
        stringstream ss(line);
        vector<string> splits;
        splits.reserve(expectedSz);
        string split;
        while (getline(ss, split, delim))
        {
            splits.push_back(split);
        }
        if (splits.size() != expectedSz)
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
    }
};
}
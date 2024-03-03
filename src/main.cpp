
// Headers
#include <iostream>
#include <algorithm>
using namespace std;

int main(int argc, char *argv[])
{
    cout << "Here we go " << endl;
    if (argc < 2)
    {
        cout << "ERROR -> Not enough input arguments. Please pass list of integers with each integer as a argument. Exiting" << endl;
        exit(-1);
    }
    int numFloors = argc - 1;
    int floorReq[numFloors];
    int startFloor = atoi(argv[1]);
    for (int i = 2; i < argc; i++)
    {
        floorReq[i - 1] = atoi(argv[i]);
        cout << floorReq[i-1] << endl;
    }
    sort(floorReq+1, floorReq + numFloors);
    floorReq[0] = startFloor;
    cout << "------------------------------" << endl;
    for (int i = 0; i < numFloors; i++)
    {
        cout << floorReq[i] << endl;
    }
    const int floorTravelTime = 10;
    int distUp = floorReq[(numFloors - 1)] - startFloor;
    int distDown = startFloor - floorReq[1];
    cout << "Start floor: " << startFloor << endl;
    cout << "Dist down: " << distDown << endl;
    cout << "Dist up: " << distUp << endl;
    //int floorVisits[numFloors + 1];
    //floorVisits[0] = startFloor;
    int distTravel = 0;
    if (distUp < distDown) // Go up first
    {
        distTravel = ((2*distUp) + distDown)*floorTravelTime;
        reverse(floorReq + 1, floorReq + numFloors);
    }
    else
    {
        distTravel = ((2*distDown) + distUp)*floorTravelTime;
    }
    cout << "Total Travel Time: " << distTravel << endl;
    cout << "Floors visted in order: ";
    for (int i = 0; i < numFloors - 1; i++)
    {
        cout << floorReq[i] << ", ";
    }
    cout << floorReq[numFloors-1] << endl;
    return 0;
}
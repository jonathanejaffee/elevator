
// Headers
#include <iostream>
#include <algorithm> // Used for sort
using namespace std;

/**
 * @param argc = number of arguments. Will be program name + the floor requests
 * @param argv array of char* input arguments. index 0 is program name, 1 is starting floor, rest are floors to visit
*/
int main(int argc, char *argv[])
{
    if (argc < 2) // Make sure number of input arguments makes sense
    {
        cout << "ERROR -> Not enough input arguments. Please pass list of integers with each integer as a argument. Exiting" << endl;
        exit(-1);
    }
    int numFloors = argc - 1; // Number of floors to visit
    int floorReq[numFloors]; // Array of floors to visit
    int startFloor = atoi(argv[1]); // What the starting floor is
    for (int i = 2; i < argc; i++)
    {
        floorReq[i - 1] = atoi(argv[i]); // Only populate from 1:n indexes where n is the number of floors to visit - 1. This is because first floor will be start floor
    }
    sort(floorReq+1, floorReq + numFloors); // Sort the requested floors to visit.
    floorReq[0] = startFloor; // The floorReq array will be reused to be the floors visited in order, so set first visted floor to the startign floor.
    const int floorTravelTime = 10; // time to travel 1 floor.
    int distUp = max(0, (floorReq[(numFloors - 1)] - startFloor)); // The distance from the starting floor for the top most floor to visit
    int distDown = max(0, startFloor - floorReq[1]); // Distance from starting floor to lowest floor to visit
    int distTravel = 0; // Distance traveled
    /**
     * A note on the approach here -
     * Will move to one extremity first, either lowest or highest floor to visit. Then will move to other floors in a single direction. 
     * Will move to the closer extremity first. For example - if start is 3, and requests are 1, 2, 8, 9 - will first go to 1, then go to 2-8-9.
     * This be quickest way of going, with cost fo 2*shorter exteremity + long extremity. Very similar to LOOK disk scheduling algorithm. 
     * Quick note - order in the above example could also be 2, 1, 8, 9 with same cost, 1, 2, 8, 9 is used for reducing complexity in forming output array.
    */
    if (distUp < distDown) // Go up first
    {
        distTravel = ((2*distUp) + distDown)*floorTravelTime;
        reverse(floorReq + 1, floorReq + numFloors); // Since will start at the top, our order visted will just be the reversal of the floor visit requests
    }
    else // Go down first
    {
        distTravel = ((2*distDown) + distUp)*floorTravelTime; // Note that floor visits will eb sorted order of inital requests.
    }
    // Print the results
    cout << "Total Travel Time: " << distTravel << endl;
    cout << "Floors visted in order: ";
    for (int i = 0; i < numFloors - 1; i++)
    {
        cout << floorReq[i] << ", ";
    }
    cout << floorReq[numFloors-1] << endl;
    return 0;
}
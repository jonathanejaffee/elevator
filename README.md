*** This is the simple implementation of the elevator - for complex see branch: complex_branch ***

-   Assumptions:
    -   Elevator has no information regarding intended direction of passenger, as such order 1-2-3 is equivalent to 3-2-1 sice desired traversal/visit order
        is not defined
    -   It's okay to print output (was not sure if strictly needed to be a list, I just print it to stdout)
    -   A floor only counts as being visted if the elevator "stops". That is, the elevator may pass by the same floor twice, but may choose which occassion
        to stop on. Ie, if the elevator goes 3-2-1-2-3, it can choose to either return 3-2-1 or 1-2-3, both of which reports of visting order being equivalent,
        aaccording to assumption 1. (Note - The path length/transit time would be that of 3-2-1-2-3, but order of visit can be selected by elevator, it may visit going down, or going up) (complex_elevator branch addresses this more)

-   How to use:
    -   ./elevator 1 2 3 4 (each integer argument is the desired floor, starting with the starting floor)
    -   Ouput = printed travel time and floors visited in order

-   Implementation:
    -   The implementation is very similar to the LOOK disk scheduling algorithm. In this algorith, the servicing
        will occur in one direction, until all items on that path are serviced, the direction is reversed same process is repeated.
        In this implementation, the inital direction serviced is based on which path is shorter. If it is shorter to go up to highest floor,
        as opposed to go down to lowest floor, then the elevator will go up first. This reduces path length, since in this case the elevator will
        always need to repeat one path (unless iti only going up or down) so better to repeat the shorter path. Implementation of this is done via sorting
        (ascending) the input array, the if the evevator is going to go down first, it just returns the sorted order, otherwise the reverse sorted order, with
        starting floor prepended. What this would mean is, if the elevator was on floor 3, and needed to go to 1, 2, and 8, it would sort the requests to [1, 2, 8]. Since 1 is closer to it's starting point of 3, it will go 3-2-1-2-3-4-5-6-7-8, where travel time is 90, and floor vists in order are 3-1-2-8.
        This could also be written as 3-2-1-8, as described in the last assumption, but is always returned in either ascending or descending order (not including first element) for simplicity since array is being sorted anyway.

    -   Implementation is in main.cpp

-   Example Run

-   (base) jjaffee@jjaffee-pc:~/proj/elevator> ./elevator 3 1 2 8
-   Total Travel Time: 90
-   Floors visted in order: 3, 1, 2, 8
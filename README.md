**** This is the complex version of the elevator - for the simple version switch to branch simple-elevator ****

This version of the elevator incorporates additional complexity to account for new information, and be an overall higher fidelity sim. 

Added complexities:

-    1. Realtime execution/modeling of ansynchronous requests

        In a real elevator, input requests to floors will come irregularly throughout the elevators operations.
    Requests may come in very spaced apart (people slowly pressing buttons at different times), or all at once (if multiple floor buttons are pressed at once).
    To model this, floor requests are read in form a .csv, where an included time field is added to represent the relative spacing between requests.
    Furthermore, while these requests, the elevator still needs to operate independently. To do this, the elevator and it's dynamics are a separate thread
    that independently processes the floor requests as they flow in. The floor requests are posted to a shared deque asynchronously, in a non-blocking manner, 
    relative to the elevator operation. The approach to handling these requests is similar to the LOOK disk scheduling algorithm, with the exception that
    there is added logic to go to either the lowest, or highest floor first, depending on which is closer. The elevator may make stops along the way when ideal (further explained below). Lastly - since requests are flowing in asychronously andnot all up-front,  the elevator only processes new requests when
    stopped. This helps with starvation, where if rapidly changing distances between lowest and tallest floor request can cause the elevator to operate. 
    SEE - elevator.cpp:121 for more info.

-   2. Timing

        Actual timing of how long it takes the elevator to move is simulated via sleep calls. So if the elevator takes 10 seconds to move 1 floor, and it
    moves 3 floors, the program will take 30 seconds to run for those 3 floors. In addition, the time it takes to stop at a floor is simulated as well in
    the same manner. Both these values are configurable via command line argument.
        

-   3. Incorporating direction of request

        In a real elevator, generally a request made has a direction associated to it. If you are on floor 1 and want to go up, you hit the up arrow.
    If you are on floor 10 and want to go down, you would hit the down arrow. The up direction is associated with an integer value of 1, down with -1.
    In the case you were in the elevator, there would not be an associated diretcion since you are getting off, this will be associated with a value of 0.
    One more note - as the elevator is moving, it will only stop at floor that haa a request direction that matches the direction the elevator is moving (or is 0). So if there is a request to go down on floor 5, and the elevator is going to from floor 3 to 6, it would not stop at 5, until is swings around 
    the other direction. If the elevator was going from 3 to 6 and an up request came in on floor 5, it would stop. There is a special exception to this rule,
    explaine below.

-   3. For fun - Adherence to fire safety code/occupancy (openCV image recognition)

        Imagine an elevator lobby on a given floor, maybe a big conference just let out on that floor, and people are flooding into that lobby. Perhaps
    more people are entering that lobby than can safely be there (occupancy according to fire code is reached or exceeded). In this case, it would be 
    preferable to clear out this lobby faster, even at the cost of overall travel time. This use case is simulated here. If the occupancy in a lobby is
    too much, the elevator as is passes it will stop, even if it's in the opposite direction that the elevator was intending to go. If it is in the opposite direction, the elevator will switch directions to more quickly clear out that overfilled lobby. BUT - how does the elevator know how many people are in a given lobby? To do this, it is imagined that the elevator has access to security cameras that it can request real time images from. The elevator then takes the picture of the lobby and runs human image recognition software to get a count of the number of people in the loccy. This is only done for floor requests coming from a lobby (direction 1 or -1). In this sim, when that occurs, a path to a test image is passed to a human-recognition class that runs openCV's pretrained people detector model (from hisogram of gradients module) on the image, and returns an estimate of the number of people in it. Images are supposed to represent elevator lobbies (they aren't pics of that but I don't have any of those laying around). These images can be found in ./data/. There are 101.

Some limitations:
    - Capacity of elevator is not considered.


Files:
    - main.cpp: Reads in csv data (found in ./input/) and passes it to elevator based on relative time order (spaces by delat time).
                    Waits for program to end.

    - floor_request.h: Defines a struct to hold the floor request data in.

    - elevator.h/.cpp: Defines the elevator sim logic

    - human_detection.h/.cpp: openCV based human image detection class

    - generateInputData.py: Python script to randomly generate input data

    - ./input - contains .csv input data files in which to run the sime

    - ./data - contains images in which human_detction.h/.cpp uses for image recognition. The input files in ./input contain a path to specific images to use in ./data




./elevator ./input/inputDataDemoFullLobby.csv 2 2 3 6 - showcase reverse logic for full lobby
./elevator ./input/inputDataDemoFullLobby.csv 2 2 10 6 - regular realtime logic

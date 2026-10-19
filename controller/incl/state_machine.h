// Filename: state_machine.h
// Description: Header containing external functions and data types for the state machine operation
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <string>

#include "abstract_thread.h"

namespace StateMachine
{

// Possible states of the rocket.
enum RocketState
{
    standby = 0,   // Rocket is on the pad, waiting for launch.
    launch = 1,    // Rocket is in flight.
    postBurnout = 2, // Rocket has completed its burn.
    postApogee = 3,  // Rocket has reached its apogee.
    NUM_STATES
}; 

// Shorter name for the RocketState type.
typedef RocketState State;

constexpr const char* stateStrings[NUM_STATES] = 
{
    "Standby",
    "Launch",
    "Post-Burnout",
    "Post-Apogee"
};

// Stores the latest rocket measurements.
struct RocketData
{
    double altitude;
    double velocity;
    double acceleration;
};

class StateMachine
{
public:
    // Start the state machine in the specified state.
    explicit StateMachine(State initialState = standby)
        : currentState(initialState), consecutiveReadings(0)
    {
    }

    State currentState;
    int consecutiveReadings = 0;

    // Process a new reading and return the current state.
    State update(const RocketData& rocket);
};

class StateMachineThread : public SNAIL::AbstractThread
{
public:

private:
    StateMachineThread();    
    ~StateMachineThread();

    int init();
    int loop(int initStatus);
};


}   

#endif // STATE_MACHINE_H
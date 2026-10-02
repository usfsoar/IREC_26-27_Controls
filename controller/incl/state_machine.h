// Filename: state_machine.h
// Description: Header containing external functions and data types for the state machine operation

#include <string>

#include "abstract_thread.h"

namespace StateMachine
{

enum RocketState
{
    standby = 0,
    launch = 1,
    postBurnout = 2,
    postApogee = 3,
    NUM_STATES
} typedef State;

constexpr char* stateStrings[NUM_STATES] = 
{
    "Standby",
    "Launch",
    "Post-Burnout",
    "Post-Apogee"
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
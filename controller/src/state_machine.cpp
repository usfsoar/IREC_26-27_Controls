// Filename: state_machine.cpp
// Description: 


#include "state_machine.h"

namespace StateMachine
{

// Number of consecutive readings required to confirm a transition.
constexpr int REQUIRED_READINGS = 3;

//TEMPORARY THRESHOLDS FOR TESTING
// Standby -> Launch thresholds
constexpr double LAUNCH_ALTITUDE_THRESHOLD = 1.0;
constexpr double LAUNCH_VELOCITY_THRESHOLD = 0.5;
constexpr double LAUNCH_ACCELERATION_THRESHOLD = 0.0;

// Launch -> Post-Burnout thresholds
constexpr double BURNOUT_ALTITUDE_THRESHOLD = 1.0;
constexpr double BURNOUT_VELOCITY_THRESHOLD = 0.5;
constexpr double BURNOUT_ACCELERATION_THRESHOLD = 0.0;

// Post-Burnout -> Post-Apogee thresholds
constexpr double APOGEE_ALTITUDE_THRESHOLD = 1.0;
constexpr double APOGEE_VELOCITY_THRESHOLD = 0.5;
constexpr double APOGEE_ACCELERATION_THRESHOLD = 0.0;


// Process a new rocket measurement and update the state if needed.
State StateMachine::update(const RocketData& rocket)
{
    bool transitionCondition = false;
    State nextState = currentState;

// Determine the condition required to leave the current state.
switch (currentState)
{
    case standby:
        // Standby -> Launch
        transitionCondition =
            rocket.altitude > LAUNCH_ALTITUDE_THRESHOLD &&
            rocket.velocity > LAUNCH_VELOCITY_THRESHOLD &&
            rocket.acceleration > LAUNCH_ACCELERATION_THRESHOLD;

        nextState = launch;
        break;

    case launch:
        // Launch -> Post-Burnout
        transitionCondition =
            rocket.altitude > BURNOUT_ALTITUDE_THRESHOLD &&
            rocket.velocity > BURNOUT_VELOCITY_THRESHOLD &&
            rocket.acceleration <= BURNOUT_ACCELERATION_THRESHOLD;

        nextState = postBurnout;
        break;

    case postBurnout:
        // Post-Burnout -> Post-Apogee
        transitionCondition =
            rocket.altitude > APOGEE_ALTITUDE_THRESHOLD &&
            rocket.velocity < APOGEE_VELOCITY_THRESHOLD &&
            rocket.acceleration <= APOGEE_ACCELERATION_THRESHOLD;

        nextState = postApogee;
        break;

    case postApogee:
        return currentState;

    default:
        return currentState;
}

    // Count readings that continuously satisfy the condition.
    if (transitionCondition)
    {
        consecutiveReadings++;

        // Transition only after enough consecutive readings.
        if (consecutiveReadings >= REQUIRED_READINGS)
        {
            currentState = nextState;
            consecutiveReadings = 0;
        }
    }
    else
    {
        // A reading that fails the condition resets the counter.
        consecutiveReadings = 0;
    }

    return currentState;
}


int StateMachineThread::init()
{
    // TODO: Initialize rocket data and state-tracking variables.
    return 0;
}


int StateMachineThread::loop(int initStatus)
{
    // TODO: Read sensor data and pass it to StateMachine::update().
    (void)initStatus;

    return 0;
}

} // namespace StateMachine

// Filename: state_machine.cpp
// Description: 


#include "state_machine.h"

namespace StateMachine
{

// Number of consecutive readings required to confirm a transition.
constexpr int REQUIRED_READINGS = 3;

// TEMPORARY thresholds.
// These must be validated against the actual flight and sensor design.
constexpr double ALTITUDE_THRESHOLD = 1.0;
constexpr double VELOCITY_THRESHOLD = 0.5;
constexpr double ACCELERATION_THRESHOLD = 0.0;


// Process a new rocket measurement and update the state if needed.
State StateTracker::update(const RocketData& rocket)
{
    bool transitionCondition = false;
    State nextState = currentState;

    // Determine the condition required to leave the current state.
    switch (currentState)
    {
        case standby:
            // Require altitude and upward velocity to exceed thresholds.
            transitionCondition =
                rocket.altitude > ALTITUDE_THRESHOLD &&
                rocket.velocity > VELOCITY_THRESHOLD;

            nextState = launch;
            break;

        case launch:
            // Preliminary burnout condition.
            transitionCondition =
                rocket.acceleration <= ACCELERATION_THRESHOLD;

            nextState = postBurnout;
            break;

        case postBurnout:
            // Negative velocity indicates descent.
            transitionCondition =
                rocket.velocity < -VELOCITY_THRESHOLD;

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
    // TODO: Read sensor data and pass it to StateTracker::update().
    (void)initStatus;

    return 0;
}

} // namespace StateMachine

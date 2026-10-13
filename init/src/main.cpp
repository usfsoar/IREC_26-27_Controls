// Filename: main.cpp
// Description: 

#include <iostream>

#include "state_machine.h"


int main(int argc, char *argv[]) 
{
    std::cout << "Hello World" << "\n";

    // Check for power loss


    // Perform any general hardware initialization


    // Start threads
    StateMachine::StateMachineThread::run();
    

    // Block on threads
        // If any threads end without pass, handle

    return 0;
}
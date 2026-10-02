
#include "abstract_thread.h"

int SNAIL::AbstractThread::run()
{
    // POSIX Thread creation on taskbody here
}

int SNAIL::AbstractThread::taskBody()
{
    int status = init();

    return loop(status);
}
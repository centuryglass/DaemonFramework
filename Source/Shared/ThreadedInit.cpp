#include "ThreadedInit.h"
#include "Debug.h"
#include <chrono>

#ifdef DF_DEBUG
// Print the application and class name before all info/error messages:
static const constexpr char* messagePrefix
    = "DaemonFramework::ThreadedInit::";
#endif


// Stops the init thread if it is still running on destruction.
DaemonFramework::ThreadedInit::~ThreadedInit()
{
    cancelInit();
}


// Return whether initialization has started.
bool DaemonFramework::ThreadedInit::startedInit()
{
    std::lock_guard<std::mutex> lock(initMutex);
    return initStarted;
}


// Return whether initialization has finished.
bool DaemonFramework::ThreadedInit::finishedInit()
{
    std::lock_guard<std::mutex> lock(initMutex);
    return initFinished;
}


// Return whether initialization was successful.
bool DaemonFramework::ThreadedInit::successfulInit()
{
    std::lock_guard<std::mutex> lock(initMutex);
    return initSucceeded;
}


// Wait for the thread to finish initializing.
bool DaemonFramework::ThreadedInit::waitForInit(const int timeout)
{
    std::unique_lock<std::mutex> lock(initMutex);
    if (initFinished)
    {
        return true;
    }
    if (timeout < 1)
    {
        initCondition.wait(lock);
        return true;
    }
    using namespace std::chrono;
    const seconds maxWait(timeout);
    std::cv_status waitResult = initCondition.wait_for(lock, maxWait);
    return waitResult != std::cv_status::timeout;

}


// If not already initialized or initializing, start the initialization thread
// and run the initialization function.
void DaemonFramework::ThreadedInit::startInitThread()
{
    std::lock_guard<std::mutex> lock(initMutex);
    if (! initStarted)
    {
        initStarted = true;
        int initResult = pthread_create(&initThreadID, nullptr, threadAction,
                this);
        if (initResult != 0)
        {
            DF_DBG(messagePrefix << __func__ 
                    << ": Failed to start init thread.");
            initFinished = true;
            initSucceeded = false;
        }
    }
}


// If still initializing, force the initialization thread to stop.
void DaemonFramework::ThreadedInit::cancelInit()
{
    int cancelResult;
    {
        std::lock_guard<std::mutex> lock(initMutex);
        if (! initStarted || initFinished || initThreadID == 0)
        {
            return;
        }
        DF_DBG_V(messagePrefix << __func__ << ": Force-closing init thread.");
        errno = 0;
        cancelResult = pthread_cancel(initThreadID);
    }
    if (cancelResult != 0)
    {
        DF_DBG(messagePrefix << __func__ 
                << ": Encountered an error when cancelling the init thread.");
        DF_PERROR("Init pthread_cancel error");
    }
    errno = 0;
    if (pthread_join(initThreadID, nullptr) != 0)
    {
        DF_DBG(messagePrefix << __func__ 
                << ": Encountered an error when joining the init thread.");
        DF_PERROR("Init pthread_join error");
    }
}


//The main callback function that will run on the new thread.
void* DaemonFramework::ThreadedInit::threadAction(void* threadInit)
{
    DF_DBG_V(messagePrefix << __func__ << ": Init thread running.");
    ThreadedInit* threadInitObject = static_cast<ThreadedInit*>(threadInit);
    const bool initResult = threadInitObject->threadedInitAction();
    {
        std::lock_guard<std::mutex> lock(threadInitObject->initMutex);
        threadInitObject->initFinished = true;
        threadInitObject->initSucceeded = initResult;
    }
    threadInitObject->initCondition.notify_all();
    return nullptr;
}

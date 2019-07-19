/**
 * @file  ThreadedInit.h
 *
 * @brief  Abstract basis for classes that perform an initialization step within
 *         a new thread.
 */

#include <pthread.h>
#include <mutex>
#include <condition_variable>

namespace DaemonFramework
{
    class ThreadedInit;
}

class DaemonFramework::ThreadedInit
{
public:
    /**
     * @brief  Initializes thread control structures.
     */
    ThreadedInit();

    /**
     * @brief  Stops the init thread if it is still running on destruction.
     */
    virtual ~ThreadedInit();

    /**
     * @brief  Return whether initialization has started.
     *
     * @return  Whether startInitThread() has been called.
     */
    bool startedInit();

    /**
     * @brief  Return whether initialization has finished.
     *
     * @return  Whether the initialization thread has finished running.
     */
    bool finishedInit();

    /**
     * @brief  Return whether initialization was successful.
     *
     * @return  Whether the thread finished, and threadedInit returned true.
     */
    bool successfulInit();

    /**
     * @brief  Wait for the thread to finish initializing.
     *
     *  If called without first calling startInitThread(), this will immediately
     * exit, returning false.
     *
     * @param timeout  Maximum time in seconds to wait, or any value less than 
     *                 one to wait indefinitely.
     *
     * @return         True if the init thread finished successfully, false
     *                 otherwise.
     */
    bool waitForInit(const int timeout = -1);

protected:
    /**
     * @brief  If not already initialized or initializing, start the
     *         initialization thread and run the initialization function.
     */
    void startInitThread();

    /**
     * @brief  If still initializing, force the initialization thread to stop.
     */
    void cancelInit();

private:
    /**
     * @brief  The main threaded initialization routine, to be implemented by
     *         inheriting classes.
     *
     * @return  Whether the initialization succeeded.
     */
    virtual bool threadedInit() = 0;

    /**
     * @brief  The main callback function that will run on the new thread.
     *
     * @param threadInit  The ThreadedInit object that will be initialized on
     *                    the thread.
     *
     * @return            Nullptr, as the return value is not needed.
     */
    static void* threadAction(void* threadInit);

    // Control access to thread state data:
    std::mutex initMutex;
    std::condition_variable initCondition;
    // Store thread initialization state data:
    bool initStarted = false;
    bool initFinished = false;
    bool initSucceeded = false;
    pthread_t initThreadID = 0;
};

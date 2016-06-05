/**
 * File: thread-pool.h
 * -------------------
 * This class defines the ThreadPool class, which accepts a collection
 * of thunks (which are zero-argument functions that don't return a value)
 * and schedules them in a FIFO manner to be executed by a constant number
 * of child threads that exist solely to invoke previously scheduled thunks.
 */

#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>     // for size_t
#include <functional>  // for the function template used in the schedule signature
#include <thread>      // for thread
#include <vector>      // for vector
#include <queue>
#include <stack>
#include <condition_variable>

using JobType = std::function<void(void)>;

struct WorkerInfo {
    bool working = false;
    bool alive = false;                 // whether the worker has been spawned
    std::mutex mt;
    std::condition_variable cv;
    JobType job;
    std::thread th;
};

struct DispatcherInfo {
    std::condition_variable cv_new_job;
	std::condition_variable cv_available_worker;
    std::thread th;
};

class ThreadPool {
public:

    /**
     * Constructs a ThreadPool configured to spawn up to the specified
     * number of threads.
     */
    ThreadPool(size_t numThreads);

    /**
     * Schedules the provided thunk (which is something that can
     * be invoked as a zero-argument function without a return value)
     * to be executed by one of the ThreadPool's threads as soon as
     * all previously scheduled thunks have been handled.
     */
    void schedule(const std::function<void(void)>& thunk);

    /**
     * Blocks and waits until all previously scheduled thunks
     * have been executed in full.
     */
    void wait();

    /**
     * Waits for all previously scheduled thunks to execute, then waits
     * for all threads to be be destroyed, and then otherwise brings
     * down all resources associated with the ThreadPool.
     */
    ~ThreadPool();

private:

    const int kNumThreads;
    std::thread dt;                // dispatcher thread handle
    std::vector<std::thread> wts;  // worker thread handles
    std::queue<JobType> jq;        // queue of jobs
    std::stack<int> available_workers;
	std::mutex mt;
    std::vector<WorkerInfo> worker_infos;
    bool running;
    DispatcherInfo d_info;
    std::condition_variable cv_destructor;

    /**
     * ThreadPools are the type of thing that shouldn't be cloneable, since it's
     * not clear what it means to clone a ThreadPool (should copies of all outstanding
     * functions to be executed be copied?).
     *
     * In order to prevent cloning, we remove the copy constructor and the
     * assignment operator.  By doing so, the compiler will ensure we never clone
     * a ThreadPool.
     */
    ThreadPool(const ThreadPool& original) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
    void dispatcher();
    void worker(size_t workerID);
};

#endif

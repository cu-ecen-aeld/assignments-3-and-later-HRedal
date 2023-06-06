#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    thread_func_args->thread_complete_success = false;
    usleep(thread_func_args->wait_to_obtain_msM * 1000);
    int returnCode = pthread_mutex_lock(thread_func_args->mutexM);
    if (returnCode != 0)
    {
      thread_func_args->thread_complete_success = false;
      ERROR_LOG("failed to obtain the mutex\n");
    }
    else
    {
	    usleep(thread_func_args->wait_to_release_msM * 1000);
	    returnCode = pthread_mutex_unlock(thread_func_args->mutexM);
	    if (returnCode != 0)
	    {
             thread_func_args->thread_complete_success = false;
             ERROR_LOG("failed to obtain the release\n");
	    }
	    else
	    {
	      thread_func_args->thread_complete_success = true;
	    }
    }
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data* theThreadData   = malloc(sizeof(theThreadData));
    bool                theResMethod    = false;
    
    if (theThreadData == 0L)
    {
      ERROR_LOG("Out of memory\n");
      return false;
    }

    
    theThreadData->thread_complete_success = false;
    theThreadData->wait_to_obtain_msM      = wait_to_obtain_ms;
    theThreadData->wait_to_release_msM     = wait_to_release_ms;
    theThreadData->mutexM                  = mutex;
    
    int returnCode = pthread_create(thread, NULL, &threadfunc, theThreadData);
    
    if (returnCode != 0)
    {
      ERROR_LOG("Failed to create the thread");
      theResMethod = false;
    }
    else
    {
      theResMethod = true;
    }
        
    return theResMethod;
}


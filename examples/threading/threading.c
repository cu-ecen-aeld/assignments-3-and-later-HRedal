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
    
    printf("threadfunc - Enter\n");
    usleep(thread_func_args->wait_to_obtain_msM * 1000);
    printf("threadfunc - 1\n");
    pthread_mutex_lock(thread_func_args->mutexM);
    printf("threadfunc - 2\n");
    usleep(thread_func_args->wait_to_release_msM * 1000);
    printf("threadfunc - 3\n");
    pthread_mutex_unlock(thread_func_args->mutexM);
    
    printf("threadfunc - Exit\n");
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
    printf("start_thread_obtaining_mutext - Enter\n");
    struct thread_data* theThreadData   = malloc(sizeof(theThreadData));
    pthread_t           theThread;
    int                 theError     = 0;
    bool                theResMethod = false;
    
    printf("start_thread_obtaining_mutext - 0\n");
    
    theThreadData->wait_to_obtain_msM   = wait_to_obtain_ms;
    theThreadData->wait_to_release_msM  = wait_to_release_ms;
    theThreadData->mutexM               = mutex;
    
    printf("start_thread_obtaining_mutext - 1\n");
    theError = pthread_create(&theThread, NULL, &threadfunc, theThreadData);
    printf("start_thread_obtaining_mutext - 2\n");
    theThreadData->threadM = theThread;
    pthread_join(theThread, (void**) &theThreadData);
    
    free(theThreadData);
    
    if (theError)
    {
      theResMethod = false;
    }
    else
    {
      theResMethod = true;
    }
    
    printf("start_thread_obtaining_mutext - Exit\n");
    return theResMethod;
}


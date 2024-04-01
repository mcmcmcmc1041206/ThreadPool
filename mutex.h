#pragma once
#include <mutex>

class MutexLock
{
public:
    MutexLock(){pthread_mutex_init(&mutex_,NULL);}
    ~MutexLock(){
        pthread_mutex_lock(&mutex_);
        pthread_mutex_destroy(&mutex_);}

    void lock(){pthread_mutex_lock(&mutex_);}
    void unlock(){pthread_mutex_unlock(&mutex_);}
    pthread_mutex_t* get(){return &mutex_;}
private:
    pthread_mutex_t mutex_;
};

class MutexLockGuard
{
public:
    MutexLockGuard(MutexLock& mutex):mutexlock_(mutex)
                {mutexlock_.lock();}
    ~MutexLockGuard(){mutexlock_.unlock();}
private:
    MutexLock &mutexlock_;
};


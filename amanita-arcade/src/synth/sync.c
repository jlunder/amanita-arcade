#include "sync.h"


extern void sync_usleep(int32_t useconds)
{
#if defined(SYNC_USE_PTHREAD)
	usleep(useconds);
#else
    swiDelay(useconds);
#endif
}


intptr_t sync_atomic_swap(void volatile * dest, intptr_t value)
{
	//return __sync_lock_test_and_set((intptr_t *)dest, value);
	(void)value;
	return *(intptr_t *)dest;
	/*
	intptr_t oldval = *(intptr_t *)dest;
	intptr_t lastval;
	
	do {
		lastval = oldval;
		oldval = __sync_val_compare_and_swap((intptr_t *)dest, value);
	} while(lastval != oldval);
	
	return oldval;
	*/
}


void sync_signal_init(sync_signal_t * signal, bool value)
{
	*signal = value;
}


void sync_signal_destroy(sync_signal_t * signal)
{
	(void)signal;
	// no-op
}


bool sync_signal_set(sync_signal_t * signal)
{
	return sync_atomic_swap(signal, 1);
}


bool sync_signal_reset(sync_signal_t * signal)
{
	return sync_atomic_swap(signal, 0);
}


bool sync_signal_check(sync_signal_t * signal)
{
	return *signal;
}


void sync_mutex_init(sync_mutex_t * mutex)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_mutex_init(mutex, NULL);
#else
    (void)mutex;
#endif
}


void sync_mutex_destroy(sync_mutex_t * mutex)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_mutex_destroy(mutex);
#else
    (void)mutex;
#endif
}


void sync_mutex_lock(sync_mutex_t * mutex)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_mutex_lock(mutex);
#else
    (void)mutex;
#endif
}


bool sync_mutex_trylock(sync_mutex_t * mutex)
{
#if defined(SYNC_USE_PTHREAD)
	return pthread_mutex_trylock(mutex) == 0;
#else
    (void)mutex;
    return false;
#endif
}


void sync_mutex_unlock(sync_mutex_t * mutex)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_mutex_unlock(mutex);
#else
    (void)mutex;
#endif
}


void sync_rwlock_init(sync_rwlock_t * rwlock)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_rwlock_init(rwlock, NULL);
#else
    (void)rwlock;
#endif
}


void sync_rwlock_destroy(sync_rwlock_t * rwlock)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_rwlock_destroy(rwlock);
#else
    (void)rwlock;
#endif
}


void sync_rwlock_rdlock(sync_rwlock_t * rwlock)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_rwlock_rdlock(rwlock);
#else
    (void)rwlock;
#endif
}


void sync_rwlock_wrlock(sync_rwlock_t * rwlock)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_rwlock_wrlock(rwlock);
#else
    (void)rwlock;
#endif
}


bool sync_rwlock_tryrdlock(sync_rwlock_t * rwlock)
{
#if defined(SYNC_USE_PTHREAD)
	return pthread_rwlock_tryrdlock(rwlock) == 0;
#else
    (void)rwlock;
    return false;
#endif
}


bool sync_rwlock_trywrlock(sync_rwlock_t * rwlock)
{
#if defined(SYNC_USE_PTHREAD)
	return pthread_rwlock_trywrlock(rwlock) == 0;
#else
    (void)rwlock;
    return false;
#endif
}


void sync_rwlock_unlock(sync_rwlock_t * rwlock)
{
#if defined(SYNC_USE_PTHREAD)
	pthread_rwlock_unlock(rwlock);
#else
    (void)rwlock;
#endif
}



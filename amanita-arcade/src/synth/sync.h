#ifndef SYNC_H
#define SYNC_H


#include "synth_core.h"

#if defined(SYNC_USE_PTHREAD)
#include <pthread.h>
#endif


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


typedef intptr_t sync_signal_t;
#if defined(SYNC_USE_PTHREAD)
typedef pthread_mutex_t sync_mutex_t;
typedef pthread_rwlock_t sync_rwlock_t;
#else
typedef struct {int i;} sync_mutex_t;
typedef struct {int i;} sync_rwlock_t;
#endif

static inline void sync_memory_barrier(void)
{
    __sync_synchronize();
}

extern void sync_usleep(int32_t useconds);

extern intptr_t sync_atomic_swap(void volatile * dest, intptr_t value);

// set and reset return the previous value; this is so you can tell whether
// the other cpu has modified the value since you last touched it (this
// operation is atomic -- check() then init() would leave a window where the
// other CPU modifies the signal and you don't notice)
extern void sync_signal_init(sync_signal_t * signal, bool value);
extern void sync_signal_destroy(sync_signal_t * signal);
extern bool sync_signal_set(sync_signal_t * signal);
extern bool sync_signal_reset(sync_signal_t * signal);
extern bool sync_signal_check(sync_signal_t * signal);

extern void sync_mutex_init(sync_mutex_t * mutex);
extern void sync_mutex_destroy(sync_mutex_t * mutex);
extern void sync_mutex_lock(sync_mutex_t * mutex);
extern bool sync_mutex_trylock(sync_mutex_t * mutex);
extern void sync_mutex_unlock(sync_mutex_t * mutex);

extern void sync_rwlock_init(sync_rwlock_t * rwlock);
extern void sync_rwlock_destroy(sync_rwlock_t * rwlock);
extern void sync_rwlock_rdlock(sync_rwlock_t * rwlock);
extern void sync_rwlock_wrlock(sync_rwlock_t * rwlock);
extern bool sync_rwlock_tryrdlock(sync_rwlock_t * rwlock);
extern bool sync_rwlock_trywrlock(sync_rwlock_t * rwlock);
extern void sync_rwlock_unlock(sync_rwlock_t * rwlock);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // SYNC_H



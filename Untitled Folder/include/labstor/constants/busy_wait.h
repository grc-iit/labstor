//
// Created by lukemartinlogan on 12/12/21.
//

#ifndef LABSTOR_BUSY_WAIT_H
#define LABSTOR_BUSY_WAIT_H

#include <labstor/constants/macros.h>

/*Conversions*/
#define LABSTOR_NS_TO_MS(ns) (ns/1000000)

/*TIMER*/
#if defined(__cplusplus)
#include <labstor/userspace/util/timer.h>
#define LABSTOR_TIMER_PREAMBLE()\
    labstor::HighResMonotonicTimer labstor_timer;\
    int labstor_timer_out;
#define LABSTOR_TIMER_START()\
    labstor_timer_out = 0;\
    labstor_timer.Resume();
#define LABSTOR_TIMER_RESET()\
    labstor_timer_out = 0;\
    labstor_timer.Reset();
#define LABSTOR_TIMER_PAUSE(max_ms)\
    labstor_timer.Pause();\
    if(labstor_timer.GetMsec() > max_ms) { labstor_timer_out = 1; }
#define LABSTOR_TIMER_TIMED_OUT()\
    labstor_timer_out
#elif defined(KERNEL_BUILD)
#include <linux/time.h>
#define LABSTOR_TIMER_PREAMBLE()\
    ktime_t timer_start, timer_end; int labstor_timer_out;
#define LABSTOR_TIMER_START() \
    labstor_timer_out = 0;\
    timer_start = ktime_get_ns();
#define LABSTOR_TIMER_RESET()\
    LABSTOR_TIMER_START()
#define LABSTOR_TIMER_PAUSE(max_ms)\
    timer_end = ktime_get_ns();\
    if(LABSTOR_NS_TO_MS(timer_end - timer_start) > max_ms) { labstor_timer_out = 1; }
#define LABSTOR_TIMER_TIMED_OUT()\
    labstor_timer_out
#endif

/*BUSY WAIT*/

#define MAX_YIELDS 1000
#define SPINS_PER_USEC 100000
#define MAX_SPINS (SPINS_PER_USEC * 1000)

#define LABSTOR_SIMPLE_SPINWAIT_PREAMBLE()\
    int simple_spinwait_yields, simple_spinwait_spins;
#define LABSTOR_SIMPLE_SPINWAIT_START() \
    for(simple_spinwait_yields = 0; simple_spinwait_yields < MAX_YIELDS; ++simple_spinwait_yields) {\
        for(simple_spinwait_spins = 0; simple_spinwait_spins < MAX_SPINS; ++simple_spinwait_spins) {
#define LABSTOR_SIMPLE_SPINWAIT_END() } LABSTOR_YIELD(); }

/*INFINITE SPINWAIT*/

#ifdef LABSTOR_YIELD_SPINWAIT
#define LABSTOR_INF_SPINWAIT_PREAMBLE()\
    LABSTOR_SIMPLE_SPINWAIT_PREAMBLE()
#define LABSTOR_INF_SPINWAIT_START() \
    while(1) {  LABSTOR_SIMPLE_SPINWAIT_START()
#define LABSTOR_INF_SPINWAIT_END()\
    LABSTOR_SIMPLE_SPINWAIT_END() }
#else
#define LABSTOR_INF_SPINWAIT_PREAMBLE()
#define LABSTOR_INF_SPINWAIT_START() \
    while(1) {
#define LABSTOR_INF_SPINWAIT_END()\
    }
#endif

/*TIMED SPINWAIT*/

#define LABSTOR_TIMED_SPINWAIT_PREAMBLE()\
    LABSTOR_SIMPLE_SPINWAIT_PREAMBLE()\
    LABSTOR_TIMER_PREAMBLE()\
    bool start_timer=true;

#define LABSTOR_TIMED_SPINWAIT_START(max_ms) \
   do {                                      \
        LABSTOR_SIMPLE_SPINWAIT_START()
    
#define LABSTOR_TIMED_SPINWAIT_END(max_ms)\
        LABSTOR_SIMPLE_SPINWAIT_END()   \
        if(start_timer) {LABSTOR_TIMER_START(); start_timer = false; }\
        else { LABSTOR_TIMER_PAUSE(max_ms); }\
    } while(!LABSTOR_TIMER_TIMED_OUT());
#define LABSTOR_SPINWAIT_TIMED_OUT() (LABSTOR_TIMER_TIMED_OUT())

/*RELEASE ARBITRARY SPINLOCK*/
#define LABSTOR_SPINLOCK_RELEASE(lockptr) \
    *lockptr = 0;

/*INFINITE SPINLOCK*/

#ifdef LABSTOR_YIELD_SPINWAIT
#define LABSTOR_INF_LOCK_PREAMBLE()\
    LABSTOR_INF_SPINWAIT_PREAMBLE()
static inline int LABSTOR_INF_LOCK_TRYLOCK(uint16_t *lockptr) {
    uint16_t unlocked = 0, is_locked = 1;
    return __atomic_compare_exchange_n(lockptr, &unlocked, is_locked, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}
#define LABSTOR_INF_LOCK_ACQUIRE(lockptr)\
    LABSTOR_SIMPLE_SPINWAIT_START()\
    if(LABSTOR_INF_LOCK_TRYLOCK(lockptr)) { break; }\
    LABSTOR_SIMPLE_SPINWAIT_END()
#define LABSTOR_INF_LOCK_RELEASE(lockptr) \
    LABSTOR_SPINLOCK_RELEASE(lockptr);
#else
#define LABSTOR_INF_LOCK_PREAMBLE()
static inline int LABSTOR_INF_LOCK_TRYLOCK(uint16_t *lockptr) {
    uint16_t unlocked = 0, is_locked = 1;
    return __atomic_compare_exchange_n(lockptr, &unlocked, is_locked, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}
#define LABSTOR_INF_LOCK_ACQUIRE(lockptr) \
    TRACEPOINT("INF SPINLOCK")                                      \
    while(1) { if(LABSTOR_INF_LOCK_TRYLOCK(lockptr)) { break; } }
#define LABSTOR_INF_LOCK_RELEASE(lockptr) \
    LABSTOR_SPINLOCK_RELEASE(lockptr);
#endif

/*TIMED SPINLOCK*/

#define LABSTOR_TIMED_LOCK_PREAMBLE()\
    LABSTOR_TIMED_SPINWAIT_PREAMBLE()
static inline int LABSTOR_TIMED_LOCK_TRYLOCK(uint16_t *lockptr) {
    uint16_t unlocked = 0, is_locked = 1;
    return __atomic_compare_exchange_n(lockptr, &unlocked, is_locked, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}
#define LABSTOR_TIMED_LOCK_ACQUIRE(lockptr, max_ms)\
    LABSTOR_TIMED_SPINWAIT_START(max_ms)\
    if(LABSTOR_TIMED_LOCK_TRYLOCK(lockptr)) { break; }\
    LABSTOR_TIMED_SPINWAIT_END(max_ms)
#define LABSTOR_TIMED_LOCK_RELEASE(lockptr) \
    LABSTOR_SPINLOCK_RELEASE(lockptr);
#define LABSTOR_LOCK_TIMED_OUT() (LABSTOR_TIMER_TIMED_OUT())

#endif //LABSTOR_BUSY_WAIT_H

//
// Created by lukemartinlogan on 11/12/21.
//

#ifndef LABSTOR_ATOMIC_BUSY_H
#define LABSTOR_ATOMIC_BUSY_H

#define ATOMIC_PLEASE_PAUSE 0x80000000

#ifdef __cplusplus

namespace labstor {

struct AtomicBusy {
    uint32_t flag_ref_;

    AtomicBusy() { Init(); }
    inline void Init() {
        flag_ref_ = 0;
    }
    inline void MarkPaused() {
        uint32_t flag_ref, new_flags;
        do {
            flag_ref = flag_ref_;
            new_flags = flag_ref | ATOMIC_PLEASE_PAUSE;
        } while (!__atomic_compare_exchange_n(&flag_ref_, &flag_ref, new_flags, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    }
    inline bool IsPaused() {
        uint32_t new_flags = ATOMIC_PLEASE_PAUSE;
        return __atomic_compare_exchange_n(&flag_ref_, &new_flags, new_flags, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    }
    inline void UnPause() {
        flag_ref_ &= ~ATOMIC_PLEASE_PAUSE;
    }
    inline void PleaseWork() {
        uint32_t flag_ref = 0, new_flags;
        do {
            flag_ref = flag_ref_;
            if (flag_ref & ATOMIC_PLEASE_PAUSE) { continue; }
            new_flags = flag_ref + 1;
        } while (!__atomic_compare_exchange_n(&flag_ref_, &flag_ref, new_flags, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    }
    inline void FinishWork() {
        uint32_t flag_ref, new_flags;
        do {
            flag_ref = flag_ref_;
            new_flags = flag_ref - 1;
        } while (__atomic_compare_exchange_n(&flag_ref_, &flag_ref, new_flags, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    }
};

}

#endif

#endif //LABSTOR_ATOMIC_BUSY_H

//
// Created by lukemartinlogan on 11/12/21.
//

#ifndef LABSTOR_SHMEM_ATOMIC_BUSY_H
#define LABSTOR_SHMEM_ATOMIC_BUSY_H

#define ATOMIC_PLEASE_PAUSE 0x80000000

struct labstor_atomic_busy {
    uint32_t *flag_ref_;
};

static inline void labstor_atomic_busy_Init(struct labstor_atomic_busy *flags, uint32_t *ptr) {
    flags->flag_ref_ = ptr;
    *flags->flag_ref_ = 0;
}
static inline void labstor_atomic_busy_Attach(struct labstor_atomic_busy *flags, uint32_t *ptr) {
    flags->flag_ref_ = ptr;
}
static inline uint32_t labstor_atomic_busy_GetFlags(struct labstor_atomic_busy *flags) {
    return *flags->flag_ref_;
}
static inline void labstor_atomic_busy_MarkPaused(struct labstor_atomic_busy *flags) {
    uint32_t flag_ref, new_flags;
    do {
        flag_ref = *flags->flag_ref_;
        new_flags = flag_ref | ATOMIC_PLEASE_PAUSE;
    } while (!__atomic_compare_exchange_n(flags->flag_ref_, &flag_ref, new_flags, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}
static inline bool labstor_atomic_busy_IsPaused(struct labstor_atomic_busy *flags) {
    uint32_t new_flags = ATOMIC_PLEASE_PAUSE;
    return __atomic_compare_exchange_n(flags->flag_ref_, &new_flags, new_flags, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}
static inline void labstor_atomic_busy_UnPause(struct labstor_atomic_busy *flags) {
    (*flags->flag_ref_) &= ~ATOMIC_PLEASE_PAUSE;
}
static inline void labstor_atomic_busy_PleaseWork(struct labstor_atomic_busy *flags) {
    uint32_t flag_ref = 0, new_flags;
    do {
        flag_ref = *flags->flag_ref_;
        if (flag_ref & ATOMIC_PLEASE_PAUSE) { continue; }
        new_flags = flag_ref + 1;
    } while (!__atomic_compare_exchange_n(flags->flag_ref_, &flag_ref, new_flags, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}
static inline void labstor_atomic_busy_FinishWork(struct labstor_atomic_busy *flags) {
    uint32_t flag_ref, new_flags;
    do {
        flag_ref = *flags->flag_ref_;
        new_flags = flag_ref - 1;
    } while (__atomic_compare_exchange_n(flags->flag_ref_, &flag_ref, new_flags, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}

#ifdef __cplusplus

namespace labstor {

struct atomic_busy : public labstor_atomic_busy {
    inline void Init(uint32_t *ptr) {
        labstor_atomic_busy_Init(this, ptr);
    }
    inline void MarkPaused() {
        labstor_atomic_busy_MarkPaused(this);
    }
    inline bool IsPaused() {
        return labstor_atomic_busy_IsPaused(this);
    }
    inline void UnPause() {
        labstor_atomic_busy_UnPause(this);
    }
    inline void PleaseWork() {
        labstor_atomic_busy_PleaseWork(this);
    }
    inline void FinishWork() {
        labstor_atomic_busy_FinishWork(this);
    }
};

}

#endif

#endif //LABSTOR_SHMEM_ATOMIC_BUSY_H

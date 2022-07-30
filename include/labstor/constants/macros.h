
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LABSTOR_MACROS_H
#define LABSTOR_MACROS_H

#include <labstor/types/basics.h>
#include <labstor/constants/debug.h>
#include <labstor/userspace/util/errors.h>

/*KERNEL_PID*/
#define KERNEL_PID 0

/*LABSTOR_REGION_ADD AND LABSTOR_REGION_SUB*/
#ifdef LABSTOR_MEM_DEBUG
static inline labstor::off_t LABSTOR_REGION_SUB(void *ptr, void *region) {
    if ((size_t) ptr >= (size_t) region) {
        return (size_t) ptr - (size_t) region;
    } else {
#ifdef __cplusplus
        throw labstor::INVALID_REGION_SUB.format((size_t)ptr, (size_t)region);
#elif KERNEL_BUILD
        pr_err("LABSTOR_REGION_SUB, ptr should never be smaller than region!");
        return (size_t) ptr - (size_t) region;
#endif
    }
}
#else
#define LABSTOR_REGION_SUB(ptr,region) (labstor_off_t)((size_t) ptr - (size_t) region)
#endif
#define LABSTOR_REGION_ADD(off, region) (void*)((char*)region + off)
#define LABSTOR_PTR_DIFF(region1,region2) (region1 >= region2 ? (size_t)region1 - (size_t)region2 : (int64_t)((size_t)region1 - (size_t)region2))

/*YIELD*/
#ifdef KERNEL_BUILD
#include <linux/sched.h>
#define LABSTOR_YIELD() yield()
#elif __cplusplus
#include <sched.h>
#define LABSTOR_YIELD() sched_yield()
#endif

/*SINGLETON*/
#ifdef __cplusplus
#define DEFINE_SINGLETON(NAME) template<> LABSTOR_##NAME##_T LABSTOR_##NAME##_SINGLETON::obj_ = nullptr;
#endif

#endif //LABSTOR_MACROS_H
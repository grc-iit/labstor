
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

#ifndef LABSTOR_UNORDERED_MAP_CONSTANTS_H
#define LABSTOR_UNORDERED_MAP_CONSTANTS_H

#include "labstor/types/basics.h"

static const labstor_off_t null1_mark = 1 << (sizeof(labstor_off_t)*8 - 1);
static const labstor_off_t null1_null = ~((labstor_off_t)0) ^ null1_mark;

static const labstor_off_t null0_mark = 1 << (sizeof(labstor_off_t)*8 - 1);
static const labstor_off_t null0_null = 0;


#endif //LABSTOR_UNORDERED_MAP_CONSTANTS_H
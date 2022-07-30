
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

#include "labstor/constants/debug.h"
#include "labmods/registrar/registrar.h"
#include "no_op_client.h"

void labstor::iosched::NoOp::Client::Register(YAML::Node config) {
    AUTO_TRACE("")
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(NO_OP_IOSCHED_MODULE_ID, config["labmod_uuid"].as<std::string>());
    LABSTOR_REGISTRAR->InitializeInstance<register_request>(ns_id_, config["next"].as<std::string>());
}

labstor::ipc::qtok_t labstor::iosched::NoOp::Client::AIO(void *buf, size_t size, size_t off, labstor::GenericBlock::Ops op) {
    return labstor::ipc::qtok_t();
}

LABSTOR_MODULE_CONSTRUCT(labstor::iosched::NoOp::Client, NO_OP_IOSCHED_MODULE_ID);
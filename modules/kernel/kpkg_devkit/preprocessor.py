import os

class LabStorDataStructures:
    def __init__(self):
        with open("template/data_structures/labstor_array.h") as fp:
            self.labstor_array = fp.read()
        with open("template/data_structures/labstor_request_queue.h") as fp:
            self.labstor_request_queue = fp.read()
        with open("template/data_structures/labstor_unordered_map.h") as fp:
            self.labstor_unordered_map = fp.read()
        with open("template/data_structures/labstor_request_map.h") as fp:
            self.labstor_request_map = fp.read()
        with open("template/data_structures/labstor_work_queue.h") as fp:
            self.labstor_work_queue = fp.read()

    def load_templates(self):
        return

    def create_array(self, SUFFIX, T):
        return

    def create_ring_buffer(self, SUFFIX, T):
        return

    def create_unordered_map(self, SUFFIX, ARR_SUFFIX, BUCKET_T, BUCKET_HASH, S, S_Atomic, T, GetAtomicKey, GetAtomicKeyRef, GetKey, NullKey, KeyCompare):
        return

    def create_request_map(self):
        umap = self.create_unordered_map("request",
                                         "uint32_t",
                                         "labstor_off_t",
                                         "labstor_off_t",
                                         "labstor_request_map_bucket_GetAtomicKey",
                                         "labstor_request_map_bucket_GetAtomicKeyRef",
                                         "GetKey",
                                         "NullKey",
                                         "KeyCompare")


    def create_request_queue(self):
        return

    def create_work_queue(self):
        return

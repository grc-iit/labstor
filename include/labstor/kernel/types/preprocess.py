import os

class LabStorDataStructures:
    def __init__(self):
        with open("../../types/data_structures/array/shmem_array.h") as fp:
            self.array =  fp.read()
        with open("../../types/data_structures/ring_buffer/shmem_ring_buffer.h") as fp:
            self.rbuf = fp.read()
        with open("../../types/data_structures/unordered_map/shmem_unordered_map.h") as fp:
            self.unordered_map = fp.read()

    def load_templates(self):
        return

    def replace(self, str, **args):
        for key,value in args.items():
            str = str.replace(f"{{{key}}}", value)
        return str

    def create_array(self, SUFFIX, T):
        output = f"data_structures/shmem_array_{SUFFIX}.h"
        text = self.replace(
            self.array,
            SUFFIX=SUFFIX,
            T=T
        )
        with open(output, 'w') as fp:
            fp.write(text)

    def create_ring_buffer(self, SUFFIX, T):
        output = f"data_structures/shmem_ring_buffer_{SUFFIX}.h"
        text = self.replace(
            self.rbuf,
            SUFFIX=SUFFIX,
            T=T
        )
        with open(output, 'w') as fp:
            fp.write(text)

    def create_unordered_map(self, BUCKET_PREFIX, ARR_SUFFIX, SUFFIX, S, S_Atomic, T):
        output = f"data_structures/shmem_unordered_map_{SUFFIX}.h"
        text = self.replace(self.unordered_map,
            SUFFIX=SUFFIX,
            ARR_SUFFIX=ARR_SUFFIX,
            S=S,
            S_Atomic=S_Atomic,
            T=T,
            BUCKET_T=f"struct {BUCKET_PREFIX}",
            BUCKET_HASH=f"{BUCKET_PREFIX}_hash",
            GetAtomicKey=f"{BUCKET_PREFIX}_GetAtomicKey",
            GetAtomicKeyRef=f"{BUCKET_PREFIX}_GetAtomicKeyRef",
            GetKey=f"{BUCKET_PREFIX}_GetKey",
            GetValue=f"{BUCKET_PREFIX}_GetValue",
            NullKey=f"{BUCKET_PREFIX}_NullKey",
            KeyCompare=f"{BUCKET_PREFIX}_KeyCompare",
            GetMarkedAtomicKey=f"{BUCKET_PREFIX}_GetMarkedAtomicKey")
        with open(output, 'w') as fp:
            fp.write(text)

    def create_array_off_t(self):
        self.create_array("off_t", "labstor_off_t")

    def create_ring_buffer_off_t(self):
        self.create_ring_buffer("off_t", "labstor_off_t")

    def create_ring_buffer_qp_ptr(self):
        self.create_ring_buffer("qp_ptr", "struct labstor_queue_pair_ptr")

    def create_array_request_bucket(self):
        self.create_array("request_map_bucket", "struct labstor_request_map_bucket")

    def create_request_map(self):
        self.create_unordered_map(
            "labstor_request_map_bucket",
            "request_map_bucket",
            "request",
            "labstor_off_t",
            "labstor_off_t",
            "struct labstor_request*")

    def compile(self):
        self.create_array_off_t()
        self.create_ring_buffer_off_t()
        self.create_ring_buffer_qp_ptr()
        self.create_array_request_bucket()
        self.create_request_map()

LabStorDataStructures().compile()


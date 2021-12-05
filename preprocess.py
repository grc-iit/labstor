import os,sys

class LabStorDataStructures:
    def __init__(self, root):
        self.root = root
        self.data_structures = f"{self.root}/include/labstor/types/data_structures"
        with open(f"{self.data_structures}/array/shmem_array.h") as fp:
            self.array =  fp.read()
        with open(f"{self.data_structures}/ring_buffer/shmem_ring_buffer.h") as fp:
            self.rbuf = fp.read()
        with open(f"{self.data_structures}/unordered_map/shmem_unordered_map.h") as fp:
            self.unordered_map = fp.read()
        with open(f"{self.data_structures}/shmem_int_map.h") as fp:
            self.int_map = fp.read()

    def load_templates(self):
        return

    def replace(self, str, **args):
        for key,value in args.items():
            str = str.replace(f"{{{key}}}", value)
        return str

    def _create_array(self, T, T_NAME=None):
        if T_NAME is None:
            T_NAME = T
        output = f"{self.data_structures}/array/shmem_array_{T_NAME}.h"
        text = self.replace(
            self.array,
            T_NAME=T_NAME,
            T=T
        )
        with open(output, 'w') as fp:
            fp.write(text)

    def _create_ring_buffer(self, T, T_NAME=None):
        if T_NAME is None:
            T_NAME = T
        output = f"{self.data_structures}/ring_buffer/shmem_ring_buffer_{T_NAME}.h"
        text = self.replace(
            self.rbuf,
            T_NAME=T_NAME,
            T=T
        )
        with open(output, 'w') as fp:
            fp.write(text)

    def _create_unordered_map(self, S, S_NAME, S_ATOMIC, T, T_NAME, BUCKET_T_NAME):
        BUCKET_T = f"struct {BUCKET_T_NAME}"
        self._create_array(BUCKET_T, BUCKET_T_NAME)
        output = f"{self.data_structures}/unordered_map/shmem_unordered_map_{S_NAME}_{T_NAME}_impl.h"
        text = self.replace(self.unordered_map,
            S=S,
            S_NAME=S_NAME,
            S_ATOMIC=S_ATOMIC,
            T=T,
            T_NAME=T_NAME,
            BUCKET_T=BUCKET_T,
            BUCKET_T_NAME=BUCKET_T_NAME,
            KeyHash=f"{BUCKET_T_NAME}_hash",
            GetAtomicKey=f"{BUCKET_T_NAME}_GetAtomicKey",
            GetAtomicKeyRef=f"{BUCKET_T_NAME}_GetAtomicKeyRef",
            GetKey=f"{BUCKET_T_NAME}_GetKey",
            GetValue=f"{BUCKET_T_NAME}_GetValue",
            NullKey=f"{BUCKET_T_NAME}_NullKey",
            KeyCompare=f"{BUCKET_T_NAME}_KeyCompare",
            GetMarkedAtomicKey=f"{BUCKET_T_NAME}_GetMarkedAtomicKey")
        with open(output, 'w') as fp:
            fp.write(text)

    def _create_int_map(self, S, T, T_NAME):
        BUCKET_T_NAME = f"labstor_{S}_{T_NAME}_bucket"
        S_NAME = S
        S_ATOMIC = S
        self._create_unordered_map(S, S_NAME, S_ATOMIC, T, T_NAME, BUCKET_T_NAME)
        output=f"{self.data_structures}/shmem_unordered_map_{S_NAME}_{T_NAME}.h"
        text = self.replace(self.int_map,
            S=S,
            T=T,
            T_NAME=T_NAME,
        )
        with open(output, 'w') as fp:
            fp.write(text)

    def create_array_labstor_off_t(self):
        self._create_array("labstor_off_t")

    def create_array_labstor_qtok_t(self):
        self._create_array("labstor_qtok_t")

    def create_ring_buffer_labstor_off_t(self):
        self._create_ring_buffer("labstor_off_t")

    def create_request_queue(self):
        self._create_ring_buffer("struct labstor_queue_pair_ptr", "qp_ptr")

    def create_request_map(self):
        self._create_unordered_map(
            S="uint32_t",
            S_NAME="uint32_t",
            S_ATOMIC="uint32_t",
            T="struct labstor_request*",
            T_NAME="request",
            BUCKET_T_NAME="labstor_request_map_bucket")

    def create_string_map(self):
        self._create_unordered_map(
            S="labstor::ipc::string",
            S_NAME="labstor_string",
            S_ATOMIC="labstor_off_t",
            T="uint32_t",
            T_NAME="uint32_t",
            BUCKET_T_NAME="labstor_string_map_bucket")

    def create_pid_to_ipc_map(self):
        self._create_int_map(
            S="int",
            T="labstor::Server::PerProcessIPC*",
            T_NAME="PerProcessIPC"
        )

    def create_id_to_qp_map(self):
        self._create_int_map(
            S="labstor_qid_t",
            T="labstor::ipc::queue_pair*",
            T_NAME="qp"
        )

    def create_array_uint32_t(self):
        self._create_array("uint32_t")

    def create_ring_buffer_uint32_t(self):
        self._create_ring_buffer("uint32_t")

    def create_int_map_uint32_t(self):
        self._create_int_map(
            S="uint32_t",
            T="uint32_t",
            T_NAME="uint32_t"
        )

    def compile(self):
        self.create_array_labstor_off_t()
        self.create_array_labstor_qtok_t()
        self.create_ring_buffer_labstor_off_t()
        self.create_request_queue()
        self.create_request_map()
        self.create_string_map()
        self.create_pid_to_ipc_map()
        self.create_id_to_qp_map()
        self.create_array_uint32_t()
        self.create_ring_buffer_uint32_t()
        self.create_int_map_uint32_t()

print("PREPROCESSING!")
CMAKE_SOURCE_DIR=sys.argv[1]
LabStorDataStructures(CMAKE_SOURCE_DIR).compile()


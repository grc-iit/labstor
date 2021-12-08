from scsbench.runtime.bash_runtime_chain import BashRuntimeChain
import yaml

from

KB=1024
OUTPUT_DIR = "/home/cc/tests"
N_SERVER_CORES = [0, 1, 2, 4, 8, 16]
FRAC_KERNEL_CORES = [.25, .5, .75]
DEDICATED_CORES = [0,1]
N_CLIENT_PROCS = [1, 2, 4, 8, 16]
IO_SIZE = [4*KB, 8*KB, 16*KB, 32*KB, 64*KB, 128*KB]
IO_TYPE = ["seq", "rand"]

for n_server_cores in N_SERVER_CORES:
    for frac_kernel_cores in FRAC_KERNEL_CORES:
        for dedicated_cores in DEDICATED_CORES:
            for n_client_procs in N_CLIENT_PROCS:
                for io_size in IO_SIZE:
                    for io_type in IO_TYPE:
                        chain = [
                            "create config file",
                            "create kernel"
                            "${LABSTOR_BUILD_DIR}/start_kernel_server",
                            "start "
                        ]
                        #Create config file

                        #Run experiment
                        labstor_server = BashRuntimeChain(chain, do_stdout=True, output_dir=OUTPUT_DIR/)
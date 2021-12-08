from scsbench.runtime.bash_runtime import BashRuntime
from scsbench.runtime.bash_runtime_chain import BashRuntimeChain
from .config import LabstorConfigurator
import os,sys
import pandas as pd

CONFIG = "/home/cc/labstor/config/config.yaml"
OUTPUT_DIR = "/home/cc/tests"
#N_SERVER_CORES = [0, 1, 2, 4, 8, 16]
#FRAC_KERNEL_CORES = [.25, .5, .75]
#DEDICATED_CORES = [0,1]
#N_CLIENT_PROCS = [1, 2, 4, 8, 16]
#REQS_PER_CLIENT = 2048

N_SERVER_CORES = [4]
FRAC_KERNEL_CORES = [.5]
DEDICATED_CORES = [1]
N_CLIENT_PROCS = [4]
REQS_PER_CLIENT = 2048

os.environ["LABSTOR_ROOT"] = "/home/cc/labstor"
os.environ["LABSTOR_CONF"] = "/home/cc/labstor/config"
os.environ["LABSTOR_UTIL"] = "/home/cc/labstor/util"
os.environ["LABSTOR_BUILD_DIR"] = "/home/cc/labstor/cmake-build-release-chameleon"

def RunTests(path):
    #Load labstor configurator
    conf = LabstorConfigurator()
    conf.Load(f'{os.environ["LABSTOR_CONF"]}/config_template.yaml')

    #Run experiments
    columns = ["n_server_cores", "server_core_mult", "n_kernel_cores", "dedicated", "n_clients", "n_msgs", "time_us", "thrpt_mops"]
    results = []
    for n_server_cores in N_SERVER_CORES:
        for frac_kernel_cores in FRAC_KERNEL_CORES:
            for dedicated_cores in DEDICATED_CORES:
                for n_client_procs in N_CLIENT_PROCS:
                    #Create LabStor config
                    conf.SetFractionalWorkers(10, 4, .5)
                    conf.SaveTemp()
                    #Start kernel server
                    kern_server = BashRuntime(f'"bash {os.environ["LABSTOR_UTIL"]}/start_kernel_server.sh"', do_stdout=True).Run()
                    if kern_server.GetExitCode() != 0:
                        kern_server.PrintOutput()
                        break
                    #Start LabStor server
                    labstor_server = BashRuntime(f'bash {os.environ["LABSTOR_UTIL"]}/start_kernel_server.sh', do_stdout=True).RunAsync()
                    if labstor_server.GetExitCode() != 0:
                        labstor_server.PrintOutput()
                        break
                    #Partition the cores
                    test_app = BashRuntime(
                        f'{os.environ["LABSTOR_BUILD_DIR"]}/proc_partition {labstor_server.GetPid()} {conf.GetServerAffinity()}',
                        affinity=conf.GetAffinity())
                    #Start application
                    test_app = BashRuntime(
                        f'{os.environ["LABSTOR_BUILD_DIR"]}/test/performance/test_server_thrpt /tmp/labstor_conf.yaml',
                        affinity=conf.GetAffinity())
                    if test_app.GetExitCode() != 0:
                        test_app.PrintOutput()
                        break
                    #Kill LabStor server
                    labstor_server.Kill()
                    #Get application output
                    results.append(test_app.GetOutput().splitlines()[-1])

    df = pd.DataFrame(results,columns=columns)
    df.to_csv(path)

RunTests("/home/cc/test/server_scalability.csv")
from scsbench.runtime.bash_runtime import BashRuntime
from scsbench.runtime.bash_runtime_chain import BashRuntimeChain
from scsbench.test_tracker.test_tracker import TestTracker
from config import LabStorConfigurator
from commands import *
import os,sys
import pandas as pd

#Run tests
class ServerScalabilityTest(TestTracker):
    def TrialInit(self):
        self.kernel_server = None
        self.labstor_server = None
        LabStorKernelServer().Kill()

    def Trial(self, n_server_cores, oversubscribe, frac_kernel_cores, dedicated_cores, n_client_procs, n_msgs, labstor_conf):
        #Create LabStor config
        labstor_conf.SetFractionalWorkers(n_server_cores, oversubscribe, frac_kernel_cores)
        labstor_conf.SaveTemp()

        #Start kernel server
        print("Starting kernel server")
        self.kernel_server = LabStorKernelServer().Start()

        #Start LabStor server
        print("Starting LabStor server")
        self.labstor_server = LabStorTrustedServer(conf=labstor_conf.GetTemp()).RunAsync()
        print(f"PID: {self.labstor_server.GetPid()}")
        print(f"Status: {self.labstor_server.GetExitCode()}")

        #Partition the cores
        print("Dedicating cores")
        if dedicated_cores:
            PartitionCores(self.labstor_server.GetPid(), labstor_conf.GetBothServerAffinitiesCmdline()).Run()

        #Start application
        print("Starting application")
        test_app = BashRuntime(
            f'{os.environ["LABSTOR_BUILD_DIR"]}/test/performance/test_server_thrpt_exec {n_server_cores} {oversubscribe} {frac_kernel_cores} {dedicated_cores} {n_client_procs} {n_msgs}',
            affinity=labstor_conf.GetApplicationAffinity(),
            max_retry_ms=5000).Run()
        if test_app.GetExitCode() != 0:
            test_app.PrintOutput()
            raise Exception(f'Error code {test_app.GetExitCode()}')

        #Store application output
        app_output = test_app.GetOutput()
        if app_output:
            return test_app.GetOutput().splitlines()[-1]
        return ''

    def TrialEnd(self):
        if self.labstor_server:
            print("kill labstor")
            self.labstor_server.Kill()
            print(self.labstor_server.GetOutput())
        if self.kernel_server:
            print("Kill kernel")
            self.kernel_server.Kill()
            print(self.kernel_server.GetOutput())

    def SaveResults(self):
        header ="n_server_cores,server_core_mult,n_kernel_cores,dedicated,n_clients,n_msgs,time_us,thrpt_mops\n"
        csv="\n".join([result for result in self.trials.values() if result is not False])
        print(csv)
        with open(os.path.join(self.test_dir, "results.csv"), 'w') as fp:
            fp.write(header)
            fp.write(csv)

#Set environment variables
os.environ["LABSTOR_ROOT"] = f'{os.environ["HOME"]}/labstor'
os.environ["LABSTOR_CONF"] = f'{os.environ["LABSTOR_ROOT"]}/config'
os.environ["LABSTOR_UTIL"] = f'{os.environ["LABSTOR_ROOT"]}/util'
#os.environ["LABSTOR_BUILD_DIR"] = f'{os.environ["HOME"]}/cmake-build-release-chameleon'
os.environ["LABSTOR_BUILD_DIR"] = f'{os.environ["LABSTOR_ROOT"]}/cmake-build-release-virtalbox'

#Set testing parameters
BIG_TEST=True
if BIG_TEST:
    N_SERVER_CORES = [0, 1, 2, 4, 8, 16]
    FRAC_KERNEL_CORES = [.25, .5, .75]
    DEDICATED_CORES = [0,1]
    N_CLIENT_PROCS = [1, 2, 4, 8, 16]
    REQS_PER_CLIENT = 2048
else:
    N_SERVER_CORES = [2]
    OVERSUBSCRIBE=[1]
    FRAC_KERNEL_CORES = [.5]
    DEDICATED_CORES = [1]
    OVERSUBSCRIBE=[1]
    N_CLIENT_PROCS = [4]
    REQS_PER_CLIENT = [128]

#Load labstor configurator
labstor_conf = LabStorConfigurator()
labstor_conf.Load(f'{os.environ["LABSTOR_CONF"]}/config_template.yaml')

#Run experiments
TEST_OUTPUT_DIR = f'{os.environ["HOME"]}/tests/server_scalability'
tests = ServerScalabilityTest(TEST_OUTPUT_DIR, 10)
tests.SetVariables(N_SERVER_CORES, OVERSUBSCRIBE, FRAC_KERNEL_CORES, DEDICATED_CORES, N_CLIENT_PROCS, REQS_PER_CLIENT)
tests.SetInputs(labstor_conf)
tests.Run()
tests.SaveResults()
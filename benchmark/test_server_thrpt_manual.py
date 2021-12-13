from scsbench.runtime.bash_runtime import BashRuntime
from scsbench.test_tracker.test_tracker import TestTracker
from config import LabStorConfigurator
from commands import *
import os,sys

#Run tests
class ServerScalabilityTest(TestTracker):

    def ExperimentInit(self):
        return

    def ExperimentEnd(self):
        return

    def TrialInit(self):
        return

    def Trial(self, n_server_cores, oversubscribe, frac_kernel_cores, dedicated_cores, n_client_procs, n_msgs, labstor_conf):
        print(f"Trial Start: n_server_cores={n_server_cores}, oversub={oversubscribe}, kfrac={frac_kernel_cores}, dedicate={dedicated_cores}, clients={n_client_procs}, msgs={n_msgs}")
        #Create LabStor config
        labstor_conf.SetFractionalWorkers(n_server_cores, oversubscribe, frac_kernel_cores)
        labstor_conf.SaveTemp()
        #labstor_conf.PrintConfig()

        commands = []

        #Set environment
        commands.append(f"export LABSTOR_BUILD_DIR={os.environ['LABSTOR_BUILD_DIR']}")
        commands.append(f"export LABSTOR_ROOT={os.environ['LABSTOR_ROOT']}")
        #Make sure kernel server is not already running
        commands.append(LabStorKernelServer().GetKillCommand())
        #Start kernel server
        commands.append(LabStorKernelServer().GetStartCommand())
        #Start LabStor server in the background
        commands.append(f"{LabStorTrustedServer(labstor_conf.GetTemp()).GetCommand()} &")
        commands.append(f"PID=$!")
        #Partition cores
        if dedicated_cores:
            commands.append(PartitionCores("$PID", labstor_conf.GetBothServerAffinitiesCmdline()).GetCommand())
        #Start application
        commands.append(TestServerThroughput(
            n_server_cores=n_server_cores,
            oversubscribe=oversubscribe,
            frac_kernel_cores=frac_kernel_cores,
            dedicated_cores=dedicated_cores,
            n_client_procs=n_client_procs,
            n_msgs=n_msgs,
            affinity=labstor_conf.GetApplicationAffinity()).GetCommand())
        #Echo the results into the test file
        commands.append(f"echo >> {os.path.join(self.test_dir, 'results.csv')}")
        #Kill labstor server
        commands.append("kill -9 $PID")
        #kill kernel server
        commands.append(LabStorKernelServer().GetKillCommand())

        #Print the command
        for command in commands:
            print(command)

        #Did test pass?
        result = bool(input("Did test pass? (True/False)"))
        if not result:
            raise Exception("Didn't pass")
        return True


    def TrialEnd(self):
        return

    def SaveResults(self):
        return

#Set environment variables
IS_CHAMELEON=True
os.environ["LABSTOR_ROOT"] = f'{os.environ["HOME"]}/labstor'
os.environ["LABSTOR_CONF"] = f'{os.environ["LABSTOR_ROOT"]}/config'
os.environ["LABSTOR_UTIL"] = f'{os.environ["LABSTOR_ROOT"]}/util'
if IS_CHAMELEON:
    os.environ["LABSTOR_BUILD_DIR"] = f'{os.environ["LABSTOR_ROOT"]}/cmake-build-debug-chameleon'
else:
    os.environ["LABSTOR_BUILD_DIR"] = f'{os.environ["LABSTOR_ROOT"]}/cmake-build-release-virtalbox'

#Set testing parameters
BIG_TEST=True
if BIG_TEST:
    N_SERVER_CORES = [4, 8, 16]
    FRAC_KERNEL_CORES = [.25, .5, .75]
    DEDICATED_CORES = [0,1]
    OVERSUBSCRIBE=[1]
    N_CLIENT_PROCS = [1, 2, 4, 8, 16]
    REQS_PER_CLIENT = [2048]
else:
    N_SERVER_CORES = [4,8,16]
    OVERSUBSCRIBE=[0,1]
    FRAC_KERNEL_CORES = [.25, .5, .75]
    DEDICATED_CORES = [1]
    OVERSUBSCRIBE=[1]
    N_CLIENT_PROCS = [4]
    REQS_PER_CLIENT = [128]

#Load labstor configurator
labstor_conf = LabStorConfigurator()
labstor_conf.Load(f'{os.environ["LABSTOR_CONF"]}/config_template.yaml')

#Run experiments
TEST_OUTPUT_DIR = f'{os.environ["HOME"]}/tests/server_scalability'
tests = ServerScalabilityTest(TEST_OUTPUT_DIR, 1)
tests.SetVariables(N_SERVER_CORES, OVERSUBSCRIBE, FRAC_KERNEL_CORES, DEDICATED_CORES, N_CLIENT_PROCS, REQS_PER_CLIENT)
tests.SetInputs(labstor_conf)
tests.Run()
tests.SaveResults()
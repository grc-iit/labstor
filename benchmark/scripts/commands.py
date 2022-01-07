from scsbench.runtime.bash_runtime import BashRuntime
import os

class DropCaches:
    def Run(self):
        BashRuntime(GetSyncCachesCommand(), view_stdout=True).Run()
        DropCaches()

    def GetSyncCachesCommand(self):
        return f'sync'

    def DropCaches(self):
        with open("/proc/sys/vm/drop_caches", "w") as fp:
            fp.write("3")

class LabStorKernelServer:
    def Start(self):
        command = self.GetStartCommand()
        self.start = BashRuntime(command, view_stdout=True).Run()
        return self.start

    def Kill(self):
        command = self.GetKillCommand()
        self.end = BashRuntime(command, view_stdout=True).Run()

    def GetStartCommand(self):
        return f'bash {os.environ["LABSTOR_UTIL"]}/start_kernel_server.sh'

    def GetKillCommand(self):
        return f'bash {os.environ["LABSTOR_UTIL"]}/stop_kernel_server.sh'

class LabStorTrustedServer(BashRuntime):
    def __init__(self, conf):
        self.conf = conf
        self.command = self.GetCommand()
        super().__init__(self.command, view_stdout=True)

    def GetCommand(self):
        return f'{os.environ["LABSTOR_BUILD_DIR"]}/labstor_trusted_server {self.conf}'

class PartitionCores(BashRuntime):
    def __init__(self, pid, affinity):
        self.pid = pid
        self.affinity = affinity
        command = self.GetCommand()
        super().__init__(command, view_stdout=True)

    def GetCommand(self):
        return f'{os.environ["LABSTOR_BUILD_DIR"]}/proc_partition {self.pid} {self.affinity}'

class TestServerThroughput(BashRuntime):
    def __init__(self, affinity, n_server_cores, oversubscribe, frac_kernel_cores, dedicated_cores, n_client_procs,  n_msgs):
        self.affinity = affinity
        self.affinity = affinity
        self.n_server_cores = n_server_cores
        self.oversubscribe = oversubscribe
        self.frac_kernel_cores = frac_kernel_cores
        self.dedicated_cores = dedicated_cores
        self.n_client_procs = n_client_procs
        self.n_msgs = n_msgs
        command = self.GetCommand()
        super().__init__(command, sleep_period_ms=1000, max_retries=5, view_stdout=True)

    def GetCommand(self):
        return f'{os.environ["LABSTOR_BUILD_DIR"]}/test/performance/test_server_thrpt_exec {self.n_server_cores} {self.oversubscribe} {self.frac_kernel_cores} {self.dedicated_cores} {self.n_client_procs} {self.n_msgs}'
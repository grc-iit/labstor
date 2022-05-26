from jarvis_cd.exec_node import ExecNode
import os

class LabStorKernelServer:
    def Start(self, labstor_root):
        self.node = ExecNode("start_kern_server", self.GetStartCommand(), collect_output=False)
        self.node.Run()

    def Kill(self):
        self.node = ExecNode("kill_kern_server", self.GetKillCommand(), collect_output=False)
        self.node.Run()

    def GetStartCommand(self):
        return f'bash {os.environ["LABSTOR_UTIL"]}/start_kernel_server.sh'

    def GetKillCommand(self):
        return f'bash {os.environ["LABSTOR_UTIL"]}/stop_kernel_server.sh'

class LabStorTrustedServer:
    def __init__(self, conf):
        self.conf = conf
        self.command = self.GetCommand()

    def Run(self):
        self.node = ExecNode("start_trusted_server", self.GetStartCommand(), collect_output=False)
        self.node.Run()

    def Kill(self):
        self.node = ExecNode("kill_trusted_server", self.GetCommand(), collect_output=False)
        self.node.Run()

    def GetStartCommand(self):
        return f'{os.environ["LABSTOR_BUILD_DIR"]}/labstor_trusted_server {self.conf}'

    def GetKillCommand(self):
        return f'{os.environ["LABSTOR_BUILD_DIR"]}/labmods/registrar/terminate'

class LabStack:
    def __init__(self, labmods, build_dir="build"):
        for labmod in labmods:
            os.path.join("")

class PartitionCores:
    def __init__(self, pid, affinity):
        self.affinity = affinity
        self.pid = pid
        command = self.GetCommand()
        super().__init__(command, collect_output=False)

    def GetCommand(self):
        return f'{os.environ["LABSTOR_BUILD_DIR"]}/proc_partition {self.pid} {self.affinity}'

class TestServerThroughput:
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
        super().__init__(command, sleep_period_ms=1000, max_retries=5, collect_output=False)

    def GetCommand(self):
        return f'{os.environ["LABSTOR_BUILD_DIR"]}/test/performance/test_server_thrpt_exec {self.n_server_cores} {self.oversubscribe} {self.frac_kernel_cores} {self.dedicated_cores} {self.n_client_procs} {self.n_msgs}'
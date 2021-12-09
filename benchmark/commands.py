from scsbench.runtime.bash_runtime import BashRuntime
import os

class LabStorKernelServer:
    def Start(self):
        command = f'bash {os.environ["LABSTOR_UTIL"]}/start_kernel_server.sh'
        self.start = BashRuntime(command, do_stdout=True).Run()
        return self.start

    def Kill(self):
        command = f'bash {os.environ["LABSTOR_UTIL"]}/stop_kernel_server.sh'
        self.end = BashRuntime(command, do_stdout=True).Run()

class LabStorTrustedServer(BashRuntime):
    def __init__(self, conf):
        self.conf = conf
        self.command = f'{os.environ["LABSTOR_BUILD_DIR"]}/labstor_trusted_server {self.conf}'
        super().__init__(self.command)

class PartitionCores(BashRuntime):
    def __init__(self, pid, affinity):
        self.pid = pid
        self.affinity = affinity
        command = f'{os.environ["LABSTOR_BUILD_DIR"]}/proc_partition {self.pid} {self.affinity}'
        super().__init__(command, do_stdout=True)
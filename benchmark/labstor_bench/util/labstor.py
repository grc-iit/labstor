
from jarvis_cd.shell.exec_node import ExecNode
from jarvis_cd.shell.kill_node import KillNode
import sys,os

class LabStorKernelServerStart(ExecNode):
    def __init__(self, **kwargs):
        cmd = f"bash {os.environ['LABSTOR_ROOT']}/start_kernel_server.sh"
        super().__init__(cmd, **kwargs)

class LabStorKernelServerStop(ExecNode):
    def __init__(self, **kwargs):
        cmd = f"{os.environ['LABSTOR_BUILD_DIR']}/labmods/registrar/terminate"

class LabStorRuntimeStart(ExecNode):
    def __init__(self, conf, **kwargs):
        cmd = f"labstor_trusted_server {self.conf}"

class LabStorRuntimeStop(KillNode):
    def __init__(self, **kwargs):
        regex = '.*labstor.*'
        super().__init__(regex)
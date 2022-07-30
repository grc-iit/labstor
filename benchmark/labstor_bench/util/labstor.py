
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
        super().__init__(cmd, **kwargs)

class LabStorRuntimeStart(ExecNode):
    def __init__(self, conf, **kwargs):
        cmd = f"labstor_trusted_server {self.conf}"
        super().__init__(cmd, **kwargs)

class LabStorRuntimeStop(KillNode):
    def __init__(self, **kwargs):
        regex = '.*labstor.*'
        super().__init__(regex)

class MountLabStack(ExecNode):
    def __init__(self, labstack, **kwargs):
        cmd = f"mount.labstack {labstack}"
        super().__init__(cmd, **kwargs)

class ModifyLabStack(ExecNode):
    def __init__(self, labstack, **kwargs):
        cmd = f"modify.labstack {labstack}"
        super().__init__(cmd, **kwargs)

class LaunchDummy(ExecNode):
    def __init__(self, nthreads, qdepth, time, **kwargs):
        cmd = f"labstor_dummy {nthreads} {qdepth} {time}"
        super().__init__(cmd, **kwargs)
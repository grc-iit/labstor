
from jarvis_cd.shell.exec_node import ExecNode

class LabStorKernelServer(ExecNode):
    def __init__(self, **kwargs):
        cmd = f"bash {os.environ['LABSTOR_ROOT']}/start_kernel_server.sh"
        super().__init__(cmd, )

from labstor_bench.util.test import Test
from labstor_bench.util.labstor import LabStorKernelServerStart,LabStorKernelServerStop,LabStorRuntimeStart,LabStorRuntimeStop, MountLabStack
import os

class WorkOrchCpuTest(Test):
    def _Replace(self, ncpu_kern, ncpu_runtime):
        conf = YAMLFile(os.path.join(self.root, 'conf', 'config.yaml')).Load()
        conf['kernel_workers'] = []
        conf['server_workers'] = []
        for cpu in ncpu_kern:
            conf['kernel_workers'].append({'worker_id': cpu, 'cpu_id': cpu})
        for cpu in ncpu_runtime:
            conf['server_workers'].append({'worker_id': cpu, 'cpu_id': cpu})
        YAMLFile(os.path.join(self.root, 'config.yaml')).Save()
        return os.path.join(self.root, 'config.yaml')

    def Run(self):
        ncpu = [1,2,4,8]
        for cpu in ncpu:
            conf = self._Replace(ncpu,ncpu)
            LabStorKernelServerStart().Run()
            LabStorRuntimeStart(conf).Run()
            MountLabStack(os.path.join(self.root, 'conf', 'labstack.yaml')).Run()
            LabStorRuntimeStop().Run()
            LabStorKernelServerStop().Run()
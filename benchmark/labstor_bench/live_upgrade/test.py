from labstor_bench.util.labstor import LabStorKernelServerStart,LabStorKernelServerStop,LabStorRuntimeStart,LabStorRuntimeStop,MountLabStack,LaunchDummy
from labstor_bench.util.labstor import MountLabStack,ModifyLabStack,LaunchDummy
from labstor_bench.util.test import Test
from jarvis_cd.serialize.text_file import TextFile
from jarvis_cd.workloads.fio import FIO
import os

class LiveUpgradeTest(Test):
    def _Replace(self, mount_point):
        old_conf = os.path.join(self.root, 'conf', 'simple.fio')
        new_conf = os.path.join(self.root, 'simple.fio')
        conf_text = TextFile(old_conf).Load()
        conf_text = conf_text.replace('{filename}', mount_point)
        TextFile(new_conf).Save(conf_text)
        return new_conf

    def Run(self):
        LabStorKernelServerStart().Run()
        LabStorRuntimeStart(os.path.join(self.root, 'conf', 'config.yaml')).Run()
        MountLabStack(os.path.join(self.root, 'conf', 'labstack_async.yaml')).Run()
        MountLabStack(os.path.join(self.root, 'conf', 'labstack_sync.yaml')).Run()

        n_upgrades = [0, 256, 512, 1024]
        for n in n_upgrades:
            for labstack in ['async', 'sync']:
                node = LaunchDummy(1, 16, 30).Run(exec_async=True)
                for i in range(n):
                    ModifyLabStack(os.path.join(self.root, 'conf', f"labstack_{labstack}.yaml")).Run(exec_async=True)
                node.Wait()

        LabStorRuntimeStop().Run()
        LabStorKernelServerStop().Run()

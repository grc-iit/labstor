
from labstor_bench.util.test import Test
from labstor_bench.util.labstor import LabStorKernelServerStart,LabStorKernelServerStop,LabStorRuntimeStart,LabStorRuntimeStop, MountLabStack
from jarvis_cd.serialize.text_file import TextFile
from jarvis_cd.workloads.fio import FIO
import os

class IoAnatomyTest(Test):
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
        async_conf = self._Replace("fs::/home/async")
        sync_conf = self._Replace("fs::/home/sync")
        FIO(sync_conf).Run()
        FIO(async_conf).Run()
        LabStorRuntimeStop().Run()
        LabStorKernelServerStop().Run()
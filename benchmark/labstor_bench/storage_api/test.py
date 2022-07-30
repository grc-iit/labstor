
from labstor_bench.util.labstor import LabStorKernelServerStart,LabStorKernelServerStop,LabStorRuntimeStart,LabStorRuntimeStop,MountLabStack
from labstor_bench.util.test import Test
from jarvis_cd.serialize.text_file import TextFile
from jarvis_cd.workloads.fio import FIO
import os

class StorageApiTest(Test):
    def _Replace(self, dev_path, engine):
        old_conf = os.path.join(self.root, 'conf', 'microbench.fio')
        new_conf = os.path.join(self.root, 'microbench.fio')
        conf_text = TextFile(old_conf).Load()
        conf_text = conf_text.replace('{filename}', dev_path)
        conf_text = conf_text.replace('{engine}', engine)
        TextFile(new_conf).Save(conf_text)
        return new_conf

    def Run(self):
        engines = ['liburing', 'sync', 'aio', 'libaio', 'io_uring']
        for dev_name,dev_path in self.config['DEVICES'].items():
            for engine in engines:
                conf = self._Replace(dev_path,engine)
                node = FIO(dev_path, sudo=True).Run()
                node.Run()

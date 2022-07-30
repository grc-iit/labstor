
from labstor_bench.util.test import Test
from jarvis_cd.serialize.text_file import TextFile
from jarvis_cd.workloads.fio import FIO
import os

class PreconditionTest(Test):
    def _Replace(self, dev_path, engine):
        old_conf = os.path.join(self.root, 'conf', 'microbench.fio')
        new_conf = os.path.join(self.root, 'microbench.fio')
        conf_text = TextFile(old_conf).Load()
        conf_text = conf_text.replace('{filename}', dev_path)
        TextFile(new_conf).Save(conf_text)
        return new_conf

    def Run(self):
        dev_path = self.config['DEVICES']['NVME_PATH']
        conf = self._Replace(dev_path)
        node = FIO(dev_path, sudo=True).Run()
        node.Run()
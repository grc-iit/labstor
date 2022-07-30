
from labstor_bench.util.test import Test
from jarvis_cd.serialize.text_file import TextFile
from jarvis_cd.workloads.fio import FIO
import os

class IoschedBlkswTest(Test):
    def _Replace(self, name, dev):
        old_conf = os.path.join(self.root, 'conf', name)
        new_conf = os.path.join(self.root, name)
        conf_text = TextFile(old_conf).Load()
        conf_text = conf_text.replace('{filename}', dev)
        TextFile(new_conf).Save(conf_text)
        return new_conf

    def Run(self):
        dev = self.config['DEVICES']['NVME_PATH']
        l = self._Replace('latency.fio', dev)
        t = self._Replace('thrpt.fio', dev)
        print(l)
        LNode = FIO(l, exec_async=True, sudo=True).Run()
        TNode = FIO(t, exec_async=True, sudo=True).Run()
        LNode.Wait()
        TNode.Wait()

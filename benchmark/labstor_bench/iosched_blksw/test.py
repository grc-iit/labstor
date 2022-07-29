
from labstor_bench.util.test import Test
from jarvis_cd.serialize.ini_file import IniFile
from jarvis_cd.fs.fs import FIO
import os

class IoschedBlksw(Test):
    def Run(self):
        dev = self.config['DEVICES']['NVME_PATH']

        l = os.path.join(self.root, 'latency.fio')
        t = os.path.join(self.root, 'thrpt.fio')
        L = IniFile(l)
        T = IniFile(l)

        conf = L.Load()
        conf['filename'] = dev
        L.Save(conf)

        conf = T.Load()
        conf['filename'] = dev
        T.Save(conf)

        LNode = FIO(l, exec_async=True).Run()
        TNode = FIO(t, exec_async=True).Run()
        LNode.Wait()
        TNode.Wait()

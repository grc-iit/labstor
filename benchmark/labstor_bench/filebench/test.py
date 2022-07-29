
from labstor_bench.util.test import Test
from jarvis_cd.fs.fs.filebench import FilebenchNode
from jarvis_cd.fs.fs import EXT4Format, XFSFormat, F2FSFormat, UnmountFS, MountFS, Filebench
import os

class FilebenchTest(Test):
    def Run(self):
        mount = self.config['MOUNT_POINT']
        fses = [EXT4Format, XFSFormat, F2FSFormat]
        configs = ['fileserver', 'varmail', 'webproxy', 'webserver']
        for dev in ['NVME_PATH', 'PMEM_PATH']:
            for fs_format in fses:
                UnmountFS(mount).Run()
                fs_format(self.config['DEVICES'][dev]).Run()
                MountFS(mount).Run()
                for config in configs:
                    FilebenchNode(os.path.join(self.root, f"{config.f}")).Run()
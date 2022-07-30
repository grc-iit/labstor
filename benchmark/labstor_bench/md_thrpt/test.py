
from labstor_bench.util.test import Test
from jarvis_cd.serialize.ini_file import IniFile
from jarvis_cd.fs.fs import EXT4Format, XFSFormat, F2FSFormat, UnmountFS, MountFS, ChownFS
from jarvis_cd.workloads.fxmark import FxMark, FxMarkOp
import os

class MdThrpt(Test):
    def Run(self):
        ncore = [1,2,3,4,5,6]
        formats = [EXT4Format, XFSFormat, F2FSFormat]
        mount = self.config['MOUNT_POINT']
        dev_path = self.config['DEVICES']['NVME_PATH']
        for cores in ncore:
            for fs_format in formats:
                UnmountFS(mount).Run()
                fs_format().Run()
                MountFS(dev_path, mount).Run()
                ChownFS(mount).Run()
                FxMark(FxMarkOp.MWCL, cores, 20, mount).Run()

from labstor_bench.util.test import Test
from jarvis_cd.serialize.ini_file import IniFile
from jarvis_cd.fs.fs import EXT4Format, XFSFormat, F2FSFormat, UnmountFS, MountFS,FxMark,FxMarkOp
import os

class MdThrpt(Test):
    def Run(self):
        ncore = [1,2,3,4,5,6]
        formats = [EXT4Format, XFSFormat, F2FSFormat]
        mount = self.config['MOUNT_POINT']
        for cores in ncore:
            for fs_format in formats:
                UnmountFS(mount).Run()
                fs_format(self.config['DEVICES']['NVME_PATH']).Run()
                MountFS(mount).Run()
                FxMark(FxMarkOp.MWCL, cores, 20, self.config['MOUNT_POINT']).Run()
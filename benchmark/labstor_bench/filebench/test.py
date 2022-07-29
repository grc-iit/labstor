
from labstor_bench.util.test import Test
from jarvis_cd.fs.fs.filebench import FilebenchNode
from jarvis_cd.fs.fs import EXT4Node, XFSNode, F2FSNode, UnmountFS

class FilebenchTest(Test):
    def Run(self):
        UnmountFS(self.config['MOUNT_POINT']).Run()
        EXT4Format(self.config['NVME']).Run()
        FilebenchNode('').Run()
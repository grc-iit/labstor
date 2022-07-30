
from labstor_bench.util.test import Test
from jarvis_cd.fs.fs import EXT4Format, XFSFormat, F2FSFormat, UnmountFS, MountFS, ChownFS, DisableVARandomization
from labstor_bench.util.labstor import LabStorKernelServerStart,LabStorKernelServerStop,LabStorRuntimeStart,LabStorRuntimeStop, MountLabStack
from jarvis_cd.workloads.filebench import Filebench
from jarvis_cd.fs.mkdir_node import MkdirNode
from jarvis_cd.serialize.text_file import TextFile
from jarvis_cd.basic.echo_node import EchoNode
import pandas as pd
import os

class FilebenchTest(Test):
    def Run(self):
        mount = self.config['MOUNT_POINT']
        fses = [EXT4Format, XFSFormat, F2FSFormat]
        fs_pretty = ['ext4', 'xfs', 'f2fs']
        configs = ['fileserver', 'varmail', 'webproxy', 'webserver']
        nthreads=4
        DisableVARandomization().Run()
        results = []
        for config in configs:
            raw_config_path = os.path.join(self.root, "conf", f"{config}.f")
            config_path = os.path.join(self.root, f"{config}.f")
            config_text = TextFile(raw_config_path).Load()
            config_text = config_text.replace('{mount}', f"\"{mount}\"")
            config_text = config_text.replace('{nthreads}', str(nthreads))
            TextFile(config_path).Save(config_text)
            for dev in ['NVME_PATH', 'PMEM_PATH']:
                dev_path = self.config['DEVICES'][dev]
                for fs_format,fs_type in zip(fses,fs_pretty):
                    UnmountFS(mount).Run()
                    fs_format(dev_path).Run()
                    MountFS(dev_path, mount).Run()
                    ChownFS(mount).Run()
                    EchoNode("Starting filebench").Run()
                    node = Filebench(config_path).Run()
                    result = node.Parse()
                    result['config'] = config
                    result['dev'] = dev_path
                    result['fs_format'] = fs_type
                    results.append(result)
        pd.DataFrame(results).to_csv(os.path.join(self.config['LOG_DIR'], 'filebench.csv'))
import sys,os
from jarvis_cd.exec_node import ExecNode
from jarvis_cd.test_tracker import TestTracker
from jarvis_cd.fs.fs import DropCaches, MountFS, UnmountFS, ChownFS, EXT4Format, XFSFormat, F2FSFormat
#from jarvis_cd.workloads.ycsb import YCSB
from .ycsb import YCSB
import pandas as pd

class RocksDBTest(TestTracker):
    def __init__(self, conf):
        super().__init__(conf.log_dir)
        OP_TYPE = ["load", "run"]
        WORKLOAD = ["a", "b", "c", "d", "e", "f"]
        WORKLOAD = [os.path.join(conf.ycsb_dir, "workloads", f"workload{w}") for w in WORKLOAD]
        FS_TYPE = ["EXT4", "XFS", "F2FS"]
        DEV_PATH = [conf.nvme_path]

        self.conf = conf
        self.ycsb_dir = conf.ycsb_dir
        self.fs_mount = conf.mount_point
        self.SetVariables(OP_TYPE, WORKLOAD, FS_TYPE, DEV_PATH)

    def ExperimentInit(self):
        return

    def ExperimentEnd(self):
        return

    def TrialInit(self, op_type, workload, fs_type, dev_path):
        UnmountFS("umount", dev_path).Run()
        if fs_type == "EXT4":
            FormatFS = EXT4Format
        if fs_type == "XFS":
            FormatFS = XFSFormat
        if fs_type == "F2FS":
            FormatFS = F2FSFormat
        FormatFS("fmt", dev_path, self.fs_mount).Run()
        MountFS("mount", dev_path, self.fs_mount).Run()
        return True

    def Trial(self, op_type, workload, fs_type, dev_path):
        ycsb = YCSB("ycsb", "rocksdb", op_type, workload, self.fs_mount, self.ycsb_dir)
        ycsb.Run()
        if ycsb.GetExitCode() != 0:
            ycsb.Print(ycsb.output)
            raise Exception("YCSB failed")
        runtime = ycsb.GetRuntime()
        print(f"RUNTIME: {runtime}")
        return runtime

    def TrialEnd(self, op_type, workload, fs_type, dev_path):
        return True

    def SaveResults(self):
        records = []
        for trial,runtime in self.trials.items():
            record = {
                "op_type": trial[0],
                "workload": trial[1],
                "fs_type": trial[2],
                "dev_path": trial[3],
                "runtime": runtime
            }
            records.append(record)
        df = pd.DataFrame(records)
        df.to_csv(os.path.join(self.test_dir, "results.csv"))
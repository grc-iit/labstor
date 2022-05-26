from jarvis_cd.exec_node import ExecNode
import re
import os,sys

class FIO(ExecNode):
    def __init__(self, name, ,
                 print_output=False, collect_output=True, affinity=None, sleep_period_ms=100, max_retries=0):
        params = [
            ""
        ]
        cmd = "fio"
        super().__init__(name, cmd, print_output, collect_output, affinity, sleep_period_ms, max_retries, cwd=ycsb_dir)

    def GetRuntime(self):
        for host,outputs in self.output.items():
            for line in outputs['stdout']:
                grp = re.match("\[OVERALL\], RunTime\(ms\), ([0-9]+)", line)
                if grp:
                    return float(grp.group(1))
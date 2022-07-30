import argparse
from enum import Enum
from labstor_bench.util.test import TestConfig
from jarvis_cd.util.naming import ToCamelCase
import yaml
import inspect, pathlib
import os

class TestCase(Enum):
    Anatomy = "io_anatomy",
    LiveUpgrade = "live_upgrade",
    WorkOrchCPU = "work_orch_cpu",
    WorkOrchReq = "work_orch_req",
    StorageAPI = "storage_api",
    IOSchedLabStor = "iosched_labstor"
    IOSchedBlksw = "iosched_blksw"
    Filebench = "filebench"

    def __str__(self):
        return str(self.value)

parser = argparse.ArgumentParser(description="LabStor Benchmark")
parser.add_argument('test_case', metavar="case", choices=list(TestCase), type=TestCase, help="The LabStor test case to run")
args = parser.parse_args()

bench_root = pathlib.Path(__file__).parent.resolve()
conf = TestConfig(os.path.join(bench_root, 'config.yaml')).Get()

test_case = str(args.test_case)
class_name = f"{ToCamelCase(test_case)}Test"
jarvis_test = __import__(f"labstor_bench.{test_case}.test", fromlist=[class_name])
klass = getattr(jarvis_test, class_name)

klass(conf).Run()
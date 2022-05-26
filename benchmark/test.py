from labstor_bench.rocksdb import RocksDBTest
from labstor_bench.bench_conf import LabStorBenchConfig
import os,sys

conf = LabStorBenchConfig(sys.argv[1])
tests = RocksDBTest(conf)
tests.Run()
tests.SaveResults()
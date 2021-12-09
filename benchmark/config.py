import sys, os
import yaml
import multiprocessing


class LabStorConfigurator:
    def __init__(self):
        self.conf = None
        return
    def _not0(self, num):
        if num == 0:
            return 1
        return int(num)
    def _num(self, num):
        return int(num)
    def Load(self, path):
        with open(path, 'r') as fp:
            self.conf = yaml.safe_load(fp)
    def AddKernelWorker(self, worker_id, cpu_id):
        self.conf["work_orchestrator"]["kernel_workers"].append({'worker_id': worker_id, "cpu_id": cpu_id})
    def AddServerWorker(self, worker_id, cpu_id):
        self.conf["work_orchestrator"]["server_workers"].append({'worker_id': worker_id, "cpu_id": cpu_id})
    def GetBothServerAffinities(self):
        num_cpus = multiprocessing.cpu_count()
        cpu_set = {cpu:False for cpu in range(num_cpus)}
        for worker_conf in self.conf["work_orchestrator"]["kernel_workers"]:
            cpu_set[worker_conf["cpu_id"]] = True
        for worker_conf in self.conf["work_orchestrator"]["server_workers"]:
            cpu_set[worker_conf["cpu_id"]] = True
        return {cpu for cpu in range(num_cpus) if cpu_set[cpu]}
    def GetBothServerAffinitiesCmdline(self):
        return " ".join([str(cpu) for cpu in self.GetBothServerAffinities()])
    def GetApplicationAffinity(self):
        num_cpus = multiprocessing.cpu_count()
        cpu_set = {cpu:True for cpu in range(num_cpus)}
        for worker_conf in self.conf["work_orchestrator"]["kernel_workers"]:
            cpu_set[worker_conf["cpu_id"]] = False
        for worker_conf in self.conf["work_orchestrator"]["server_workers"]:
            cpu_set[worker_conf["cpu_id"]] = False
        return {cpu for cpu in range(num_cpus) if cpu_set[cpu]}
    def GetApplicationAffinityCmdline(self):
        return " ".join([str(cpu) for cpu in self.GetApplicationAffinity()])
    def ResetWorkers(self):
        self.conf["work_orchestrator"]["kernel_workers"] = []
        self.conf["work_orchestrator"]["server_workers"] = []
    def SetFractionalWorkers(self, ncpu, oversubscribe, kern_frac):
        self.ResetWorkers()
        ncpu = int(ncpu)
        kern_frac = float(kern_frac)
        n_server_cpu = self._not0(ncpu * (1 - kern_frac))
        n_server_workers = n_server_cpu * oversubscribe
        n_kernel_cpu = self._not0(ncpu * kern_frac)
        kernel_cpu_start = self._num(ncpu * (1 - kern_frac))
        n_kernel_workers = n_kernel_cpu
        for worker_id in range(n_server_workers):
            self.AddServerWorker(worker_id, worker_id % n_server_cpu)
        for worker_id in range(n_kernel_workers):
            self.AddKernelWorker(worker_id, kernel_cpu_start + worker_id % n_kernel_cpu)
    def SaveTemp(self):
        tmp = self.GetTemp()
        with open(tmp, 'w') as fp:
            yaml.safe_dump(self.conf, fp, default_flow_style=False, allow_unicode=True)
    def GetTemp(self):
        return "/tmp/labstor_conf.yaml"
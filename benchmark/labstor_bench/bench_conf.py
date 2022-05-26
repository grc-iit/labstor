import configparser
import os,sys

class LabStorBenchConfig:
    def __init__(self, path):
        self.config = {}
        config = configparser.ConfigParser()
        config.read(path)
        for section in config.sections():
            if section not in self.config:
                self.config[section] = {}
            for key in config[section]:
                self.config[section][key.upper()] = os.path.expandvars(config[section][key])

        self.hdd_path = self.config["CONFIG"]["HDD_PATH"]
        self.ssd_path = self.config["CONFIG"]["SSD_PATH"]
        self.nvme_path = self.config["CONFIG"]["NVME_PATH"]
        self.pmem_path = self.config["CONFIG"]["PMEM_PATH"]
        self.log_dir = self.config["CONFIG"]["LOG_DIR"]
        self.mount_point = self.config["CONFIG"]["MOUNT_POINT"]
        self.ycsb_dir = self.config["YCSB"]["YCSB_DIR"]
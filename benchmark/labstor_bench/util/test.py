from abc import abstractmethod,ABC
from jarvis_cd.serialize.yaml_file import YAMLFile
from jarvis_cd.fs.mkdir_node import MkdirNode
import inspect,os


class TestConfig:
    def __init__(self, path):
        self.config = YAMLFile(path).Load()
        self.config['MOUNT_POINT'] = os.path.expandvars(self.config['MOUNT_POINT'])
        self.config['LOG_DIR'] = os.path.expandvars(self.config['LOG_DIR'])
        MkdirNode(self.config['MOUNT_POINT']).Run()
        MkdirNode(self.config['LOG_DIR']).Run()

    def Get(self):
        return self.config

class Test(ABC):
    def __init__(self, config):
        self.config = config
        self.root = os.path.dirname(inspect.getfile(self.__class__))

    @abstractmethod
    def Run(self):
        pass
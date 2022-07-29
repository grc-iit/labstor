from abc import abstractmethod,ABC
from jarvis_cd.serialize.yaml_file import YAMLFile
import inspect,os


class TestConfig:
    def __init__(self, path):

class Test(ABC):
    def __init__(self, config):
        self.config = config
        self.dir = os.path.dirname(inspect.getfile(self.__class__))

    @abstractmethod
    def Run(self):
        pass
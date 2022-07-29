from abc import abstractmethod,ABC
from jarvis_cd.serialize.yaml_file import YAMLFile

class TestConfig:
    def __init__(self, path):

class Test(ABC):
    def __init__(self, config):
        self.config = config

    @abstractmethod
    def Run(self):
        pass
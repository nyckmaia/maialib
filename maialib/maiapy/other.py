from importlib import resources


def mySum(x: int, y: int) -> int:
    return x + y


def getData(filePath: str) -> str:
    with resources.path('maialib', 'xml-scores-examples') as data_path:
        default_config_path = data_path / filePath
        return str(default_config_path)

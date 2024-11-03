import os
import sys
from enum import Enum
import subprocess
import maialib.maiacore as mc
import importlib.resources as pkg_resources

__all__ = ["getSampleScorePath", "SampleScore",
           "setScoreEditorApp", "getScoreEditorApp", "openScore", "getXmlSamplesDirPath"]

_scoreEditorApp = ""


def setScoreEditorApp(executableFullPath: str) -> None:
    """Set the full path to the installed score editor app

    Args:
       executableFullPath (str): Score editor full path
       Example 01: "C:/path/to/MuseScore"
       Example 02: "/Applications/MuseScore 4.app/Contents/MacOS/mscore"

    Examples of use:

    >>> import maialib as ml
    >>> # Example for Windows:
    >>> ml.setScoreEditorApp("C:/path/to/MuseScore.exe")
    >>> # Example for Mac OSX:
    >>> ml.setScoreEditorApp("/Applications/MuseScore 4.app/Contents/MacOS/mscore")
    """
    global _scoreEditorApp

    if os.path.isfile(executableFullPath):
        _scoreEditorApp = executableFullPath
    else:
        raise ValueError('Invalid executable full path')


def getScoreEditorApp() -> str:
    global _scoreEditorApp
    return _scoreEditorApp


def openScore(score: mc.Score) -> None:
    score.toFile("temp")
    global _scoreEditorApp

    if not _scoreEditorApp:
        raise RuntimeError(
            "Please, set your installed music score editor using the 'setScoreEditorApp' function")

    print(f"Opening Score Editor App: {_scoreEditorApp}")
    ret = subprocess.Popen([_scoreEditorApp, "temp.xml"])
    ret.wait()


class SampleScore(Enum):
    Bach_Cello_Suite_1 = "Bach_Cello_Suite_1"
    Beethoven_Symphony_5th = "Beethoven_Symphony_5th"
    Chopin_Fantasie_Impromptu = "Chopin_Fantasie_Impromptu"
    Dvorak_Symphony_9_mov_4 = "Dvorak_Symphony_9_mov_4"
    Mahler_Symphony_8_Finale = "Mahler_Symphony_8_Finale"
    Mozart_Requiem_Introitus = "Mozart_Requiem_Introitus"
    Strauss_Also_Sprach_Zarathustra = "Strauss_Also_Sprach_Zarathustra"


def getSampleScorePath(sampleEnum: SampleScore) -> str:
    """Get a maialib internal XML sample file

    Args:
       sampleEnum (SampleScore): Maialib SampleScore enum value
           - Bach_Cello_Suite_1
           - Beethoven_Symphony_5th
           - Chopin_Fantasie_Impromptu
           - Dvorak_Symphony_9_mov_4
           - Mahler_Symphony_8_Finale
           - Mozart_Requiem_Introitus
           - Strauss_Also_Sprach_Zarathustra

    Kwargs:
       None

    Returns:
       A full file path (str) to the XML maialib internal sample score

    Raises:
       RuntimeError

    Examples of use:

    >>> import maialib as ml
    >>> filePath = ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1)
    >>> score = ml.Score(filePath)
    >>> score.info()
    """
    # Get the actual XML file name for the given 'alias'
    xmlFileName = {
        SampleScore.Bach_Cello_Suite_1: "Bach_Cello_Suite_1.mxl",
        SampleScore.Beethoven_Symphony_5th: "Beethoven_Symphony_5_mov_1.xml",
        SampleScore.Chopin_Fantasie_Impromptu: "Chopin_Fantasie_Impromptu.mxl",
        SampleScore.Dvorak_Symphony_9_mov_4: "Dvorak_Symphony_9_mov_4.mxl",
        SampleScore.Mahler_Symphony_8_Finale: "Mahler_Symphony_8_Finale.mxl",
        SampleScore.Mozart_Requiem_Introitus: "Mozart_Requiem_Introitus.mxl",
        SampleScore.Strauss_Also_Sprach_Zarathustra: "Strauss_Also_Sprach_Zarathustra.mxl"
    }[sampleEnum]

    # xmlDir = pkg_resources.files("maialib").joinpath("xml-scores-examples")
    xmlDir = ""
    if sys.version_info >= (3, 9):
        from importlib import resources
        xmlDir = resources.files("maialib").joinpath("xml-scores-examples")
    else:
        from importlib_resources import files
        xmlDir = files("maialib").joinpath("xml-scores-examples")

    fileFullPath = os.path.join(xmlDir, xmlFileName)
    return str(fileFullPath)


def getXmlSamplesDirPath() -> str:
    """Get the maialib XML samples directory path

    Kwargs:
       None

    Returns:
       A full dir path (str) to the XML maialib internal samples score directory

    Raises:
       RuntimeError

    Examples of use:

    >>> import maialib as ml
    >>> xmlDir = ml.getXmlSamplesDirPath()
    >>> score = ml.Score(xmlDir + "Bach/cello_suite_1_violin.xml")
    >>> score.info()
    """
    xmlDir = pkg_resources.files("maialib").joinpath("xml-scores-examples")
    return str(xmlDir)

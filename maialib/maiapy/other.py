import os
import pkg_resources
from enum import Enum
import platform
import subprocess
import maialib as ml

__all__ = ["getSampleScorePath", "SampleScore",
           "setScoreEditorApp", "getScoreEditorApp", "openScore"]

_scoreEditorApp = ""


def setScoreEditorApp(executableFullPath: str) -> None:
    """Set the full path to the installed score editor app

    Args:
       executableFullPath (str):  Score editor full path
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

    if (os.path.isfile(executableFullPath)):
        _scoreEditorApp = executableFullPath
    else:
        raise ValueError('Invalid executable full path')


def getScoreEditorApp() -> str:
    global _scoreEditorApp
    return _scoreEditorApp


def openScore(score: ml.Score) -> None:
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
       sampleEnum (SampleScore):  Maialib SampleScore enum value
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

    xmlDir = pkg_resources.resource_filename("maialib", "xml-scores-examples")
    fileFullPath = os.path.join(xmlDir, xmlFileName)
    return fileFullPath

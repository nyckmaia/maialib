from importlib import resources


def getScoreSamplePath(sampleName: str) -> str:
    """Get a maialib internal XML sample file

    Args:
       sampleName (str):  Maialib pre-defined XML sample name
           - "Bach-Solo"
           - "Beethoven-Orchestra"

    Kwargs:
       None

    Returns:
       A full file path (str) to the XML maialib internal sample

    Raises:
       RuntimeError

    Examples of use:

    >>> import maialib as ml
    >>> filePath = ml.getScoreSamplePath("Beethoven-Orchestra")
    >>> score = ml.Score(filePath)
    >>> score.info()
    """
    # ===== INPUT VALIDATION ===== #
    validInputs = ["Bach-Solo", "Beethoven-Orchestra"]

    if sampleName not in validInputs:
        raise RuntimeError(
            f"Invalid sample name: {sampleName}.\nThe valid keywords are: {validInputs}")

    xmlFileName = ""
    match sampleName:
        case "Bach-Solo":
            xmlFileName = "cello_suite_1_violin.xml"
        case "Beethoven-Orchestra":
            xmlFileName = "Symphony_5th_1Mov.xml"

    with resources.path('maialib', 'xml-scores-examples') as data_path:
        default_config_path = data_path / xmlFileName
        return str(default_config_path)

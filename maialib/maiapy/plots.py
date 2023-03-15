import maialib.maiacore as mc
import pandas as pd
import plotly.express as px

__all__ = ["plotPartsActivity", "plotPianoRoll"]


def _score2DataFrame(score: mc.Score, kwargs) -> None:
    """Auxiliar function to convert a maialib Score object to a Pandas DataFrame

    Args:
       score (maialib.Score):  A maialib Score object loaded with a valid MusicXML file

    Kwargs:
       measureStart (int): Start measure to plot
       measureEnd (int): End measure to plot
       partNames (list): A str list that contains the filtered desired score parts to plot

    Returns:
       None

    Raises:
       RuntimeError, KeyError
    """
    # ===== INPUT VALIDATION ===== #

    # 1) Validate keywords arguments keys
    params = ["measureStart", "measureEnd", "partNames"]
    for k in kwargs.keys():
        if k not in params:
            raise RuntimeError(
                f"plotPartsActivity() got an unexpected keyword argument '{k}'.\nThe valid keywords are: {params}")

    # 2) Validate 'partNames'
    # 2.1) Type cheking
    if "partNames" in kwargs and not isinstance(kwargs["partNames"], list):
        print(
            "ERROR: 'partNames' is a optional kwargs argument and MUST BE a strings array")
        print(score.getPartNames())
        return

    # 2.2) Check each individual part name
    partNames = []
    if not "partNames" in kwargs:
        partNames = score.getPartNames()
    else:
        for partNameValue in kwargs["partNames"]:
            idx = 0
            isValid = score.getPartIndex(partNameValue, idx)

            if not isValid:
                print(f"ERROR: Invalid part name: {partNameValue}")
                print(score.getPartNames())
                return

            partNames.append(partNameValue)

    # 3) Validate 'author' and 'work title'
    author = score.getComposerName()
    work_title = score.getTitle()

    if str(author) == "":
        author = "No Author"

    if str(work_title) == "":
        work_title = "No Title"

    # 4) Validade 'measureStart' and 'measureEnd'
    measureStart = 1
    measureEnd = score.getNumMeasures() + 1

    if "measureStart" in kwargs:
        measureStart = kwargs["measureStart"]
        if measureStart < 1:
            print("ERROR: 'measureStart' must be greater than 1")
            return

    if "measureEnd" in kwargs:
        measureEnd = kwargs["measureEnd"]
        if measureEnd > score.getNumMeasures():
            print(
                f"ERROR: 'measureEnd' must be lesser than than {score.getNumMeasures() + 1}'")
            return

    if measureEnd < measureStart:
        print("ERROR: 'measureEnd' must be greater than 'measureStart'")
        return

    # ===== CREATE BASIC DATA STRUCTURE ===== #
    plotData = {}
    plotData["notesData"] = []

    # ===== ITERATE ON EACH SCORE NOTE ===== #
    for partName in partNames:  # For each part 'p'
        currentPart = score.getPart(partName)
        # partName = currentPart.getName()

        # Control variable: note time position on score
        currentTimePosition = 0

        for m in range(measureStart - 1, measureEnd - 1):  # for each measure 'm'
            currentMeasure = currentPart.getMeasure(m)

            measureQuarterTimeAmount = 0
            for n in range(currentMeasure.getNumNotes(0)):  # for each note 'n'
                note = currentMeasure.getNote(n, 0)

                # Skip other voices
                if note.getVoice() != 1 or note.inChord():
                    continue

                measureQuarterTimeAmount = measureQuarterTimeAmount + note.getQuarterDuration()

            if m == measureStart - 1:
                currentTimePosition = measureQuarterTimeAmount * measureStart

            for s in range(currentMeasure.getNumStaves()):  # for each stave 's'
                staveNumElememts = currentMeasure.getNumNotes(s)

                # Temp control variables
                internalStaveCurrentTime = 0
                currentVoice = 1
                for n in range(staveNumElememts):  # for each note 'n'
                    currentNote = currentMeasure.getNote(n, s)

                    voice = currentNote.getVoice()

                    # Reset time on voice change
                    if voice != currentVoice:
                        internalStaveCurrentTime = 0
                        currentVoice = voice

                    # Skip chord upper notes (unnecessary for this plot type)
                    if currentNote.inChord():
                        continue

                    # Get note data
                    noteDuration = currentNote.getQuarterDuration()
                    midiValue = currentNote.getMIDINumber()
                    notePitch = currentNote.getPitch()

                    aux = currentTimePosition + internalStaveCurrentTime
                    noteStart = (aux / measureQuarterTimeAmount) + 1
                    noteFinish = ((aux + noteDuration) /
                                  measureQuarterTimeAmount) + 1

                    # This plotly timeline function requires the use of these 3 names below: 'Tasks', 'Start' and 'Finish'
                    noteData = {
                        "Task": partName,
                        "Start": noteStart,
                        "Finish": noteFinish,
                        "midiValue": midiValue,
                        "notePitch": notePitch
                    }

                    # Increment control time variable
                    internalStaveCurrentTime = internalStaveCurrentTime + noteDuration

                    # Skip rests
                    if (currentNote.isNoteOff()):
                        continue

                    # Add 'noteData' object to the list
                    plotData["notesData"].append(noteData)

            # Update the global current time position
            currentTimePosition = currentTimePosition + measureQuarterTimeAmount

    # ===== CREATE THE VISUAL PLOT ===== #
    df = pd.DataFrame(plotData["notesData"])
    df['delta'] = df['Finish'] - df['Start']

    return [df, author, work_title]

# plotPartsActivity
#
# This function plots a timeline graph showing the musical activity of each score instrument
#
# Paper: Uma análise da organização e fragmentação de Farben de Arnold Schoenberg (2013)
# Author: Prof. Igor Leão Maia (UFMG)
# Contributor: Prof. Adolfo Maia Junior (UNICAMP)
# Code Implementation: PhD. Nycholas Maia (UNICAMP) - 01/02/2023
#
# To get more information about it:
# https://www.researchgate.net/publication/321335427_Uma_analise_da_organizacao_e_fragmentacao_de_Farben_de_Arnold_Schoenberg


def plotPartsActivity(score: mc.Score, **kwargs) -> None:
    """Plots a timeline graph showing the musical activity of each score instrument

    Args:
       score (maialib.Score):  A maialib Score object loaded with a valid MusicXML file

    Kwargs:
       measureStart (int): Start measure to plot
       measureEnd (int): End measure to plot
       partNames (list): A str list that contains the filtered desired score parts to plot

    Returns:
       None

    Raises:
       RuntimeError, KeyError

    Examples of use:

    >>> plotPartsActivity(myScore)
    >>> plotPartsActivity(myScore, measureStart=50)
    >>> plotPartsActivity(myScore, measureStart=50, measureEnd=100)
    >>> plotPartsActivity(myScore, measureStart=50, measureEnd=100, partNames=["Violin 1", "Cello"])
    """
    # ===== CREATE A PLOTLY TIMELINE PLOT ===== #
    df, author, work_title = _score2DataFrame(score, kwargs)
    fig = px.timeline(df, template="plotly_white", x_start="Start",
                      x_end="Finish", y="Task", color="midiValue", hover_data=["notePitch"], labels={"Task": "Part", "Start": "Start Measure", "Finish": "Finish Measure", "notePitch": "Pitch", "midiValue": "MIDI Value"}, color_continuous_scale=px.colors.sequential.Turbo_r, title=f"<b>Parts Activity<br>{work_title} - {author}</b>")

    fig.data[0].x = df.delta.tolist()

    # Update plot layout
    fig.update_xaxes(type='linear', autorange=True, showgrid=True,
                     gridwidth=1, title="Measures")
    fig.update_yaxes(autorange="reversed", showgrid=True,
                     gridwidth=1, ticksuffix="  ")
    fig.update_layout(title_x=0.5, yaxis_title=None, font={
        "size": 18,
    }, coloraxis_colorbar=dict(
        title='Pitch',
        tickvals=[12, 24, 36, 48, 60, 72, 84, 96, 108, 120],
        ticktext=["C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9"],
    ))

    fig.add_shape(
        # Rectangle with reference to the plot
        type="rect",
        xref="paper",
        yref="paper",
        x0=0,
        y0=0,
        x1=1.0,
        y1=1.0,
        line=dict(
            color="black",
            width=1,
        )
    )

    fig.show()


def plotPianoRoll(score: mc.Score, **kwargs) -> None:
    """Plots a piano roll graph showing the musical activity of each score instrument

    Args:
       score (maialib.Score):  A maialib Score object loaded with a valid MusicXML file

    Kwargs:
       measureStart (int): Start measure to plot
       measureEnd (int): End measure to plot
       partNames (list): A str list that contains the filtered desired score parts to plot

    Returns:
       None

    Raises:
       RuntimeError, KeyError

    Examples of use:

    >>> plotPianoRoll(myScore)
    >>> plotPianoRoll(myScore, measureStart=50)
    >>> plotPianoRoll(myScore, measureStart=50, measureEnd=100)
    >>> plotPianoRoll(myScore, measureStart=50, measureEnd=100, partNames=["Violin 1", "Cello"])
    """
    # ===== CREATE A PLOTLY TIMELINE PLOT ===== #
    df, author, work_title = _score2DataFrame(score, kwargs)

    fig = px.timeline(df, template="plotly_white", x_start="Start",
                      x_end="Finish", y="midiValue", color="Task", hover_data=["notePitch"], labels={"Task": "Part", "Start": "Start Measure", "Finish": "Finish Measure", "notePitch": "Pitch", "midiValue": "MIDI Value"}, title=f"<b>Piano Roll<br>{work_title} - {author}</b>")

    for d in fig.data:
        d.x = df.delta.tolist()

    # Update plot layout
    fig.update_xaxes(type='linear', autorange=True, showgrid=True,
                     gridwidth=1, title="Measures")
    fig.update_yaxes(autorange=True, showgrid=True,
                     gridwidth=1, title='Pitch',
                     tickvals=[12, 24, 36, 48, 60, 72, 84, 96, 108, 120],
                     ticktext=["C0 ", "C1 ", "C2 ", "C3 ", "C4 ",
                               "C5 ", "C6 ", "C7 ", "C8 ", "C9 "],
                     )
    fig.update_layout(title_x=0.5, font={"size": 18})

    fig.add_shape(
        # Rectangle with reference to the plot
        type="rect",
        xref="paper",
        yref="paper",
        x0=0,
        y0=0,
        x1=1.0,
        y1=1.0,
        line=dict(
            color="black",
            width=1,
        )
    )

    fig.update_traces(width=0.8)

    fig.show()

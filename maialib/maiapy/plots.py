import plotly.graph_objects as go
import maialib.maiacore as mc
import pandas as pd
import plotly.express as px
import plotly
from typing import List, Tuple

__all__ = ["plotPartsActivity", "plotPianoRoll",
           "plotScorePitchEnvelope", "plotChordsNumberOfNotes"]


def _score2DataFrame(score: mc.Score, kwargs) -> Tuple[pd.DataFrame, str, str]:
    """Auxiliar function to convert a maialib Score object to a Pandas DataFrame

    Args:
       score (maialib.Score):  A maialib Score object loaded with a valid MusicXML file

    Kwargs:
       measureStart (int): Start measure to plot
       measureEnd (int): End measure to plot
       partNames (list): A str list that contains the filtered desired score parts to plot

    Returns:
       Tuple: DataFrame, author, work_title

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
        print(score.getPartsNames())
        return

    # 2.2) Check each individual part name
    partNames = []
    if not "partNames" in kwargs:
        partNames = score.getPartsNames()
    else:
        for partNameValue in kwargs["partNames"]:
            idx = 0
            isValid = score.getPartIndex(partNameValue, idx)

            if not isValid:
                print(f"ERROR: Invalid part name: {partNameValue}")
                print(score.getPartsNames())
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
                    midiValue = currentNote.getMidiNumber()
                    notePitch = currentNote.getPitch()

                    aux = currentTimePosition + internalStaveCurrentTime
                    noteStart = (aux / measureQuarterTimeAmount)
                    noteFinish = (aux + noteDuration) / \
                        measureQuarterTimeAmount

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

    return df, author, work_title

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


def plotPartsActivity(score: mc.Score, **kwargs) -> Tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]:
    """Plots a timeline graph showing the musical activity of each score instrument

    Args:
       score (maialib.Score):  A maialib Score object loaded with a valid MusicXML file

    Kwargs:
       measureStart (int): Start measure to plot
       measureEnd (int): End measure to plot
       partNames (list): A str list that contains the filtered desired score parts to plot

    Returns:
       A list: [Plotly Figure, The plot data as a Pandas Dataframe]

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

    return fig, df


def plotPianoRoll(score: mc.Score, **kwargs) -> Tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]:
    """Plots a piano roll graph showing the musical activity of each score instrument

    Args:
       score (maialib.Score):  A maialib Score object loaded with a valid MusicXML file

    Kwargs:
       measureStart (int): Start measure to plot
       measureEnd (int): End measure to plot
       partNames (list): A str list that contains the filtered desired score parts to plot

    Returns:
       A list: [Plotly Figure, The plot data as a Pandas Dataframe]

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
    fig.update_xaxes(
        type='linear',
        range=[1, None],  # Fixando limite inferior do eixo X em 1
        showgrid=True,
        gridwidth=1,
        title="Measures"
    )

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

    return fig, df


def _removeNoteOffLines(df: pd.DataFrame) -> pd.DataFrame:
    df["low"] = df["low"].map(lambda x: None if x == 0 else x)
    df["high"] = df["high"].map(lambda x: None if x == 0 else x)
    df["mean"] = df["mean"].map(lambda x: None if x == 0 else x)
    df["meanOfExtremes"] = df["meanOfExtremes"].map(
        lambda x: None if x == 0 else x)

    return df


def _scoreEnvelopeDataFrame(df: pd.DataFrame) -> pd.DataFrame:
    data = []
    for index, row in df.iterrows():
        chord = row["chord"]
        chordSize = chord.size()

        if chordSize == 0:
            obj = {
                "floatMeasure": row["floatMeasure"],
                "low": 0,
                "meanOfExtremes": 0,
                "mean": 0,
                "high": 0,
                "lowPitch": None,
                "meanOfExtremesPitch": None,
                "meanPitch": None,
                "highPitch": None
            }
        else:
            obj = {
                "floatMeasure": row["floatMeasure"],
                "low": chord.getNote(0).getMidiNumber(),
                "meanOfExtremes": chord.getMeanOfExtremesMidiValue(),
                "mean": chord.getMeanMidiValue(),
                "high": chord.getNote(chordSize-1).getMidiNumber(),
                "lowPitch": chord.getNote(0).getPitch(),
                "meanOfExtremesPitch": chord.getMeanOfExtremesPitch(),
                "meanPitch": chord.getMeanPitch(),
                "highPitch": chord.getNote(chordSize-1).getPitch()
            }

        data.append(obj)

    new_df = pd.DataFrame.from_records(data)
    return new_df


def _envelopeDataFrameInterpolation(df: pd.DataFrame, interpolatePoints: int) -> pd.DataFrame:
    def split(a, n):
        k, m = divmod(len(a), n)
        return (a[i*k+min(i, m):(i+1)*k+min(i+1, m)] for i in range(n))

    totalMeasures = int(df.floatMeasure.max())

    if interpolatePoints >= totalMeasures:
        raise Exception(
            "ERROR: The score number of measures must be greater then the interpolate points value")

    ranges = list(split(range(1, totalMeasures+1), interpolatePoints))

    data = []
    for sub in ranges:
        sub_df = df[(df.floatMeasure >=
                    float(sub.start)) & (df.floatMeasure < float(sub.stop))]
        floatMeasure = (sub.start + sub.stop) / 2
        low = round(sub_df.low.mean())
        meanOfExtremes = round(sub_df["meanOfExtremes"].mean())
        mean = round(sub_df["mean"].mean())
        high = round(sub_df.high.mean())

        obj = {
            "floatMeasure": floatMeasure,
            "low": low,
            "meanOfExtremes": meanOfExtremes,
            "mean": mean,
            "high": high,
            "lowPitch": mc.Helper.midiNote2pitch(low),
            "meanOfExtremesPitch": mc.Helper.midiNote2pitch(meanOfExtremes),
            "meanPitch": mc.Helper.midiNote2pitch(mean),
            "highPitch": mc.Helper.midiNote2pitch(high)
        }

        data.append(obj)

    new_df = pd.DataFrame.from_records(data)
    return new_df


def _chordNumNotesDataFrameInterpolation(df: pd.DataFrame, interpolatePoints: int) -> pd.DataFrame:
    def split(a, n):
        k, m = divmod(len(a), n)
        return (a[i*k+min(i, m):(i+1)*k+min(i+1, m)] for i in range(n))

    firstMeasureNumber = df.measure.min(skipna=True)
    lastMeasureNumber = df.measure.max(skipna=True)

    if interpolatePoints >= lastMeasureNumber:
        raise Exception(
            "ERROR: The score number of measures must be greater then the interpolate points value")

    ranges = list(
        split(range(firstMeasureNumber, lastMeasureNumber+1), interpolatePoints))
    data = []
    for sub in ranges:
        sub_df = df.query(f'(measure >= {sub.start}) & (measure < {sub.stop})')
        floatMeasure = (sub.start + sub.stop) / 2
        numNotes = round(sub_df["numNotes"].mean(skipna=True))

        obj = {
            "floatMeasure": floatMeasure,
            "numNotes": numNotes
        }

        data.append(obj)

    new_df = pd.DataFrame.from_records(data)
    return new_df


def plotScorePitchEnvelope(score: mc.Score, **kwargs) -> Tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]:
    """Plot a score pitch envelope

    Args:
       score (maialib.Score):  A maialib Score object loaded with a valid MusicXML file

    Kwargs:
       numPoints: (int): Number of interpolated points
       showHigher (bool): Plot the envelop upper limit
       showLower (bool): Plot the envelop lower limit
       showMean (bool): Plot the envelop mean curve
       showMeanOfExtremes (bool): Plot the envelop mean of extremes curve

    Returns:
       A list: [Plotly Figure, The plot data as a Pandas Dataframe]

    Raises:
       RuntimeError, KeyError

    Examples of use:

    >>> myScore = ml.Score("/path/to/score.xml")
    >>> plotScorePitchEnvelope(myScore)
    >>> plotScorePitchEnvelope(myScore, numPoints=10)
    >>> plotScorePitchEnvelope(myScore, showLower=False)
    >>> plotScorePitchEnvelope(myScore, showMean=False, showMean=True)
    """
    # ===== GET BASIC INFO ===== #
    df = score.getChordsDataFrame()
    df = _scoreEnvelopeDataFrame(df)
    df = _removeNoteOffLines(df)

    if "numPoints" in kwargs:
        df = _envelopeDataFrameInterpolation(df, kwargs["numPoints"])

    workTitle = score.getTitle()
    author = score.getComposerName()

    if str(author) == "":
        author = "No Author"

    if str(workTitle) == "":
        workTitle = "No Title"

    # ===== PLOT DATA ===== #
    fig = go.Figure()

    # Get mouse houver plot data
    customLow = list(df[['lowPitch']].to_numpy())
    customMeanOfExtremes = list(df[['meanOfExtremesPitch']].to_numpy())
    customMean = list(df[['meanPitch']].to_numpy())
    customHigh = list(df[['highPitch']].to_numpy())

    # ===== PLOT TRACES CONTROL ===== #
    showHigher = True
    if "showHigher" in kwargs:
        showHigher = kwargs["showHigher"]

    showLower = True
    if "showLower" in kwargs:
        showLower = kwargs["showLower"]

    showMeanOfExtremes = True
    if "showMeanOfExtremes" in kwargs:
        showMeanOfExtremes = kwargs["showMeanOfExtremes"]

    showMean = True
    if "showMean" in kwargs:
        showMean = kwargs["showMean"]

    # Create plot traces
    if showHigher:
        fig.add_trace(go.Scatter(x=df.floatMeasure, y=df.high, name='higher pitch', line_shape="spline",
                                 customdata=customHigh, hovertemplate="%{customdata[0]}", line_color="blue"))

    if showMeanOfExtremes:
        fig.add_trace(go.Scatter(x=df.floatMeasure, y=df["meanOfExtremes"], name='mean of extremes', line_shape="spline",
                                 customdata=customMeanOfExtremes, hovertemplate="%{customdata[0]}", line_color="black"))

    if showMean:
        fig.add_trace(go.Scatter(x=df.floatMeasure, y=df["mean"], name='mean', line_shape="spline",
                                 customdata=customMean, hovertemplate="%{customdata[0]}", line_color="green"))

    if showLower:
        fig.add_trace(go.Scatter(x=df.floatMeasure, y=df.low, name='lower pitch', line_shape="spline",
                                 customdata=customLow, hovertemplate="%{customdata[0]}", line_color="red"))

    # ===== PLOT LAYOUT ===== #
    fig.update_layout(
        title=f"<b>Pitchs Envelope<br>{workTitle} - {author}</b>", title_x=0.5, font={"size": 18})
    fig.update_xaxes(type='linear', autorange=True, showgrid=True,
                     gridwidth=1, title="Measures")
    fig.update_yaxes(autorange=True, showgrid=True,
                     gridwidth=1, ticksuffix="  ")
    fig.update_layout(title_x=0.5, yaxis_title=None, font={
        "size": 18,
    }, yaxis=dict(
        title='Pitch',
        tickvals=[12, 24, 36, 48, 60, 72, 84, 96, 108, 120],
        ticktext=["C0 ", "C1 ", "C2 ", "C3 ", "C4 ",
                  "C5 ", "C6 ", "C7 ", "C8 ", "C9 "],
    ))
    fig.update_layout(hovermode='x unified',
                      template="plotly_white", yaxis_showticksuffix="all")

    fig.update_layout(
        legend=dict(
            orientation="h",          # Horizontal
            yanchor="bottom",         # Ancorar a parte inferior da legenda
            # Colocar a legenda abaixo do gráfico (ajuste conforme necessário)
            y=-0.4,
            xanchor="center",         # Centralizar horizontalmente
            x=0.5                     # No meio do gráfico
        )
    )

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
    return fig, df


def plotChordsNumberOfNotes(score: mc.Score, **kwargs) -> Tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]:
    """Plot chord number of notes varying in time

    Args:
       score (maialib.Score):  A maialib Score object loaded with a valid MusicXML file

    Kwargs:
       measureStart (int): Start measure to plot
       measureEnd (int): End measure to plot
       numPoints (int): Number of interpolated points

    Returns:
       A list: [Plotly Figure, The plot data as a Pandas Dataframe]

    Raises:
       RuntimeError, KeyError

    Examples of use:

    >>> myScore = ml.Score("/path/to/score.xml")
    >>> plotChordsNumberOfNotes(myScore)
    >>> plotChordsNumberOfNotes(myScore, numPoints=15)
    >>> plotChordsNumberOfNotes(myScore, measureStart=10, measureEnd=20)
    """
    # ===== INPUT VALIDATION ===== #
    # Validade 'measureStart' and 'measureEnd'
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
    # ===== GET BASIC DATA ===== #
    df = score.getChordsDataFrame()
    df["numNotes"] = df.apply(lambda line: line.chord.size(), axis=1)
    df = df.query(f'(measure >= {measureStart}) & (measure < {measureEnd})')

    if "numPoints" in kwargs:
        df = _chordNumNotesDataFrameInterpolation(df, kwargs["numPoints"])

    df["numNotes"] = df["numNotes"].map(lambda x: None if x == 0 else x)

    # ===== COMPUTE AUX DATA ===== #
    minNumNotes = df["numNotes"].min()
    maxNumNotes = df["numNotes"].max()
    meanOfExtremesNumNotes = (minNumNotes + maxNumNotes) / 2
    meanNumNotes = df["numNotes"].sum() / df.shape[0]

    # ===== CREATE PLOT TRACES ===== #
    fig = px.line(df, x="floatMeasure", y="numNotes",
                  title='Chords number of notes')
    fig.add_hline(y=meanOfExtremesNumNotes, line_width=1, line_dash="dash", line_color="green", annotation_text="Mean of Extremes",
                  annotation_position="bottom right",
                  annotation_font_size=14,
                  annotation_font_color="green")
    fig.add_hline(y=meanNumNotes, line_width=2,
                  line_dash="solid", line_color="black", annotation_text="Mean",
                  annotation_position="bottom left",
                  annotation_font_size=14,
                  annotation_font_color="black")

    # ===== PLOT LAYOUT ===== #
    fig.update_xaxes(type='linear', autorange=True, showgrid=True,
                     gridwidth=1, title="Measures")
    fig.update_yaxes(autorange=True, showgrid=True,
                     gridwidth=1, ticksuffix="  ", title="Number of Notes", gridcolor='lightgray')
    fig.update_layout(title_x=0.5, font={
        "size": 14,
    },
        plot_bgcolor='white'
    )

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

    return fig, df

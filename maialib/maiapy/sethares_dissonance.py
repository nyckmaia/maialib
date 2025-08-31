import numpy as np
import pandas as pd
import plotly
import plotly.express as px
import plotly.graph_objects as go
from maialib import maiacore as mc
from typing import List, Tuple, Callable, Optional

__all__ = ["plotSetharesDissonanceCurve", "plotScoreSetharesDissonance",
           "plotChordDyadsSetharesDissonanceHeatmap"]


def _dissmeasure(fvec: List[float], amp: List[float], model: str = 'min') -> float:
    """
    Given a list of partials in fvec, with amplitudes in amp, this routine
    calculates the dissonance by summing the roughness of every sine pair
    based on a model of Plomp-Levelt's roughness curve.

    The older model (model='product') was based on the product of the two
    amplitudes, but the newer model (model='min') is based on the minimum
    of the two amplitudes, since this matches the beat frequency amplitude.
    """
    # Sort by frequency
    sort_idx = np.argsort(fvec)
    am_sorted = np.asarray(amp)[sort_idx]
    fr_sorted = np.asarray(fvec)[sort_idx]

    # Sum amplitudes for unique frequencies
    freq_amp_dict = {}
    for f, a in zip(fr_sorted, am_sorted):
        freq_amp_dict[f] = freq_amp_dict.get(f, 0) + a

    # Extract updated frequencies and amplitudes from the dictionary
    fr_sorted = np.array(list(freq_amp_dict.keys()))
    am_sorted = np.array(list(freq_amp_dict.values()))

    # Used to stretch dissonance curve for different freqs:
    Dstar = 0.24  # Point of maximum dissonance
    S1 = 0.0207
    S2 = 18.96

    C1 = 5
    C2 = -5

    # Plomp-Levelt roughness curve:
    A1 = -3.51
    A2 = -5.75

    # Generate all combinations of frequency components
    idx = np.transpose(np.triu_indices_from(np.eye(len(fr_sorted)), k=0))
    fr_pairs = fr_sorted[idx]
    am_pairs = am_sorted[idx]

    Fmin = fr_pairs[:, 0]
    S = Dstar / (S1 * Fmin + S2)
    Fdif = fr_pairs[:, 1] - fr_pairs[:, 0]

    if model == 'min':
        a = np.amin(am_pairs, axis=1)
    elif model == 'product':
        a = np.prod(am_pairs, axis=1)  # Older model
    else:
        raise ValueError('model should be "min" or "product"')
    SFdif = S * Fdif
    D = np.sum(a * (C1 * np.exp(A1 * SFdif) + C2 * np.exp(A2 * SFdif)))

    return D, fr_pairs, am_pairs


def plotSetharesDissonanceCurve(fundamentalFreq: float = 440, numPartials: int = 6, ratioLowLimit: float = 1.0, ratioHighLimit: float = 2.3, ratioStepIncrement: float = 0.001, amplCallback: Optional[Callable[[List[float]], List[float]]] = None, partialsDecayExpRate: float = 0.88) -> Tuple[go.Figure, pd.DataFrame]:
    """
    Generate and return the sensory dissonance curve (Sethares) for a harmonic spectrum.

    Parameters
    ----------
    fundamentalFreq : float, default=440
        Base frequency (f₀) in Hz on which the partials are built.

    numPartials : int, default=6
        Number of harmonics (partials) to include.

    ratioLowLimit : float, default=1.0
        Lower bound of the frequency ratio axis (intervals).

    ratioHighLimit : float, default=2.3
        Upper bound of the frequency ratio axis.

    ratioStepIncrement : float, default=0.001
        Step size between successive frequency ratios in the dissonance curve.

    amplCallback : Optional[Callable[[List[float]], List[float]]], default=None
        Optional function that receives a list of partial frequencies and returns
        corresponding amplitudes. If None, amplitudes decay exponentially by
        `partialsDecayExpRate`.

    partialsDecayExpRate : float, default=0.88
        Exponential decay rate for harmonics when `amplCallback` is None:
        amplitude_i = (partialsDecayExpRate)**i.

    Returns
    -------
    fig : go.Figure
        Plotly figure of the sensory dissonance curve with a log-scaled frequency ratio
        axis. Includes vertical lines for musically notable intervals (e.g., 3/2, 5/4).

    df : pandas.DataFrame
        DataFrame with columns:
            - 'ratio': frequency ratio values
            - 'dissonance': sensory dissonance computed for each ratio
            - 'freqs': frequency pair vectors used for calculation
            - 'amps': amplitude pair vectors used in calculation

    Behavior
    --------
    1. Constructs frequency vector `freqs` with integer multiples of `fundamentalFreq`.
    2. Computes amplitude vector `amps` via `amplCallback`, or using exponential decay.
    3. Validates matching lengths for `freqs` and `amps`, raising ValueError if mismatched.
    4. Constructs a `ratios` array from `ratioLowLimit` to `ratioHighLimit`.
    5. For each ratio r:
       - Concatenates `freqs` with r × `freqs`; likewise for amplitudes.
       - Applies `_dissmeasure` to compute sensory dissonance, frequency pairs, and amplitude pairs.
    6. Builds a Plotly figure plotting dissonance vs. ratio and overlays lines at common musical intervals.
    7. Returns the figure and a pandas DataFrame for further analysis.

    Exceptions
    ----------
    ValueError:
        Raised if the output of `amplCallback` (if provided) does not match `numPartials` in length.
    """
    freqs = fundamentalFreq * np.array(list(range(1, numPartials+1)))
    amps = partialsDecayExpRate**np.array(list(range(0, numPartials))
                                          ) if amplCallback == None else amplCallback(freqs)

    if len(freqs) != len(amps):
        raise ValueError(
            "The size of amplVec must be equal to the 'numPartials' (6 is the default)")

    # Calculate the number of points based on ratioStepIncrement
    numPoints = int((ratioHighLimit - ratioLowLimit) / ratioStepIncrement) + 1

    ratios = np.linspace(ratioLowLimit, ratioHighLimit, numPoints)
    dissonances = []
    fr_pairsVec = []
    amp_pairsVec = []
    for r in ratios:
        extended_freqs = np.concatenate([freqs, r * freqs])
        extended_amps = np.concatenate([amps, amps])
        d, fr_pairs, amp_pairs = _dissmeasure(extended_freqs, extended_amps)
        dissonances.append(d)
        fr_pairsVec.append(fr_pairs)
        amp_pairsVec.append(amp_pairs)

    # Plotting using Plotly
    fig = go.Figure()
    fig.add_trace(go.Scatter(x=ratios, y=dissonances,
                  mode='lines', name='Dissonance'))

    # Adding lines for the notable intervals
    intervals = [(1, 1), (6, 5), (5, 4), (4, 3), (3, 2), (5, 3),
                 (2, 1), (4, 1), (8, 1), (16, 1), (32, 1)]

    # Filter intervals based on ratioHighLimit
    filtered_intervals = [(n, d)
                          for n, d in intervals if n/d <= ratioHighLimit]

    for n, d in filtered_intervals:
        fig.add_shape(
            type="line",
            x0=n/d,
            y0=min(dissonances),
            x1=n/d,
            y1=max(dissonances),
            line=dict(color="Silver", width=1)
        )

    partialsTitle = "partial" if numPartials == 1 else "partials"
    fig.update_layout(
        title=f"<b>Sethares' Sensory Dissonance Curve</b><br><i>f<sub>0</sub>={fundamentalFreq}Hz | {numPartials} harmonic {partialsTitle}</i>",
        title_x=0.5,
        xaxis_title="Frequency Ratio",
        yaxis_title="Sensory Dissonance",
        xaxis_type="log",
        xaxis=dict(
            tickvals=[n/d for n, d in filtered_intervals],
            ticktext=['{}/{}'.format(n, d) for n, d in filtered_intervals]
        ),
        yaxis=dict(showticklabels=True),
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

    df = pd.DataFrame(data=list(zip(ratios, dissonances, fr_pairsVec,
                      amp_pairsVec)), columns=['ratio', 'dissonance', 'freqs', 'amps'])

    return fig, df


def _setharesDissonanceDataFrameInterpolation(df: pd.DataFrame, interpolatePoints: int) -> pd.DataFrame:
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
        dissonance = round(sub_df["dissonance"].mean(skipna=True))
        chordSizeMean = round(sub_df["chordSize"].mean(skipna=True))

        obj = {
            "floatMeasure": floatMeasure,
            "dissonance": dissonance,
            "chordSizeMean": chordSizeMean,
        }

        data.append(obj)

    new_df = pd.DataFrame.from_records(data)
    return new_df


def plotScoreSetharesDissonance(score: mc.Score, plotType='line', lineShape='linear', numPartialsPerNote: int = 6, useMinModel: bool = True,
                                partialsDecayExpRate: float = 0.88,
                                amplCallback: Optional[Callable[[
                                    List[float]], List[float]]] = None,
                                dissCallback: Optional[Callable[[List[float]], float]] = None, **kwargs) -> Tuple[go.Figure, pd.DataFrame]:
    """Plot 2D line graph of the Sethares Dissonance over time

    Args:
       score (maialib.Score): A maialib Score object loaded with a valid MusicXML file
       plotType (str): Can be 'line' or 'scatter'
       lineShape (str): Can be 'linear' or 'spline'
       numPartialsPerNote (int): Amount of spectral partials for each note
       useMinModel (bool): Sethares dissonance values can be computed using the 'minimal amplitude' model
                    or the 'product amplitudes' model. The 'min' model is a more recent approach
       partialsDecayExpRate (float): Partials decay exponential rate (default: 0.88)
       amplCallback: Custom user function callback to generate the amplitude of each spectrum partial
       dissCallback: Custom user function callback to receive all paired partial dissonances and computes 
                     a single total dissonance value output
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
    >>> ml.plotScoreSetharesDissonance(myScore)
    >>> ml.plotScoreSetharesDissonance(myScore, numPoints=15)
    >>> ml.plotScoreSetharesDissonance(myScore, measureStart=10, measureEnd=20)
    """
    # ===== GET THE PLOT TITLE ===== #
    workTitle = score.getTitle()
    if workTitle.strip() == "":
        workTitle = "No Title"
    plotTitle = f'<b>Sethares Sensory Dissonance</b><br><i>{workTitle}</i>'

    # ===== COMPUTE THE SETHARES DISSONANCE ===== #
    df = score.getChordsDataFrame(kwargs)

    df["dissonance"] = df.apply(lambda row: row.chord.getSetharesDissonance(
        numPartialsPerNote, useMinModel, amplCallback, partialsDecayExpRate, dissCallback), axis=1)
    df["chordNotes"] = df.apply(lambda row: ', '.join(
        [str(x.getPitch()) for x in row.chord.getNotes()]), axis=1)
    df["chordSize"] = df.apply(lambda row: row.chord.size(), axis=1)
    dissonanceMean = df.dissonance.mean()

    # ===== CHECK THE INTERPOLATION POINTS ===== #
    plotHoverData = ["chordNotes", "chordSize"]
    if "numPoints" in kwargs:
        df = _setharesDissonanceDataFrameInterpolation(df, kwargs["numPoints"])
        plotHoverData = ["chordSizeMean"]

    # ===== PLOT ===== #
    fig = None
    if plotType == 'line':
        fig = px.line(df, x="floatMeasure", y="dissonance",
                      hover_data=plotHoverData, title=plotTitle, line_shape=lineShape)
    else:
        fig = px.scatter(df, x="floatMeasure", y="dissonance",
                         hover_data=plotHoverData, title=plotTitle)

    fig.add_hline(y=dissonanceMean, line_width=3, line_dash="dash",
                  line_color="green", annotation_text="Mean")

    fig.update_layout(
        title_x=0.5,
        xaxis_title="Measures",
        yaxis_title="Sensory Dissonance",
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


def plotChordDyadsSetharesDissonanceHeatmap(chord: mc.Chord, numPartialsPerNote: int = 6, useMinModel: bool = True, amplCallback: Optional[Callable[[
    List[float]], List[float]]] = None, partialsDecayExpRate: float = 0.88, dissonanceThreshold: float = 0.1, dissonanceDecimalPoint: int = 2, showValues: bool = False,
        valuesDecimalPlaces: int = 2) -> Tuple[plotly.graph_objs._figure.Figure, pd.DataFrame]:
    """Plot chord dyads Sethares dissonance heatmap

    Args:
       chord (maialib.Chord):  A maialib Chord

    Kwargs:
       numPartialsPerNote (int): Amount of spectral partials for each note
       useMinModel (bool): Sethares dissonance values can be computed using the 'minimal amplitude' model
                    or the 'product amplitudes' model. The 'min' model is a more recent approach
       amplCallback: Custom user function callback to generate the amplitude of each spectrum partial
       partialsDecayExpRate (float): Partials decay exponential rate (default: 0.88)
       dissonanceThreshold (float): Dissonance threshold to skip small dissonance values
       dissonanceDecimalPoint (int): Round chord dissonance value in the plot title
       showValues (bool): If True, show numerical values inside heatmap cells
       valuesDecimalPlaces (int): Number of decimal places to display in cell values

    Returns:
       A list: [Plotly Figure, The plot data as a Pandas Dataframe]

    Raises:
       RuntimeError, KeyError

    Examples of use:

    >>> import maialib as ml
    >>> myChord = ml.Chord(["C3", "E3", "G3"])
    >>> fig, df = plotChordDyadsSetharesDissonanceHeatmap(myChord)
    >>> fig.show()
    """
    df = chord.getSetharesDyadsDataFrame(
        numPartialsPerNote=numPartialsPerNote, useMinModel=useMinModel, amplCallback=amplCallback, partialsDecayExpRate=partialsDecayExpRate)
    dfFiltered = df[df.dissonance > dissonanceThreshold]

    # Pivot para matriz (targetFreq como linhas, baseFreq como colunas)
    matrix_df = dfFiltered.pivot(
        index='targetFreq', columns='baseFreq', values='dissonance'
    )

    # Reordena linhas e colunas para consistência
    matrix_df = matrix_df.reindex(
        index=sorted(matrix_df.index),
        columns=sorted(matrix_df.columns)
    )

    # Índices para o heatmap uniforme
    x_ticks = list(range(len(matrix_df.columns)))
    y_ticks = list(range(len(matrix_df.index)))

    # Labels reais (frequências)
    x_labels = [round(v, 0) for v in matrix_df.columns]
    y_labels = [round(v, 0) for v in matrix_df.index]

    # Formatação do texto (caso showValues=True)
    if showValues:
        text_format = f".{valuesDecimalPlaces}f"
    else:
        text_format = False

    # Criar heatmap com quadrados iguais
    fig = px.imshow(
        matrix_df.values,
        labels=dict(x="Base Frequency (Hz)",
                    y="Target Frequency (Hz)", color="Dissonance"),
        color_continuous_scale="Inferno",
        origin="lower",   # força o eixo Y a começar por baixo
        text_auto=text_format
    )

    # Ajusta ticks para mostrar frequências reais
    fig.update_xaxes(
        tickmode="array",
        tickvals=x_ticks,
        ticktext=x_labels
    )
    fig.update_yaxes(
        tickmode="array",
        tickvals=y_ticks,
        ticktext=y_labels,
    )

    # Mantém os quadrados sempre iguais
    fig.update_yaxes(scaleanchor="x", scaleratio=1)

    # Título
    roundedDissonanceValue = round(df.dissonance.sum(), dissonanceDecimalPoint)
    fig.update_layout(
        title=f'<b>Chord Dyads Sethares Dissonance Heatmap</b><br><i>Chord Dissonance={roundedDissonanceValue}</i>',
        title_x=0.5
    )

    return fig, dfFiltered

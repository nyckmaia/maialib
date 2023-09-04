import numpy as np
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
from maialib import maiacore as mc
from typing import List, Tuple, Callable, Optional

__all__ = ["plotSetharesDissonanceCurve", "plotScoreSetharesDissonance"]

def _dissmeasure(fvec, amp, model='min') -> float:
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
    idx = np.transpose(np.triu_indices(len(fr_sorted), 1))
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

    return D

def plotSetharesDissonanceCurve(base_freq=500, numPartials=6, r_low=1, r_high=2.3, n_points=3000, amplVec=None) -> Tuple[go.Figure, List[float], List[float]]:
    """
    Compute the Sethares Dissonance Curve of a given base frequency

    Return
        A tuple that contains a Plotly figure, and the pair 'ratios' and 'dissonance' lists
    """
    freqs = base_freq * np.array(list(range(1, numPartials+1)))
    amps = 0.88**np.array(list(range(0, numPartials))) if amplVec == None else amplVec
    
    if len(freqs) != len(amps):
        raise ValueError("The size of amplVec must be equal to the 'numPartials' (6 is the default)")

    ratios = np.linspace(r_low, r_high, n_points)
    dissonances = []

    for r in ratios:
        extended_freqs = np.concatenate([freqs, r * freqs])
        extended_amps = np.concatenate([amps, amps])
        d = _dissmeasure(extended_freqs, extended_amps)
        dissonances.append(d)

    # Plotting using Plotly
    fig = go.Figure()
    fig.add_trace(go.Scatter(x=ratios, y=dissonances, mode='lines', name='Dissonance'))
        
    # Adding lines for the notable intervals
    intervals = [(1, 1), (6, 5), (5, 4), (4, 3), (3, 2), (5, 3), (2, 1)]
    for n, d in intervals:
        fig.add_shape(
            type="line",
            x0=n/d,
            y0=min(dissonances),
            x1=n/d,
            y1=max(dissonances),
            line=dict(color="Silver", width=1)
        )

    fig.update_layout(
        title="Sethares' Sensory Dissonance Curve",
        xaxis_title="Frequency Ratio",
        yaxis_title="Sensory Dissonance",
        xaxis_type="log",
        xaxis=dict(tickvals=[n/d for n, d in intervals],
                   ticktext=['{}/{}'.format(n, d) for n, d in intervals]),
        yaxis=dict(showticklabels=True)
    )
    
    return fig, ratios, dissonances

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

def plotScoreSetharesDissonance(score: mc.Score, plotType='line', lineShape='linear', numPartials: int = 6, useMinModel: bool = True, 
                                amplCallback: Optional[Callable[[List[float]], List[float]]] = None,
    dissCallback: Optional[Callable[[List[float]], float]] = None, **kwargs) -> Tuple[go.Figure, pd.DataFrame]:
    """Plot 2D line graph of the Sethares Dissonance over time

    Args:
       score (maialib.Score): A maialib Score object loaded with a valid MusicXML file
       plotType (str): Can be 'line' or 'scatter'
       lineShape (str): Can be 'linear' or 'spline'
       numPartials (int): Amount of spectral partials for each note
       useMinModel (bool): Sethares dissonance values can be computed using the 'minimal amplitude' model
                    or the 'product amplitudes' model. The 'min' model is a more recent approach
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

    df["dissonance"] = df.apply(lambda row: row.chord.getSetharesDissonance(numPartials, useMinModel, amplCallback, dissCallback), axis=1)
    df["chordNotes"] = df.apply(lambda row: ', '.join([str(x.getPitch()) for x in row.chord.getNotes()]), axis=1)
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
        fig = px.line(df, x="floatMeasure", y="dissonance", hover_data=plotHoverData, title=plotTitle, line_shape=lineShape)
    else:
        fig = px.scatter(df, x="floatMeasure", y="dissonance", hover_data=plotHoverData, title=plotTitle)

    fig.add_hline(y=dissonanceMean, line_width=3, line_dash="dash", line_color="green", annotation_text="Mean")

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

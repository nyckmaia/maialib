#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/measure.h"
#include "maiacore/score.h"
#include "nlohmann/json.hpp"
#include "pybind11_json/pybind11_json.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

void ScoreClass(const py::module& m) {
    m.doc() = "Score class binding";

    // bindings to Score class
    py::class_<Score> cls(m, "Score");

    cls.def(py::init<const std::vector<std::string>&, const int>(), py::arg("partsName"),
            py::arg("numMeasures") = 20,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def(py::init<const std::string&>(), py::arg("filePath"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Load a score from a MusicXML file (``.xml``, ``.musicxml`` or compressed ``.mxl``).

        A note's accidental is read from its ``<accidental>`` element first -- where a quarter
        tone lives -- then from the decimal ``<alter>`` value, and is natural otherwise.
        ``<alter>`` is read with a ``.`` decimal point whatever the process locale, and must be
        exactly one of the nine alters this library can spell (a multiple of 0.5 from -2 to 2).
        An ``<accidental>`` name this library cannot spell (see ``Helper.alterName2symbol``)
        prints a warning and falls back to ``<alter>``; an ``<alter>`` value it cannot spell
        (e.g. 3, the eighth tone 0.25, or 0.46, near a quarter tone but not one) prints a
        warning and leaves the note natural, never rounded to the nearest pitch. When a
        recognised ``<accidental>`` and the ``<alter>`` disagree, the ``<accidental>`` is used
        and a warning is printed. None of these aborts the load.

        A ``<transpose>`` is read in every measure. It applies to the pitched notes written after
        it, in its measure and the following ones, on the staff its ``number`` names or, without
        ``number``, on every staff of the part, until the next ``<transpose>`` for that staff;
        inside a measure document order decides, and a chord is read as a unit: a ``<transpose>``
        between the notes of a chord applies from the first note after the chord. Each such note is
        given the interval with ``<octave-change>`` folded in, 7 letters and 12 semitones per octave
        (a B-flat bass clarinet's -1, -2 and -1 make ``(-8, -14)``), and the octave doubling of
        ``<double>`` (see ``Note.getOctaveDoubling``); rests and unpitched notes are left alone. A
        ``<transpose>`` without ``<diatonic>`` is given the conventional diatonic interval of its
        ``<chromatic>`` one (see ``Note.getSoundingPitch``). One whose ``<chromatic>`` or
        ``<octave-change>`` is not a whole number is ignored, leaving the previous transposition in
        force, with a warning that starts with ``[transpose-chromatic-not-integer]`` or
        ``[transpose-octave-change-not-integer]``. A ``<diatonic>`` that does not match
        ``<chromatic>`` is replaced by the conventional diatonic interval, so that nothing sounds
        different, with a ``[transpose-pair-corrected]`` warning; for a tritone both the augmented
        fourth and the diminished fifth match, and an explicit 0 with a non-zero ``<chromatic>``
        does not. A ``<transpose>`` with which a note of its scope -- the notes it would apply to,
        up to the next ``<transpose>`` for their staff -- would have no sounding pitch (below
        ``C1b-1``, or above ``B11`` where its letter cannot spell it) is ignored for its whole
        scope, with a ``[transpose-out-of-range]`` warning; there the previous transposition stays
        in force, and a chord with a note it cannot sound either is read untransposed.
        ``<for-part>`` is not modelled: it is dropped with a ``[for-part-not-modelled]`` warning.
        Each warning names the part and the measure as the file numbers it.

        Parameters
        ----------
        filePath : str
            Path to the MusicXML file.

        Raises
        ------
        RuntimeError
            If the path is too short to name a file, the file cannot be loaded, or it lacks
            the MusicXML part and measure elements.

        Examples
        --------
        >>> score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
        >>> score.getNumParts()
        1
    )pbdoc");

    cls.def("clear", &Score::clear);
    cls.def("addPart", &Score::addPart, py::arg("partName"), py::arg("numStaves") = 1,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("removePart", &Score::removePart, py::arg("partId"));
    cls.def("addMeasure", &Score::addMeasure, py::arg("numMeasures"));
    cls.def("removeMeasure", &Score::removeMeasure, py::arg("measureStart"), py::arg("measureEnd"));

    cls.def("getPart", py::overload_cast<const int>(&Score::getPart), py::arg("partId"),
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getPart", py::overload_cast<const std::string&>(&Score::getPart), py::arg("partName"),
            py::return_value_policy::reference_internal,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getNumParts", &Score::getNumParts);
    cls.def("getNumMeasures", &Score::getNumMeasures);
    cls.def("getNumNotes", &Score::getNumNotes);
    cls.def("getPartsNames", &Score::getPartsNames);
    cls.def("getTitle", &Score::getTitle);

    cls.def("getFilePath", &Score::getFilePath);
    cls.def("getFileName", &Score::getFileName);
    cls.def("setTitle", &Score::setTitle, py::arg("scoreTitle"));

    cls.def("getComposerName", &Score::getComposerName);

    cls.def("setComposerName", &Score::setComposerName, py::arg("composerName"));

    cls.def("setKeySignature",
            py::overload_cast<const int, const bool, const int>(&Score::setKeySignature),
            py::arg("fifthCicle"), py::arg("isMajorMode") = true, py::arg("measureId") = 0);

    cls.def("setKeySignature",
            py::overload_cast<const std::string&, const int>(&Score::setKeySignature),
            py::arg("key"), py::arg("measureId") = 0);

    cls.def("setTimeSignature", &Score::setTimeSignature, py::arg("timeUpper"),
            py::arg("timeLower"), py::arg("measureId") = -1);

    cls.def("setMetronomeMark", &Score::setMetronomeMark, py::arg("bpm"),
            py::arg("rhythmFigure") = RhythmFigure::QUARTER, py::arg("measureStart") = 0);

    cls.def("haveAnacrusisMeasure", &Score::haveAnacrusisMeasure);
    cls.def("toXML", &Score::toXML, py::arg("identSize") = 2,
            R"pbdoc(
        Return the score as MusicXML text. Each part's transpositions are written as
        ``<transpose>`` elements (see ``Part.toXML``).

        Raises
        ------
        RuntimeError
            If a part cannot be written: the notes of a chord have different transposing
            intervals or octave doublings (see ``Part.toXML``).
    )pbdoc");
    cls.def("toJSON", &Score::toJSON);
    cls.def("toFile", &Score::toFile, py::arg("fileName"), py::arg("compressedXML") = false,
            py::arg("identSize") = 2,
            R"pbdoc(
        Write the score as MusicXML to ``fileName`` plus ``.xml``, or to ``fileName`` plus
        ``.mxl`` when ``compressedXML`` is True. Each part's transpositions are written as
        ``<transpose>`` elements (see ``Part.toXML``).

        Raises
        ------
        RuntimeError
            If ``fileName`` is empty, the file cannot be opened, or a part cannot be written
            (see ``Part.toXML``).
    )pbdoc");
    cls.def("info", &Score::info,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("isValid", &Score::isValid,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("xPathCountNodes", &Score::xPathCountNodes, py::arg("xPath"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getPartName", &Score::getPartName, py::arg("partId"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("getPartIndex", &Score::getPartIndex, py::arg("partName"), py::arg("index"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    //     cls.def("selectNotes", &Score::selectNotes, py::arg("config"),
    //             py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    //     cls.def("countNotes", &Score::countNotes, py::arg("config"),
    //             py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());
    cls.def("setRepeat", &Score::setRepeat, py::arg("measureStart"), py::arg("measureEnd") = -1);

    cls.def(
        "findMelodyPatternDataFrame",
        [](Score& score, const std::vector<Note>& melodyPattern,
           const float totalIntervalsSimilarityThreshold,
           const float totalRhythmSimilarityThreshold,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               intervalsSimilarityCallback,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
           const std::function<float(float, float)> totalSimilarityCallback) {
            // Import Pandas module
            py::object Pandas = py::module_::import("pandas");

            // Get method 'from_records' from 'DataFrame()' object
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

            // Set DataFrame columns name
            std::vector<std::string> columns = {"partName",
                                                "measureId",
                                                "staveId",
                                                "writtenClefKey",
                                                "transposeInterval",
                                                "segmentWrittenPitch",
                                                "semitonesDiff",
                                                "rhythmDiff",
                                                "totalIntervalSimilarity",
                                                "totalRhythmSimilarity",
                                                "totalSimilarity"};

            // Fill DataFrame with records and columns
            py::object df = FromRecords(
                score.findMelodyPattern(melodyPattern, totalIntervalsSimilarityThreshold,
                                        totalRhythmSimilarityThreshold, intervalsSimilarityCallback,
                                        rhythmSimilarityCallback, totalIntervalSimilarityCallback,
                                        totalRhythmSimilarityCallback, totalSimilarityCallback),
                "columns"_a = columns);

            return df;
        },
        py::arg("melodyPattern"), py::arg("totalIntervalsSimilarityThreshold") = 0.5f,
        py::arg("totalRhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
        R"pbdoc(
        Search the score for a melodic pattern, comparing interval contours and rhythms.

        Each part's first-voice melody -- chords and other voices are skipped -- is scanned with
        a window as long as the pattern. For each window the interval differences
        (``Helper.getSemitonesDifferenceBetweenMelodies``, where a quarter tone counts as half a
        semitone) and the duration differences are reduced to two similarities, and the window is
        a match when both reach their thresholds.

        Parameters
        ----------
        melodyPattern : list of Note
            The pattern.
        totalIntervalsSimilarityThreshold : float, default 0.5
            Minimum interval similarity of a match, from 0 to 1.
        totalRhythmSimilarityThreshold : float, default 0.5
            Minimum rhythm similarity of a match, from 0 to 1.
        intervalsSimilarityCallback : callable, optional
            ``f(pattern, segment) -> list of float``, replacing the interval differences. Give
            ``totalIntervalSimilarityCallback`` with it.
        rhythmSimilarityCallback : callable, optional
            ``f(pattern, segment) -> list of float``, replacing the duration differences. Give
            ``totalRhythmSimilarityCallback`` with it.
        totalIntervalSimilarityCallback : callable, optional
            ``f(differences) -> float``, reducing the interval differences to a similarity; used
            only with ``intervalsSimilarityCallback``.
        totalRhythmSimilarityCallback : callable, optional
            ``f(differences) -> float``, reducing the duration differences to a similarity; used
            only with ``rhythmSimilarityCallback``.
        totalSimilarityCallback : callable, optional
            ``f(intervalSimilarity, rhythmSimilarity) -> float``; the default is their mean.

        Returns
        -------
        pandas.DataFrame
            One row per match, in score order, with the columns ``partName``, ``measureId``,
            ``staveId``, ``writtenClefKey`` (the measure's key), ``transposeInterval`` (from the
            pattern's first sounding note to the segment's, at concert pitch: see ``Interval``),
            ``segmentWrittenPitch`` (the segment's pitches as its part writes them),
            ``semitonesDiff``, ``rhythmDiff``, ``totalIntervalSimilarity``,
            ``totalRhythmSimilarity`` and ``totalSimilarity``.

        Raises
        ------
        RuntimeError
            If the pattern has more notes than the score; if ``intervalsSimilarityCallback`` or
            ``rhythmSimilarityCallback`` is given without its total callback ("bad function
            call"); or if the pattern or a segment starts on a quarter tone, whose transposition
            has no interval name -- melody-pattern search does not support that, and the message
            names the note and, for a segment, its ``partName``, ``measureId`` and ``staveId``.
            A quarter tone elsewhere in a segment is compared exactly.

        Examples
        --------
        >>> score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
        >>> pattern = [ml.Note("G2"), ml.Note("D3"), ml.Note("B3")]
        >>> table = score.findMelodyPatternDataFrame(pattern, 1.0, 1.0)
        >>> len(table) > 0, float(table["totalSimilarity"].min())
        (True, 1.0)
    )pbdoc");

    // Overload para multiplos padrões em paralelo
    cls.def(
        "findMelodyPatternDataFrame",
        [](Score& score, const std::vector<std::vector<Note>>& melodyPatterns,
           float totalIntervalsSimilarityThreshold, float totalRhythmSimilarityThreshold,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               intervalsSimilarityCallback,
           const std::function<std::vector<float>(const std::vector<Note>&,
                                                  const std::vector<Note>&)>
               rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
           const std::function<float(float, float)> totalSimilarityCallback) {
            // The search runs each pattern on a worker thread, and a worker that calls, copies or
            // destroys a Python callback takes the GIL to do it. Holding the GIL here while the
            // workers run would deadlock, so it is released for the search alone, inside this
            // lambda; it is held again when the lambda returns, before any DataFrame is built.
            // The search only reads the score, so other threads may search it meanwhile.
            const auto results = [&] {
                py::gil_scoped_release release;
                return score.findMelodyPattern(
                    melodyPatterns, totalIntervalsSimilarityThreshold,
                    totalRhythmSimilarityThreshold, intervalsSimilarityCallback,
                    rhythmSimilarityCallback, totalIntervalSimilarityCallback,
                    totalRhythmSimilarityCallback, totalSimilarityCallback);
            }();

            py::object Pandas = py::module_::import("pandas");
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");
            std::vector<py::object> dataframes;

            // Definindo as colunas do DataFrame
            std::vector<std::string> columns = {"partName",
                                                "measureId",
                                                "staveId",
                                                "writtenClefKey",
                                                "transposeInterval",
                                                "segmentWrittenPitch",
                                                "semitonesDiff",
                                                "rhythmDiff",
                                                "totalIntervalSimilarity",
                                                "totalRhythmSimilarity",
                                                "totalSimilarity"};

            for (size_t idx = 0; idx < results.size(); ++idx) {
                py::object df = FromRecords(results[idx], "columns"_a = columns);
                int num_columns = df.attr("shape").cast<py::tuple>()[1].cast<int>();
                df.attr("insert")(num_columns, "patternIdx", idx);
                dataframes.push_back(df);
            }

            // Concatena todos os DataFrames processados com sucesso
            if (!dataframes.empty()) {
                // std::cout << "Concatenando DataFrames..." << std::endl;
                py::object result_df = Pandas.attr("concat")(dataframes, "ignore_index"_a = true);
                result_df.attr("sort_values")("by"_a = "measureId", "ascending"_a = true,
                                              "inplace"_a = true);
                return result_df;
            } else {
                throw std::runtime_error(
                    "Nenhum DataFrame foi concatenado devido a erro de memória ou outro problema.");
            }
        },
        py::arg("melodyPatterns"), py::arg("intervalSimilarityThreshold") = 0.5f,
        py::arg("rhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        R"pbdoc(
        Search the score for several melodic patterns, each on a worker thread.

        Every pattern is searched exactly as the single-pattern overload searches it, however
        many patterns there are. Takes the same thresholds and callbacks, applied to every
        pattern. The search releases the GIL while it runs, so a Python callback is called from
        the worker threads, one call at a time, each taking the GIL, and other Python threads run
        meanwhile. The search only reads the score, so any number of threads may search the same
        score at once; no thread may modify the score, its parts, measures or notes while a
        search of it runs.

        Parameters
        ----------
        melodyPatterns : list of list of Note
            The patterns.
        intervalSimilarityThreshold : float, default 0.5
            Minimum interval similarity of a match, from 0 to 1.
        rhythmSimilarityThreshold : float, default 0.5
            Minimum rhythm similarity of a match, from 0 to 1.
        intervalsSimilarityCallback : callable, optional
        rhythmSimilarityCallback : callable, optional
        totalIntervalSimilarityCallback : callable, optional
        totalRhythmSimilarityCallback : callable, optional
        totalSimilarityCallback : callable, optional
            The callbacks of the single-pattern overload, with the same pairing rules.

        Returns
        -------
        pandas.DataFrame
            The matches of every pattern, with the single-pattern overload's columns plus
            ``patternIdx``, the pattern's index in ``melodyPatterns``, sorted by ``measureId``.

        Raises
        ------
        RuntimeError
            If the search for any pattern raises, for the reasons the single-pattern overload
            gives: the first such error, in pattern order, once every pattern has been searched
            -- never an empty result in its place.

        Examples
        --------
        >>> score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
        >>> patterns = [[ml.Note("G2"), ml.Note("D3")], [ml.Note("D3"), ml.Note("B3")]]
        >>> table = score.findMelodyPatternDataFrame(patterns, 1.0, 1.0)
        >>> sorted(table["patternIdx"].unique().tolist())
        [0, 1]
    )pbdoc");

    // cls.def(
    // "findAnyMelodyPatternDataFrame",
    // [](Score& score, const int patternNumNotes,
    //    float totalIntervalsSimilarityThreshold, float totalRhythmSimilarityThreshold,
    //    const std::function<std::vector<float>(const std::vector<Note>&,
    //                                           const std::vector<Note>&)>
    //                                           intervalsSimilarityCallback,
    //    const std::function<std::vector<float>(const std::vector<Note>&,
    //                                           const std::vector<Note>&)>
    //                                           rhythmSimilarityCallback,
    //    const std::function<float(const std::vector<float>&)> totalIntervalSimilarityCallback,
    //    const std::function<float(const std::vector<float>&)> totalRhythmSimilarityCallback,
    //    const std::function<float(float, float)> totalSimilarityCallback) {

    //     const auto& results = score.findAnyMelodyPattern(patternNumNotes,
    //     totalIntervalsSimilarityThreshold,
    //                                 totalRhythmSimilarityThreshold, intervalsSimilarityCallback,
    //                                 rhythmSimilarityCallback, totalIntervalSimilarityCallback,
    //                                 totalRhythmSimilarityCallback, totalSimilarityCallback);

    //     // Converte os resultados para DataFrames no contexto principal (com o GIL adquirido)
    //     py::gil_scoped_acquire acquire;
    //     py::object Pandas = py::module_::import("pandas");
    //     py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");
    //     std::vector<py::object> dataframes;

    //     // Definindo as colunas do DataFrame
    //     std::vector<std::string> columns = {"partName",
    //                                         "measureId",
    //                                         "staveId",
    //                                         "writtenClefKey",
    //                                         "transposeInterval",
    //                                         "segmentWrittenPitch",
    //                                         "semitonesDiff",
    //                                         "rhythmDiff",
    //                                         "totalIntervalSimilarity",
    //                                         "totalRhythmSimilarity",
    //                                         "totalSimilarity"};

    //     for (size_t idx = 0; idx < results.size(); ++idx) {
    //         py::object df = FromRecords(results[idx], "columns"_a = columns);

    //         // Verifica se o DataFrame possui dados antes de adicioná-lo
    //         if (df.attr("empty").cast<bool>()) {
    //             continue;  // Pula DataFrames vazios
    //         }

    //         // Adiciona coluna "patternIdx" para identificar o índice do padrão
    //         // int num_columns = df.attr("shape").cast<py::tuple>()[1].cast<int>();
    //         df.attr("insert")(0, "patternIdx", idx);

    //         dataframes.push_back(df);
    //     }

    //     // Concatena todos os DataFrames processados com sucesso
    //     if (!dataframes.empty()) {
    //         py::object result_df = Pandas.attr("concat")(dataframes, "ignore_index"_a = true);

    //         // Ordena o DataFrame pelo campo "measureId" antes de aplicar filtros
    //         // result_df.attr("sort_values")("by"_a = "measureId", "ascending"_a = true,
    //         "inplace"_a = true);

    //         py::list sort_cols;
    //         sort_cols.append("patternIdx");
    //         sort_cols.append("measureId");
    //         sort_cols.append("partName");

    //         py::list ascending;
    //         ascending.append(true);
    //         ascending.append(true);
    //         ascending.append(true);

    //         result_df.attr("sort_values")(
    //             "by"_a = sort_cols,
    //             "ascending"_a = ascending,
    //             "inplace"_a = true
    //         );

    //         // Filtra as linhas onde "segmentWrittenPitch" contém apenas "rest"
    //         py::object filtered_df = result_df.attr("loc")[
    //             result_df.attr("segmentWrittenPitch").attr("apply")(
    //                 py::cpp_function([](const py::object& pitchList) {
    //                     auto list = pitchList.cast<std::vector<std::string>>();
    //                     return std::any_of(list.begin(), list.end(), [](const std::string& s) {
    //                     return s != "rest"; });
    //                 })
    //             )
    //         ];

    //         // Reseta o índice do DataFrame final após o filtro
    //         filtered_df = filtered_df.attr("reset_index")("drop"_a = true);

    //         return filtered_df;
    //     } else {
    //         throw std::runtime_error("Nenhum DataFrame foi concatenado devido a erro de memória
    //         ou outro problema.");
    //     }
    // },
    //     py::arg("patternNumNotes") = 5,
    //     py::arg("intervalSimilarityThreshold") = 1.0f,
    //     py::arg("rhythmSimilarityThreshold") = 1.0f,
    //     py::arg("intervalsSimilarityCallback") = nullptr,
    //     py::arg("rhythmSimilarityCallback") = nullptr,
    //     py::arg("totalIntervalSimilarityCallback") = nullptr,
    //     py::arg("totalRhythmSimilarityCallback") = nullptr,
    //     py::arg("totalSimilarityCallback") = nullptr
    // );

    cls.def("getChords", &Score::getChords, py::arg("config") = nlohmann::json(),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Get the chords of the score: the notes that sound together at each onset.

        Each onset of a note (grace notes excluded) in the selected parts and measures gives a
        chord of every note sounding at that moment -- the notes that start there and those
        still sounding from before -- sorted from the lowest sounding position up. The chord
        holds the parts' own notes: a note of a transposing instrument keeps its written pitch
        and its transposing interval, and the chord's analysis (``getName``, ``getRoot``, ...)
        relates it at concert pitch, spelled with its written letter moved by the diatonic
        transposing interval -- inferred from the chromatic one when it is 0 -- or, where that
        gives no spelling, by the fallback described on
        ``Note.getSoundingPitch``, without the simplification. The notes that analysis returns
        are untransposed notes at that pitch (see ``Chord``).

        Parameters
        ----------
        config : dict, optional
            ``partNames`` (list of str, default every part); ``measureStart`` (int, the
            zero-based index of the first measure, default 0); ``measureEnd`` (int, the
            zero-based index one past the last measure, default the score's length);
            ``includeDuplicates`` (bool, default False, which removes duplicates as
            ``Chord.removeDuplicateNotes`` does: the notes are sorted, then each note spelled, at
            concert pitch, exactly as the note before it is removed); ``includeUnpitched``
            (bool, default False, which skips unpitched parts).

        Returns
        -------
        list of tuple of (int, float, Key, Chord, bool)
            One ``(measure, floatMeasure, key, chord, isHomophonic)`` per onset, in time order:
            the one-based measure number; the onset as a one-based measure position, whose
            fraction is the position within the measure (``1.5`` is halfway through the first
            measure); the key of that measure in the score's first part, as that part writes it,
            even when ``partNames`` leaves the part out; the chord; and whether every note of
            the chord starts at that onset.

        Raises
        ------
        RuntimeError
            If a config value is invalid (a part name the score does not have, a negative
            measure index, ``measureStart`` after ``measureEnd``), or if a note sounds below
            ``C1b-1`` (see ``Note.getSoundingPitch``).

        Examples
        --------
        >>> score = ml.Score(["Clarinet in Bb", "Violin"], 1)
        >>> clarinet = ml.Note("F#4", transposeDiatonic=-1, transposeChromatic=-2)
        >>> score.getPart(0).getMeasure(0).addNote(clarinet)
        >>> score.getPart(1).getMeasure(0).addNote(ml.Note("C4"))
        >>> measure, floatMeasure, key, chord, isHomophonic = score.getChords()[0]
        >>> chord, [note.getPitch() for note in chord.getNotes()], isHomophonic
        (<Chord [C4, E4]>, ['C4', 'F#4'], True)
    )pbdoc");
    cls.def(
        "getChordsDataFrame",
        [](Score& score, nlohmann::json config) {
            // Import Pandas module
            py::object Pandas = py::module_::import("pandas");

            // Get method 'from_records' from 'DataFrame()' object
            py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

            // Set DataFrame columns name
            std::vector<std::string> columns = {"measure", "floatMeasure", "key", "chord",
                                                "isHomophonic"};

            // Fill DataFrame with records and columns
            py::object df = FromRecords(score.getChords(config), "columns"_a = columns);

            return df;
        },
        py::arg("config") = nlohmann::json(),
        py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
        R"pbdoc(
        Get the chords of the score as a table, one row per ``getChords`` tuple.

        Parameters
        ----------
        config : dict, optional
            As for ``getChords``.

        Returns
        -------
        pandas.DataFrame
            The columns ``measure``, ``floatMeasure``, ``key``, ``chord`` and ``isHomophonic``,
            in the order and with the meaning of ``getChords``' tuples. A chord holds the parts'
            own notes, and its analysis relates a note of a transposing instrument at concert
            pitch, returning untransposed notes at that pitch (see ``getChords``).

        Raises
        ------
        RuntimeError
            As for ``getChords``.

        Examples
        --------
        >>> score = ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1))
        >>> table = score.getChordsDataFrame({"measureEnd": 1})
        >>> list(table.columns)
        ['measure', 'floatMeasure', 'key', 'chord', 'isHomophonic']
    )pbdoc");

    cls.def("forEachNote", &Score::forEachNote, py::arg("callback"), py::arg("measureStart") = 0,
            py::arg("measureEnd") = -1, py::arg("partNames") = std::vector<std::string>(),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>());

    cls.def("toDataFrame", [](Score& score) {
        // Import Pandas module
        py::object Pandas = py::module_::import("pandas");

        // Get method 'from_records' from 'DataFrame()' object
        py::object FromRecords = Pandas.attr("DataFrame").attr("from_records");

        // Get basic score data
        const int numParts = score.getNumParts();
        const int numMeasures = score.getNumMeasures();
        const int numNotes = score.getNumNotes();

        // Create a temp store object: [PartObj][MeasureObj][NoteObj]
        typedef std::tuple<Part*, Measure*, Note*> DataFrameRow;
        std::vector<DataFrameRow> df_records(numNotes);

        // For each note inside the score
        int rowNumber = 0;
        for (int p = 0; p < numParts; p++) {
            Part& currentPart = score.getPart(p);

            for (int m = 0; m < numMeasures; m++) {
                Measure& currentMeasure = currentPart.getMeasure(m);

                for (int s = 0; s < currentMeasure.getNumStaves(); s++) {
                    const int numNotes = currentMeasure.getNumNotes(s);

                    for (int n = 0; n < numNotes; n++) {
                        Note& currentNote = currentMeasure.getNote(n, s);

                        df_records[rowNumber] =
                            DataFrameRow(&currentPart, &currentMeasure, &currentNote);

                        rowNumber++;
                    }
                }
            }
        }

        // Set DataFrame columns name
        std::vector<std::string> columns = {"Part", "Measure", "Note"};

        // Fill DataFrame with records and columns
        py::object df = FromRecords(df_records, "columns"_a = columns);

        return df;
    });

    // Default Python 'print' function:
    cls.def("__repr__", [](const Score& score) {
        const std::string title = (score.getTitle().empty()) ? "No Title" : score.getTitle();
        return "<Score '" + title + "'>";
    });

    cls.def("__hash__", [](const Score& score) { return std::hash<std::string>{}(score.toXML()); });

    cls.def("__sizeof__", [](const Score& score) { return sizeof(score); });
}

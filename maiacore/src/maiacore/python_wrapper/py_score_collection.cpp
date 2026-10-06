#include <pybind11/functional.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/score_collection.h"
#include "py_melody_dataframe.h"

namespace py = pybind11;
using namespace pybind11::literals;
using maiacore_python::MelodyDataFrame;

void ScoreCollectionClass(const py::module& m) {
    m.doc() = "ScoreCollection class binding";

    // bindings to ScoreCollection class
    py::class_<ScoreCollection> cls(m, "ScoreCollection");
    cls.def(py::init<>(), R"pbdoc(
        Create an empty collection: no directory and no score.
    )pbdoc");

    cls.def(py::init<const std::string&, const bool>(), py::arg("directoryPath"),
            py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Create a collection of the MusicXML files of a directory, as ``setDirectoriesPaths``
        loads them.

        Parameters
        ----------
        directoryPath : str
            The directory.
        recursive : bool, default False
            True to load the files of its subdirectories, at any depth, too.

        Raises
        ------
        RuntimeError
            As ``setDirectoriesPaths`` raises it.
    )pbdoc");

    cls.def(py::init<const std::vector<std::string>&, const bool>(), py::arg("directoriesPaths"),
            py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Create a collection of the MusicXML files of several directories, as
        ``setDirectoriesPaths`` loads them.

        Parameters
        ----------
        directoriesPaths : list of str
            The directories; ``[]`` gives an empty collection.
        recursive : bool, default False
            True to load the files of their subdirectories, at any depth, too.

        Raises
        ------
        RuntimeError
            As ``setDirectoriesPaths`` raises it.
    )pbdoc");

    cls.def("getDirectoriesPaths", &ScoreCollection::getDirectoriesPaths);
    cls.def("setDirectoriesPaths", &ScoreCollection::setDirectoriesPaths,
            py::arg("directoriesPaths"), py::arg("recursive") = false,
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Replace the collection's directories and scores with those of the given directories.

        Every file whose extension is ``.xml``, ``.mxl`` or ``.musicxml``, compared without
        regard to case, is loaded, directory by directory in the given order and, within a
        directory, in sorted path order; subdirectories only when ``recursive`` is True. Every
        path is checked before anything is loaded, and the collection changes only when every
        file has loaded. Scores added with ``addScore`` are replaced too. A subdirectory the user
        has no permission to read is skipped.

        Parameters
        ----------
        directoriesPaths : list of str
            The directories; ``[]`` empties the collection.
        recursive : bool, default False
            True to load the files of their subdirectories, at any depth, too.

        Raises
        ------
        RuntimeError
            If a path does not exist or is not a directory, or a directory cannot be read: the
            message names the path that failed, a given path or one of its subdirectories. If a
            file fails to load: the message is the file's path, ``": "`` and the message of
            the load's error. The collection is unchanged.
    )pbdoc");

    cls.def("addDirectory", &ScoreCollection::addDirectory, py::arg("directoryPath"));

    cls.def("addScore", py::overload_cast<const Score&>(&ScoreCollection::addScore),
            py::arg("score"));
    cls.def("addScore", py::overload_cast<const std::string&>(&ScoreCollection::addScore),
            py::arg("filePath"));
    cls.def("addScore",
            py::overload_cast<const std::vector<std::string>&>(&ScoreCollection::addScore),
            py::arg("filePaths"));

    cls.def("clear", &ScoreCollection::clear);
    cls.def("getNumDirectories", &ScoreCollection::getNumDirectories);
    cls.def("getNumScores", &ScoreCollection::getNumScores);

    cls.def("getScores", py::overload_cast<>(&ScoreCollection::getScores),
            py::return_value_policy::reference_internal);
    cls.def("getScores", py::overload_cast<>(&ScoreCollection::getScores, py::const_),
            py::return_value_policy::reference_internal);

    cls.def("isEmpty", &ScoreCollection::isEmpty);
    cls.def("merge", &ScoreCollection::merge, py::arg("other"));
    cls.def("removeScore", &ScoreCollection::removeScore, py::arg("scoreIdx"),
            py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(),
            R"pbdoc(
        Remove the score at an index of the collection.

        Parameters
        ----------
        scoreIdx : int
            Index of the score, from 0 to ``getNumScores() - 1``.

        Raises
        ------
        IndexError
            If ``scoreIdx`` is negative or not below ``getNumScores()``; the collection is
            unchanged.
    )pbdoc");

    cls.def(
        "findMelodyPatternDataFrame",
        [](const ScoreCollection& collection, const std::vector<Note>& melodyPattern,
           const float intervalSimilarityThreshold, const float rhythmSimilarityThreshold,
           const std::function<std::vector<float>(
               const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback,
           const std::function<std::vector<float>(
               const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
           const std::function<float(float, float)>& totalSimilarityCallback) {
            const ScoreCollection::MelodyPatternTable table = collection.findMelodyPattern(
                melodyPattern, intervalSimilarityThreshold, rhythmSimilarityThreshold,
                intervalsSimilarityCallback, rhythmSimilarityCallback,
                totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                totalSimilarityCallback);
            MelodyDataFrame frame({{"fileName", MelodyDataFrame::Kind::Text},
                                   {"composerName", MelodyDataFrame::Kind::Text},
                                   {"scoreTitle", MelodyDataFrame::Kind::Text}});
            for (const ScoreCollection::MelodyPatternRow& row : table) {
                frame.appendRow(
                    py::list(py::make_tuple(row.fileName, row.composerName, row.scoreTitle)),
                    row.match);
            }
            return frame.build();
        },
        py::arg("melodyPattern"), py::arg("intervalSimilarityThreshold") = 0.5f,
        py::arg("rhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        R"pbdoc(
        Search every score of the collection for a melodic pattern.

        Each score is searched as ``Score.findMelodyPatternDataFrame(melodyPattern, ...)``
        searches it, with the same thresholds and callbacks.

        Parameters
        ----------
        melodyPattern : list of Note
            The pattern: at least 2 notes.
        intervalSimilarityThreshold : float, default 0.5
            Minimum interval similarity of a match, from 0 to 1.
        rhythmSimilarityThreshold : float, default 0.5
            Minimum rhythm similarity of a match, from 0 to 1.
        intervalsSimilarityCallback : callable, optional
        rhythmSimilarityCallback : callable, optional
        totalIntervalSimilarityCallback : callable, optional
        totalRhythmSimilarityCallback : callable, optional
        totalSimilarityCallback : callable, optional
            The callbacks of ``Score.findMelodyPatternDataFrame``, with the same pairing rules.

        Returns
        -------
        pandas.DataFrame
            The matches in every score: ``fileName``, ``composerName`` and ``scoreTitle`` (str),
            followed by ``Score.findMelodyPatternDataFrame``'s columns; sorted stably by
            ``scoreTitle``, each score's matches in the order the score gives them. An empty
            DataFrame, with every column and its dtype, when nothing matches or the collection
            is empty.

        Raises
        ------
        RuntimeError
            If the pattern has fewer than 2 notes, even for an empty collection, or if the
            search of any score raises, for the reasons ``Score.findMelodyPatternDataFrame``
            gives.

        Examples
        --------
        >>> collection = ml.ScoreCollection()
        >>> collection.addScore(ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1)))
        >>> table = collection.findMelodyPatternDataFrame([ml.Note("G2"), ml.Note("D3")], 1.0, 1.0)
        >>> list(table.columns[:4])
        ['fileName', 'composerName', 'scoreTitle', 'partName']
    )pbdoc");

    cls.def(
        "findMelodyPatternDataFrame",
        [](const ScoreCollection& collection, const std::vector<std::vector<Note>>& melodyPatterns,
           const float intervalSimilarityThreshold, const float rhythmSimilarityThreshold,
           const std::function<std::vector<float>(
               const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback,
           const std::function<std::vector<float>(
               const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback,
           const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
           const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
           const std::function<float(float, float)>& totalSimilarityCallback) {
            // Each score's search runs its patterns on worker threads, and a worker that calls,
            // copies or destroys a Python callback takes the GIL to do it. Holding the GIL here
            // while the workers run would deadlock, so it is released for the search alone, inside
            // this lambda; it is held again when the lambda returns, before the DataFrame is
            // built. The search only reads the collection and its scores, so other threads may
            // search them meanwhile.
            const auto tables = [&] {
                py::gil_scoped_release release;
                return collection.findMelodyPattern(
                    melodyPatterns, intervalSimilarityThreshold, rhythmSimilarityThreshold,
                    intervalsSimilarityCallback, rhythmSimilarityCallback,
                    totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                    totalSimilarityCallback);
            }();

            // Every row with its pattern's index, in pattern order, then sorted stably by score
            // title: rows of one title keep the pattern order and each table's order.
            std::vector<std::pair<size_t, const ScoreCollection::MelodyPatternRow*>> rows;
            for (size_t idx = 0; idx < tables.size(); idx++) {
                for (const ScoreCollection::MelodyPatternRow& row : tables[idx]) {
                    rows.emplace_back(idx, &row);
                }
            }
            std::stable_sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
                return a.second->scoreTitle < b.second->scoreTitle;
            });

            MelodyDataFrame frame({{"patternIdx", MelodyDataFrame::Kind::Integer},
                                   {"fileName", MelodyDataFrame::Kind::Text},
                                   {"composerName", MelodyDataFrame::Kind::Text},
                                   {"scoreTitle", MelodyDataFrame::Kind::Text}});
            for (const auto& [idx, row] : rows) {
                frame.appendRow(py::list(py::make_tuple(idx, row->fileName, row->composerName,
                                                        row->scoreTitle)),
                                row->match);
            }
            return frame.build();
        },
        py::arg("melodyPatterns"), py::arg("intervalSimilarityThreshold") = 0.5f,
        py::arg("rhythmSimilarityThreshold") = 0.5f,
        py::arg("intervalsSimilarityCallback") = nullptr,
        py::arg("rhythmSimilarityCallback") = nullptr,
        py::arg("totalIntervalSimilarityCallback") = nullptr,
        py::arg("totalRhythmSimilarityCallback") = nullptr,
        py::arg("totalSimilarityCallback") = nullptr,
        R"pbdoc(
        Search every score of the collection for several melodic patterns.

        Each score is searched as ``Score.findMelodyPatternDataFrame(melodyPatterns, ...)``
        searches it: every pattern, each on a worker thread, with the same thresholds and
        callbacks. The search releases the GIL while it runs, so a Python callback is called
        from the worker threads, one call at a time, each taking the GIL, and other Python
        threads run meanwhile. The search only reads the collection and its scores, so any
        number of threads may search them at once; no thread may modify the collection, its
        scores, or their parts, measures or notes while a search of them runs.

        Parameters
        ----------
        melodyPatterns : list of list of Note
            The patterns, each of at least 2 notes.
        intervalSimilarityThreshold : float, default 0.5
            Minimum interval similarity of a match, from 0 to 1.
        rhythmSimilarityThreshold : float, default 0.5
            Minimum rhythm similarity of a match, from 0 to 1.
        intervalsSimilarityCallback : callable, optional
        rhythmSimilarityCallback : callable, optional
        totalIntervalSimilarityCallback : callable, optional
        totalRhythmSimilarityCallback : callable, optional
        totalSimilarityCallback : callable, optional
            The callbacks of ``Score.findMelodyPatternDataFrame``, with the same pairing rules.

        Returns
        -------
        pandas.DataFrame
            The matches of every pattern in every score: ``patternIdx`` (int, the pattern's
            index in ``melodyPatterns``), ``fileName``, ``composerName`` and ``scoreTitle``
            (str), followed by ``Score.findMelodyPatternDataFrame``'s columns; sorted stably by
            ``scoreTitle``, then ``patternIdx``, each score's matches in the order the score
            gives them. An empty DataFrame, with every column and its dtype, when nothing
            matches or the collection is empty.

        Raises
        ------
        RuntimeError
            If a pattern has fewer than 2 notes, even for an empty collection, or if the search
            for any pattern in any score raises, for the reasons
            ``Score.findMelodyPatternDataFrame`` gives, never leaving an empty result in its
            place.

        Examples
        --------
        >>> collection = ml.ScoreCollection()
        >>> collection.addScore(ml.Score(ml.getSampleScorePath(ml.SampleScore.Bach_Cello_Suite_1)))
        >>> patterns = [[ml.Note("G2"), ml.Note("D3")], [ml.Note("D3"), ml.Note("B3")]]
        >>> table = collection.findMelodyPatternDataFrame(patterns, 1.0, 1.0)
        >>> sorted(table["patternIdx"].unique().tolist())
        [0, 1]
    )pbdoc");

    // Default Python 'print' function:
    cls.def("__repr__", [](const ScoreCollection& scoreCollection) {
        return "<ScoreCollection - " + std::to_string(scoreCollection.getNumScores()) + " scores>";
    });

    cls.def("__hash__", [](const ScoreCollection& scoreCollection) {
        std::string temp;
        for (const auto& dir : scoreCollection.getDirectoriesPaths()) {
            temp += dir;
        }
        return std::hash<std::string>{}(temp);
    });

    cls.def("__sizeof__",
            [](const ScoreCollection& scoreCollection) { return sizeof(scoreCollection); });

    cls.def(py::self + py::self);
}

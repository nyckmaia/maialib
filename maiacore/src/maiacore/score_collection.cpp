#include "maiacore/score_collection.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "maiacore/log.h"
#include "melodic-lines.h"

using maiacore::detail::requireTwoNotes;

namespace {
// Whether the file's extension is .xml, .mxl or .musicxml, in any case. The extension is read as
// UTF-8, which never fails, whatever characters the rest of the name holds.
bool isMusicXMLFile(const std::filesystem::path& path) {
    std::string extension = path.extension().u8string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".xml" || extension == ".mxl" || extension == ".musicxml";
}

// The MusicXML files of 'directory' (and of its subdirectories at any depth when 'recursive'),
// in sorted path order. Every failure is reported in English, naming the path: the messages of
// std::filesystem's own exceptions are localised, and Python cannot always decode them.
template <typename Iterator>
std::vector<std::filesystem::path> musicXMLFilesOf(const std::string& directory) {
    std::vector<std::filesystem::path> files;
    std::error_code error;
    for (Iterator it(directory, error); !error && it != Iterator(); it.increment(error)) {
        std::error_code typeError;
        if (it->is_regular_file(typeError) && isMusicXMLFile(it->path())) {
            files.push_back(it->path());
        }
    }
    if (error) {
        LOG_ERROR("ScoreCollection: cannot read the directory '" + directory + "'");
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::vector<std::filesystem::path> musicXMLFiles(const std::string& directory,
                                                 const bool recursive) {
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
        LOG_ERROR("ScoreCollection: '" + directory + "' is not a directory, or does not exist");
    }
    return recursive ? musicXMLFilesOf<std::filesystem::recursive_directory_iterator>(directory)
                     : musicXMLFilesOf<std::filesystem::directory_iterator>(directory);
}

// The rows of 'table', a search of 'score', with the score's file name, composer and title.
void appendRows(const Score& score, const Score::MelodyPatternTable& table,
                ScoreCollection::MelodyPatternTable* rows) {
    for (const Score::MelodyPatternRow& match : table) {
        rows->push_back({score.getFileName(), score.getComposerName(), score.getTitle(), match});
    }
}

void sortByScoreTitle(ScoreCollection::MelodyPatternTable* rows) {
    std::stable_sort(
        rows->begin(), rows->end(),
        [](const ScoreCollection::MelodyPatternRow& a, const ScoreCollection::MelodyPatternRow& b) {
            return a.scoreTitle < b.scoreTitle;
        });
}
}  // namespace

ScoreCollection::ScoreCollection() = default;

ScoreCollection::ScoreCollection(const std::string& directoryPath, const bool recursive) {
    setDirectoriesPaths({directoryPath}, recursive);
}

ScoreCollection::ScoreCollection(const std::vector<std::string>& directoriesPaths,
                                 const bool recursive) {
    setDirectoriesPaths(directoriesPaths, recursive);
}

std::vector<std::string> ScoreCollection::getDirectoriesPaths() const { return _directoriesPaths; }

void ScoreCollection::setDirectoriesPaths(const std::vector<std::string>& directoriesPaths,
                                          const bool recursive) {
    std::vector<std::vector<std::filesystem::path>> files;
    files.reserve(directoriesPaths.size());
    for (const std::string& directory : directoriesPaths) {
        files.push_back(musicXMLFiles(directory, recursive));
    }

    std::vector<Score> scores;
    for (const auto& directoryFiles : files) {
        for (const std::filesystem::path& file : directoryFiles) {
            LOG_INFO("Loading: " << file.filename().string());
            scores.emplace_back(file.string());
        }
    }
    _directoriesPaths = directoriesPaths;
    _scores = std::move(scores);
}

void ScoreCollection::addDirectory(const std::string& directoryPath) {
    _directoriesPaths.push_back(directoryPath);
}

void ScoreCollection::addScore(const Score& score) { _scores.push_back(score); }

void ScoreCollection::addScore(const std::string& filePath) { addScore(Score(filePath)); }

void ScoreCollection::addScore(const std::vector<std::string>& filePaths) {
    for (const auto& fp : filePaths) {
        addScore(fp);
    }
}

void ScoreCollection::clear() { _scores.clear(); }

int ScoreCollection::getNumDirectories() const {
    return static_cast<int>(_directoriesPaths.size());
}

int ScoreCollection::getNumScores() const { return _scores.size(); }

std::vector<Score>& ScoreCollection::getScores() { return _scores; }

const std::vector<Score>& ScoreCollection::getScores() const { return _scores; }

bool ScoreCollection::isEmpty() const { return _scores.empty(); }

void ScoreCollection::merge(const ScoreCollection& other) {
    // Detect self-merge to avoid iterator invalidation (undefined behavior)
    if (this == &other) {
        ScoreCollection copy = other;  // Make a copy
        merge(copy);                   // Merge with the copy
        return;
    }

    // Merge directories paths
    for (const auto& dir : other.getDirectoriesPaths()) {
        _directoriesPaths.push_back(dir);
    }

    // Merge Score objects
    for (const auto& sc : other.getScores()) {
        _scores.push_back(sc);
    }
}

void ScoreCollection::removeScore(const int scoreIdx) {
    if (scoreIdx < 0 || scoreIdx >= static_cast<int>(_scores.size())) {
        throw std::out_of_range("ScoreCollection::removeScore: index " + std::to_string(scoreIdx) +
                                " is outside the collection of " + std::to_string(_scores.size()) +
                                " scores");
    }

    _scores.erase(_scores.begin() + scoreIdx);
}

ScoreCollection::MelodyPatternTable ScoreCollection::findMelodyPattern(
    const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold,
    const float rhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
        intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
        rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
    const std::function<float(float, float)>& totalSimilarityCallback) const {
    // Checked here as well, so that an empty collection rejects the pattern as a full one does.
    requireTwoNotes("ScoreCollection::findMelodyPattern", melodyPattern.size());
    MelodyPatternTable rows;
    for (const Score& score : _scores) {
        appendRows(
            score,
            score.findMelodyPattern(melodyPattern, intervalSimilarityThreshold,
                                    rhythmSimilarityThreshold, intervalsSimilarityCallback,
                                    rhythmSimilarityCallback, totalIntervalSimilarityCallback,
                                    totalRhythmSimilarityCallback, totalSimilarityCallback),
            &rows);
    }
    sortByScoreTitle(&rows);
    return rows;
}

std::vector<ScoreCollection::MelodyPatternTable> ScoreCollection::findMelodyPattern(
    const std::vector<std::vector<Note>>& melodyPatterns, const float intervalSimilarityThreshold,
    const float rhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
        intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
        rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
    const std::function<float(float, float)>& totalSimilarityCallback) const {
    for (const std::vector<Note>& pattern : melodyPatterns) {
        requireTwoNotes("ScoreCollection::findMelodyPattern", pattern.size());
    }
    std::vector<MelodyPatternTable> tables(melodyPatterns.size());
    for (const Score& score : _scores) {
        const std::vector<Score::MelodyPatternTable> scoreTables = score.findMelodyPattern(
            melodyPatterns, intervalSimilarityThreshold, rhythmSimilarityThreshold,
            intervalsSimilarityCallback, rhythmSimilarityCallback, totalIntervalSimilarityCallback,
            totalRhythmSimilarityCallback, totalSimilarityCallback);
        for (size_t p = 0; p < scoreTables.size(); p++) {
            appendRows(score, scoreTables[p], &tables[p]);
        }
    }
    for (MelodyPatternTable& table : tables) {
        sortByScoreTitle(&table);
    }
    return tables;
}

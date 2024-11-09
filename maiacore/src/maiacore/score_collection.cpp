#include "maiacore/score_collection.h"

#include <filesystem>
#include <vector>
#include <string>
#include <tuple>

#include "maiacore/log.h"

ScoreCollection::ScoreCollection(const std::string& directoryPath) {
    setDirectoriesPaths({directoryPath});
}

ScoreCollection::ScoreCollection(const std::vector<std::string>& directoriesPaths) {
    setDirectoriesPaths(directoriesPaths);
}

std::vector<std::string> ScoreCollection::getDirectoriesPaths() const { return _directoriesPaths; }

void ScoreCollection::setDirectoriesPaths(const std::vector<std::string>& directoriesPaths) {
    _directoriesPaths = directoriesPaths;

    loadCollectionFiles();
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

void ScoreCollection::loadCollectionFiles() {
    if (_directoriesPaths.empty()) {
        return;
    }

    for (const auto& dir : _directoriesPaths) {
        for (const auto& fp : std::filesystem::directory_iterator(dir)) {
            const std::filesystem::path& ext = fp.path().extension();
            if (ext == ".xml" || ext == ".mxl" || ext == ".musicxml") {
                const std::string& fileName = fp.path().filename().string();
                LOG_INFO("Loading: " << fileName);

                const std::string& filePath = fp.path().string();
                _scores.emplace_back(filePath);
            }
        }
    }
}

void ScoreCollection::merge(const ScoreCollection& other) {
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
    if (scoreIdx >= static_cast<int>(_scores.size())) {
        LOG_ERROR("Invalid score index: " + std::to_string(scoreIdx));
        return;
    }

    _scores.erase(_scores.begin() + scoreIdx);
}

ScoreCollection::ExtendedMelodyPatternTable ScoreCollection::findMelodyPattern(
    const std::vector<Note>& melodyPattern, const float totalIntervalsSimilarityThreshold,
    const float totalRhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
    const std::function<float(float, float)>& totalSimilarityCallback) const {
    
    ScoreCollection::ExtendedMelodyPatternTable results;
    for (const auto& score : _scores) {
        auto scoreResults = score.findMelodyPattern(melodyPattern, totalIntervalsSimilarityThreshold,
                                                    totalRhythmSimilarityThreshold,
                                                    intervalsSimilarityCallback, rhythmSimilarityCallback,
                                                    totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                                                    totalSimilarityCallback);
        
        for (const auto& row : scoreResults) {
            // Constrói diretamente um MelodyPatternRow com o título da partitura
            results.emplace_back(score.getFileName(), score.getComposerName(), score.getTitle(), std::get<0>(row), std::get<1>(row), std::get<2>(row),
                                 std::get<3>(row), std::get<4>(row), std::get<5>(row), std::get<6>(row),
                                 std::get<7>(row), std::get<8>(row), std::get<9>(row), std::get<10>(row));
        }
    }
    return results;
}

std::vector<ScoreCollection::ExtendedMelodyPatternTable> ScoreCollection::findMelodyPattern(
    const std::vector<std::vector<Note>>& melodyPatterns, const float totalIntervalsSimilarityThreshold,
    const float totalRhythmSimilarityThreshold,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback,
    const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback,
    const std::function<float(float, float)>& totalSimilarityCallback) const {
    
    std::vector<ExtendedMelodyPatternTable> allResults;
    for (const auto& score : _scores) {
        auto scoreResults = score.findMelodyPattern(melodyPatterns, totalIntervalsSimilarityThreshold,
                                                    totalRhythmSimilarityThreshold,
                                                    intervalsSimilarityCallback, rhythmSimilarityCallback,
                                                    totalIntervalSimilarityCallback, totalRhythmSimilarityCallback,
                                                    totalSimilarityCallback);
        
        ScoreCollection::ExtendedMelodyPatternTable extendedTable;
        for (const auto& table : scoreResults) {       // Itera sobre cada tabela
            for (const Score::MelodyPatternRow& row : table) { // Itera sobre cada linha da tabela
                // Adiciona uma nova linha ao extendedTable, incluindo o título da partitura no início
                extendedTable.emplace_back(
                    score.getFileName(),                  // Nome do arquivo
                    score.getComposerName(),              // Nome do compositor
                    score.getTitle(),                       // Título da partitura
                    std::get<0>(row),                       // partName
                    std::get<1>(row),                       // measureNumber
                    std::get<2>(row),                       // staveId
                    std::get<3>(row),                       // writtenClefKey
                    std::get<4>(row),                       // transposeInterval
                    std::get<5>(row),                       // segmentWrittenPitch
                    std::get<6>(row),                       // semitonesDiff
                    std::get<7>(row),                       // rhythmDiff
                    std::get<8>(row),                       // totalIntervalSimilarity
                    std::get<9>(row),                       // totalRhythmSimilarity
                    std::get<10>(row)                       // totalSimilarity
                );
            }
        }
        allResults.push_back(extendedTable);
    }
    return allResults;
}


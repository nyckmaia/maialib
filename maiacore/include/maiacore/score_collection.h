#pragma once

#include <string>
#include <vector>

#include "maiacore/score.h"

class ScoreCollection {
   private:
    std::vector<std::string> _directoriesPaths;
    std::vector<Score> _scores;

    void loadCollectionFiles();

    typedef std::tuple<std::string, std::string, std::string, std::string, int, int, std::string, std::string, std::vector<std::string>, 
                   std::vector<float>, std::vector<float>, float, float, float> ExtendedMelodyPatternRow;
    typedef std::vector<ExtendedMelodyPatternRow> ExtendedMelodyPatternTable;
    typedef std::tuple<int, std::string, std::string, std::string, std::string, int, int, std::string, std::string, std::vector<std::string>, 
                   std::vector<float>, std::vector<float>, float, float, float> ExtendedMultiMelodyPatternRow;
    typedef std::vector<ExtendedMultiMelodyPatternRow> ExtendedMultiMelodyPatternTable;

   public:
    explicit ScoreCollection(const std::string& directoryPath = {});
    explicit ScoreCollection(const std::vector<std::string>& directoriesPaths = {});

    std::vector<std::string> getDirectoriesPaths() const;
    void setDirectoriesPaths(const std::vector<std::string>& directoriesPaths);

    void addDirectory(const std::string& directoryPath);
    void addScore(const Score& score);
    void addScore(const std::string& filePath);
    void addScore(const std::vector<std::string>& filePaths);
    void clear();
    int getNumDirectories() const;
    int getNumScores() const;
    std::vector<Score>& getScores();
    const std::vector<Score>& getScores() const;
    bool isEmpty() const;
    void merge(const ScoreCollection& other);
    void removeScore(const int scoreIdx);
    
    ExtendedMelodyPatternTable findMelodyPattern(
    const std::vector<Note>& melodyPattern, const float totalIntervalsSimilarityThreshold = 0.5f,
    const float totalRhythmSimilarityThreshold = 0.5f,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback = nullptr,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback = nullptr,
    const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback = nullptr,
    const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback = nullptr,
    const std::function<float(float, float)>& totalSimilarityCallback = nullptr) const;

    std::vector<ExtendedMultiMelodyPatternTable> findMelodyPattern(
    const std::vector<std::vector<Note>>& melodyPatterns, const float totalIntervalsSimilarityThreshold = 0.5f,
    const float totalRhythmSimilarityThreshold = 0.5f,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback = nullptr,
    const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback = nullptr,
    const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback = nullptr,
    const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback = nullptr,
    const std::function<float(float, float)>& totalSimilarityCallback = nullptr) const;

    ScoreCollection operator+(const ScoreCollection& other) const {
        ScoreCollection sc = *this;

        sc.merge(other);

        return sc;
    }
};
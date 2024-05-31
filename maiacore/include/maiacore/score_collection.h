#pragma once

#include <string>
#include <vector>

#include "maiacore/score.h"

class ScoreCollection {
   private:
    std::vector<std::string> _directoriesPaths;
    std::vector<Score> _scores;

    void loadCollectionFiles();

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

    ScoreCollection operator+(const ScoreCollection& other) const {
        ScoreCollection sc = *this;

        sc.merge(other);

        return sc;
    }
};
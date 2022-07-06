#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "score.h"

class ScoreCollection
{
private:
    std::string _collectionName;
    std::string _directoryPath;
    std::vector<Score> _scores;

public:
    explicit ScoreCollection(const std::string& collectionName, const std::string& directoryPath = {});

    std::string getName() const;
    std::vector<Score>& getScores();
    int getNumScores() const;
    std::string getDirectoryPath() const;

    void setName(const std::string& collectionName);
    void setDirectoryPath(const std::string& directoryPath);

    void loadCollectionFiles();
    void clear();
    bool isEmpty() const;

    void addScore(const Score& score);
    void removeScore(const int scoreIdx);
};
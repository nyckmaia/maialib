#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "score.h"

class ScoreCollection
{
private:
    std::string _collectionName;
    std::vector<std::string> _directoriesPaths;
    std::vector<Score> _scores;

    void loadCollectionFiles();
public:
    explicit ScoreCollection(const std::string& collectionName, const std::string& directoryPath = {});
    explicit ScoreCollection(const std::string& collectionName, const std::vector<std::string>& directoriesPaths = {});

    std::vector<std::string> getDirectoriesPaths() const;
    void setDirectoriesPaths(const std::vector<std::string>& directoriesPaths);
    
    std::string getName() const;
    void setName(const std::string& collectionName);
    
    void addDirectory(const std::string& directoryPath);
    void addScore(const Score& score);
    void clear();
    int getNumDirectories() const;
    int getNumScores() const;
    std::vector<Score>& getScores();
    const std::vector<Score>& getScores() const;
    bool isEmpty() const;
    void merge(const ScoreCollection& other, const std::string& mergedCollectionName = {});
    void removeScore(const int scoreIdx);

    ScoreCollection operator + (const ScoreCollection& other) const {
        ScoreCollection sc = *this;

        const std::string mergedName = sc.getName() + " + " + other.getName();
        sc.merge(other, mergedName);

        return sc;
    }
};
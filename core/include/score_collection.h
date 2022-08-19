// #pragma once

// #include <vector>
// #include <string>
// #include <filesystem>

// #include "score.h"

// class ScoreCollection
// {
// private:
//     std::string _collectionName;
//     std::vector<std::pair<std::vector<std::string>, bool>> _directoryPaths;
//     std::vector<Score> _scores;

// public:
//     explicit ScoreCollection(const std::string& collectionName, const std::string& directoryPath = {}, const bool includeSubFolders = false);
//     explicit ScoreCollection(const std::string& collectionName, const std::vector<std::string>& directoryPaths = {}, const bool includeSubFolders = false);

//     std::string getName() const;
//     std::vector<Score>& getScores();
//     int getNumScores() const;
//     std::vector<std::string> getDirectoryPath() const;

//     void setName(const std::string& collectionName);
//     void setDirectoryPath(const std::string& directoryPath, const bool includeSubFolders = false);
//     void setDirectoryPaths(const std::vector<std::string>& directoryPaths, const bool includeSubFolders = false);

//     void loadCollectionFiles();
//     void clear();
//     bool isEmpty() const;
//     int getNumDirectories() const;
//     void addDirectory(const std::string& directoryPath);

//     void addScore(const Score& score);
//     void removeScore(const int scoreIdx);
// };
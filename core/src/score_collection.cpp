#include "score_collection.h"

ScoreCollection::ScoreCollection(const std::string& collectionName, const std::string& directoryPath)
{
    _collectionName = collectionName;
    setDirectoriesPaths({directoryPath});
}

ScoreCollection::ScoreCollection(const std::string& collectionName, const std::vector<std::string>& directoriesPaths)
{
    _collectionName = collectionName;
    setDirectoriesPaths(directoriesPaths);
}

std::vector<std::string> ScoreCollection::getDirectoriesPaths() const
{
    return _directoriesPaths;
}

void ScoreCollection::setDirectoriesPaths(const std::vector<std::string>& directoriesPaths)
{
    _directoriesPaths = directoriesPaths;

    loadCollectionFiles();
}

std::string ScoreCollection::getName() const
{
    return _collectionName;
}

void ScoreCollection::setName(const std::string& collectionName)
{
    _collectionName = collectionName;
}

void ScoreCollection::addDirectory(const std::string& directoryPath)
{
    _directoriesPaths.push_back(directoryPath);
}

void ScoreCollection::addScore(const Score& score)
{
    _scores.push_back(score);
}

void ScoreCollection::addScore(const std::string& filePath)
{
    addScore(Score(filePath));
}

void ScoreCollection::addScore(const std::vector<std::string>& filePaths)
{
    for (const auto& fp : filePaths) {
        addScore(fp);
    }
}

void ScoreCollection::clear()
{
    _scores.clear();
}

int ScoreCollection::getNumDirectories() const
{
    return static_cast<int>(_directoriesPaths.size());
}

int ScoreCollection::getNumScores() const
{
    return _scores.size();
}

std::vector<Score>& ScoreCollection::getScores()
{
    return _scores;
}

const std::vector<Score>& ScoreCollection::getScores() const
{
    return _scores;
}

bool ScoreCollection::isEmpty() const
{
    return _scores.empty();
}

void ScoreCollection::loadCollectionFiles()
{
    if (_directoriesPaths.empty()) { return; }

    for (const auto& dir : _directoriesPaths) {
        for (const auto &fp : std::filesystem::directory_iterator(dir)) {
            const std::filesystem::path& ext = fp.path().extension();
            if (ext == ".xml" || ext == ".mxl" || ext == ".musicxml") {
                const std::string& fileName = fp.path().filename().string();
                std::cout << "Loading: " << fileName << std::endl;

                const std::string& filePath = fp.path().string();
                _scores.emplace_back(filePath);
            }
        }
    }
}

void ScoreCollection::merge(const ScoreCollection& other, const std::string& mergedCollectionName)
{
    // Merge directories paths    
    for (const auto& dir : other.getDirectoriesPaths()) {
        _directoriesPaths.push_back(dir);
    }
    
    // Merge Score objects
    for (const auto& sc : other.getScores()) {
        _scores.push_back(sc);
    }

    // Set new merged ScoreCollection name
    if (!mergedCollectionName.empty()) {
        _collectionName = mergedCollectionName;
    }
}

void ScoreCollection::removeScore(const int scoreIdx)
{
    if (scoreIdx >= static_cast<int>(_scores.size())) {
        std::cerr << "[ERROR] Invalid score index: " << scoreIdx << std::endl;
        return;
    }

    _scores.erase(_scores.begin() + scoreIdx);
}
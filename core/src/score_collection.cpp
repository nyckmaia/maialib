#include "score_collection.h"

ScoreCollection::ScoreCollection(const std::string& collectionName, const std::string& directoryPath)
{
    _collectionName = collectionName;

    std::filesystem::path fp(directoryPath); 
    if (!directoryPath.empty() && !std::filesystem::is_directory(fp)) { 
        std::cerr << "[ERROR] The path '" << directoryPath << "' is not a directory" << std::endl;
        return;
    }

    _directoryPath = directoryPath;

    if (directoryPath.empty()) { return; }

    loadCollectionFiles();
}

std::string ScoreCollection::getName() const
{
    return _collectionName;
}

std::vector<Score>& ScoreCollection::getScores()
{
    return _scores;
}

int ScoreCollection::getNumScores() const
{
    return _scores.size();
}

std::string ScoreCollection::getDirectoryPath() const
{
    return _directoryPath;
}

void ScoreCollection::setName(const std::string& collectionName)
{
    _collectionName = collectionName;
}

void ScoreCollection::setDirectoryPath(const std::string& directoryPath)
{
    std::filesystem::path fp(directoryPath); 
    if (!std::filesystem::is_directory(fp)) { 
        std::cerr << "[ERROR] The path '" << directoryPath << "' is not a directory" << std::endl;
        return;
    }

    _directoryPath = directoryPath;
}

void ScoreCollection::loadCollectionFiles()
{
    for (auto &p : std::filesystem::recursive_directory_iterator(_directoryPath)) {
        const std::filesystem::path& ext = p.path().extension();
        if (ext == ".xml" || ext == ".mxl" || ext == ".musicxml") {
            const std::string& fileName = p.path().filename().string();
            std::cout << "Loading: " << fileName << std::endl;
            
            const std::string& filePath = p.path().string();
            _scores.emplace_back(filePath);
        }
    }
}

void ScoreCollection::clear()
{
    _scores.clear();
}

bool ScoreCollection::isEmpty() const
{
    return _scores.empty();
}

void ScoreCollection::addScore(const Score& score)
{
    _scores.push_back(score);
}

void ScoreCollection::removeScore(const int scoreIdx)
{
    if (scoreIdx >= static_cast<int>(_scores.size())) {
        std::cerr << "[ERROR] Invalid score index" << std::endl;
        return;
    }

    _scores.erase(_scores.begin() + scoreIdx);
}
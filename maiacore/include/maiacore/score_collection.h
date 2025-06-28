#pragma once

#include <string>
#include <vector>
#include <functional>

#include "maiacore/score.h"

/**
 * @brief Represents a collection of musical scores, supporting batch analysis and management.
 *
 * The ScoreCollection class provides methods for loading, managing, and analyzing multiple Score objects.
 * It is designed for large-scale musicological research, corpus studies, and batch processing of MusicXML files.
 */
class ScoreCollection {
   private:
    std::vector<std::string> _directoriesPaths; ///< List of directories containing score files.
    std::vector<Score> _scores; ///< Vector of loaded Score objects.

    /**
     * @brief Loads all MusicXML files from the specified directories into the collection.
     * @details Scans each directory for .xml, .mxl, and .musicxml files and loads them as Score objects.
     */
    void loadCollectionFiles();

    /**
     * @brief Row type for extended melodic pattern search results (single pattern).
     * @details Includes file metadata and all fields from Score::MelodyPatternRow.
     */
    typedef std::tuple<std::string, std::string, std::string, std::string, int, int, std::string, std::string, std::vector<std::string>, 
                   std::vector<float>, std::vector<float>, float, float, float> ExtendedMelodyPatternRow;
    typedef std::vector<ExtendedMelodyPatternRow> ExtendedMelodyPatternTable;

    /**
     * @brief Row type for extended multi-pattern melodic search results.
     * @details Includes pattern index, file metadata, and all fields from Score::MelodyPatternRow.
     */
    typedef std::tuple<int, std::string, std::string, std::string, std::string, int, int, std::string, std::string, std::vector<std::string>, 
                   std::vector<float>, std::vector<float>, float, float, float> ExtendedMultiMelodyPatternRow;
    typedef std::vector<ExtendedMultiMelodyPatternRow> ExtendedMultiMelodyPatternTable;

   public:
    /**
     * @brief Constructs a ScoreCollection from a single directory path.
     * @param directoryPath Path to a directory containing MusicXML files.
     */
    explicit ScoreCollection(const std::string& directoryPath = {});

    /**
     * @brief Constructs a ScoreCollection from multiple directory paths.
     * @param directoriesPaths Vector of directory paths.
     */
    explicit ScoreCollection(const std::vector<std::string>& directoriesPaths = {});

    /**
     * @brief Returns the list of directory paths associated with the collection.
     * @return Vector of directory path strings.
     */
    std::vector<std::string> getDirectoriesPaths() const;

    /**
     * @brief Sets the list of directory paths and reloads the collection.
     * @param directoriesPaths Vector of directory path strings.
     */
    void setDirectoriesPaths(const std::vector<std::string>& directoriesPaths);

    /**
     * @brief Adds a directory path to the collection (does not reload files automatically).
     * @param directoryPath Directory path string.
     */
    void addDirectory(const std::string& directoryPath);

    /**
     * @brief Adds a Score object to the collection.
     * @param score Score object to add.
     */
    void addScore(const Score& score);

    /**
     * @brief Loads a Score from a file path and adds it to the collection.
     * @param filePath Path to a MusicXML file.
     */
    void addScore(const std::string& filePath);

    /**
     * @brief Loads multiple Scores from file paths and adds them to the collection.
     * @param filePaths Vector of MusicXML file paths.
     */
    void addScore(const std::vector<std::string>& filePaths);

    /**
     * @brief Removes all scores from the collection.
     */
    void clear();

    /**
     * @brief Returns the number of directories in the collection.
     * @return Number of directories.
     */
    int getNumDirectories() const;

    /**
     * @brief Returns the number of scores in the collection.
     * @return Number of Score objects.
     */
    int getNumScores() const;

    /**
     * @brief Returns a reference to the vector of Score objects (modifiable).
     * @return Reference to vector of Score.
     */
    std::vector<Score>& getScores();

    /**
     * @brief Returns a const reference to the vector of Score objects.
     * @return Const reference to vector of Score.
     */
    const std::vector<Score>& getScores() const;

    /**
     * @brief Returns true if the collection contains no scores.
     * @return True if empty.
     */
    bool isEmpty() const;

    /**
     * @brief Merges another ScoreCollection into this one, combining directories and scores.
     * @param other Another ScoreCollection.
     */
    void merge(const ScoreCollection& other);

    /**
     * @brief Removes a score from the collection by its index.
     * @param scoreIdx Index of the score to remove.
     */
    void removeScore(const int scoreIdx);

    /**
     * @brief Searches for a melodic pattern in all scores, returning extended results.
     * @details Each result row includes file metadata and all fields from Score::MelodyPatternRow.
     * @param melodyPattern Vector of Note objects representing the pattern.
     * @param totalIntervalsSimilarityThreshold Minimum interval similarity threshold.
     * @param totalRhythmSimilarityThreshold Minimum rhythm similarity threshold.
     * @param intervalsSimilarityCallback Custom function to calculate interval similarity.
     * @param rhythmSimilarityCallback Custom function to calculate rhythm similarity.
     * @param totalIntervalSimilarityCallback Function to aggregate interval similarity.
     * @param totalRhythmSimilarityCallback Function to aggregate rhythm similarity.
     * @param totalSimilarityCallback Function to combine total similarities.
     * @return ExtendedMelodyPatternTable with results from all scores.
     */
    ExtendedMelodyPatternTable findMelodyPattern(
        const std::vector<Note>& melodyPattern, const float totalIntervalsSimilarityThreshold = 0.5f,
        const float totalRhythmSimilarityThreshold = 0.5f,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback = nullptr,
        const std::function<float(float, float)>& totalSimilarityCallback = nullptr) const;

    /**
     * @brief Searches for multiple melodic patterns in all scores, returning extended results for each pattern.
     * @details Each result row includes pattern index, file metadata, and all fields from Score::MelodyPatternRow.
     * @param melodyPatterns Vector of melodic patterns (each a vector of Note).
     * @param totalIntervalsSimilarityThreshold Minimum interval similarity threshold.
     * @param totalRhythmSimilarityThreshold Minimum rhythm similarity threshold.
     * @param intervalsSimilarityCallback Custom function to calculate interval similarity.
     * @param rhythmSimilarityCallback Custom function to calculate rhythm similarity.
     * @param totalIntervalSimilarityCallback Function to aggregate interval similarity.
     * @param totalRhythmSimilarityCallback Function to aggregate rhythm similarity.
     * @param totalSimilarityCallback Function to combine total similarities.
     * @return Vector of ExtendedMultiMelodyPatternTable, one for each pattern.
     */
    std::vector<ExtendedMultiMelodyPatternTable> findMelodyPattern(
        const std::vector<std::vector<Note>>& melodyPatterns, const float totalIntervalsSimilarityThreshold = 0.5f,
        const float totalRhythmSimilarityThreshold = 0.5f,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>& rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback = nullptr,
        const std::function<float(float, float)>& totalSimilarityCallback = nullptr) const;

    /**
     * @brief Merges two ScoreCollections using the + operator.
     * @param other Another ScoreCollection.
     * @return New ScoreCollection containing all scores and directories from both.
     */
    ScoreCollection operator+(const ScoreCollection& other) const {
        ScoreCollection sc = *this;
        sc.merge(other);
        return sc;
    }
};
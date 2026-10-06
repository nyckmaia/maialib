#pragma once

#include <functional>
#include <string>
#include <vector>

#include "maiacore/score.h"

/**
 * @brief Represents a collection of musical scores, supporting batch analysis and management.
 *
 * The ScoreCollection class provides methods for loading, managing, and analyzing multiple Score
 * objects. It is designed for large-scale musicological research, corpus studies, and batch
 * processing of MusicXML files.
 */
class ScoreCollection {
   public:
    /**
     * @brief One match of a melodic pattern in a score of the collection.
     */
    struct MelodyPatternRow {
        std::string fileName;           ///< The score's file name (Score::getFileName()).
        std::string composerName;       ///< The score's composer (Score::getComposerName()).
        std::string scoreTitle;         ///< The score's title (Score::getTitle()).
        Score::MelodyPatternRow match;  ///< The match, as Score::findMelodyPattern() gives it.
    };

    /**
     * @brief The matches of one pattern in every score, sorted stably by score title: the matches
     *        of one score keep the order Score::findMelodyPattern() gives them, and scores with
     *        the same title keep the collection's order.
     */
    typedef std::vector<MelodyPatternRow> MelodyPatternTable;

    /**
     * @brief Constructs an empty collection: no directory and no score.
     */
    ScoreCollection();

    /**
     * @brief Constructs a collection of the MusicXML files of a directory.
     * @param directoryPath A directory; its files are loaded as setDirectoriesPaths() loads them.
     * @param recursive True to load the files of its subdirectories, at any depth, too.
     * @throws std::runtime_error As setDirectoriesPaths() throws.
     */
    explicit ScoreCollection(const std::string& directoryPath, const bool recursive = false);

    /**
     * @brief Constructs a collection of the MusicXML files of several directories.
     * @param directoriesPaths The directories; their files are loaded as setDirectoriesPaths()
     *        loads them.
     * @param recursive True to load the files of their subdirectories, at any depth, too.
     * @throws std::runtime_error As setDirectoriesPaths() throws.
     */
    explicit ScoreCollection(const std::vector<std::string>& directoriesPaths,
                             const bool recursive = false);

    /**
     * @brief Returns the list of directory paths associated with the collection.
     * @return Vector of directory path strings.
     */
    std::vector<std::string> getDirectoriesPaths() const;

    /**
     * @brief Replaces the collection's directories and scores with those of the given
     *        directories.
     * @details Loads every file whose extension is `.xml`, `.mxl` or `.musicxml`, compared
     *          without regard to case, directory by directory in the given order and, within a
     *          directory, in sorted path order. Every path is checked before anything is loaded,
     *          and the collection changes only when every file has loaded. A subdirectory the
     *          user has no permission to read is skipped.
     * @param directoriesPaths The directories; an empty list empties the collection.
     * @param recursive True to load the files of their subdirectories, at any depth, too.
     * @throws std::runtime_error If a path does not exist or is not a directory, or a directory
     *         cannot be read: the message names the path that failed, a given path or one of its
     *         subdirectories. If a file fails to load: the message is the file's path, ": " and
     *         the message of the load's error.
     */
    void setDirectoriesPaths(const std::vector<std::string>& directoriesPaths,
                             const bool recursive = false);

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
     * @brief Removes all scores from the collection; its directories are kept.
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
     * @param scoreIdx Index of the score to remove, from 0 to getNumScores() - 1.
     * @throws std::out_of_range If scoreIdx is negative or not below getNumScores(); the
     *         collection is unchanged.
     */
    void removeScore(const int scoreIdx);

    /**
     * @brief Searches every score of the collection for a melodic pattern.
     * @details Each score is searched as Score::findMelodyPattern() searches it.
     * @param melodyPattern The pattern: at least 2 notes.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match.
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match.
     * @param intervalsSimilarityCallback See Score::findMelodyPattern().
     * @param rhythmSimilarityCallback See Score::findMelodyPattern().
     * @param totalIntervalSimilarityCallback See Score::findMelodyPattern().
     * @param totalRhythmSimilarityCallback See Score::findMelodyPattern().
     * @param totalSimilarityCallback See Score::findMelodyPattern().
     * @return The matches in every score, sorted stably by score title.
     * @throws std::runtime_error If the pattern has fewer than 2 notes, even for an empty
     *         collection, or as Score::findMelodyPattern() throws.
     */
    MelodyPatternTable findMelodyPattern(
        const std::vector<Note>& melodyPattern, const float intervalSimilarityThreshold = 0.5f,
        const float rhythmSimilarityThreshold = 0.5f,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback =
            nullptr,
        const std::function<float(float, float)>& totalSimilarityCallback = nullptr) const;

    /**
     * @brief Searches every score of the collection for several melodic patterns.
     * @details Each score is searched as the list overload of Score::findMelodyPattern()
     *          searches it.
     * @param melodyPatterns The patterns.
     * @param intervalSimilarityThreshold Minimum interval similarity of a match.
     * @param rhythmSimilarityThreshold Minimum rhythm similarity of a match.
     * @param intervalsSimilarityCallback See Score::findMelodyPattern().
     * @param rhythmSimilarityCallback See Score::findMelodyPattern().
     * @param totalIntervalSimilarityCallback See Score::findMelodyPattern().
     * @param totalRhythmSimilarityCallback See Score::findMelodyPattern().
     * @param totalSimilarityCallback See Score::findMelodyPattern().
     * @return One table per pattern, in pattern order, each with the pattern's matches in every
     *         score, sorted stably by score title.
     * @throws std::runtime_error If a pattern has fewer than 2 notes, even for an empty
     *         collection, or as Score::findMelodyPattern() throws.
     */
    std::vector<MelodyPatternTable> findMelodyPattern(
        const std::vector<std::vector<Note>>& melodyPatterns,
        const float intervalSimilarityThreshold = 0.5f,
        const float rhythmSimilarityThreshold = 0.5f,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
            intervalsSimilarityCallback = nullptr,
        const std::function<std::vector<float>(const std::vector<Note>&, const std::vector<Note>&)>&
            rhythmSimilarityCallback = nullptr,
        const std::function<float(const std::vector<float>&)>& totalIntervalSimilarityCallback =
            nullptr,
        const std::function<float(const std::vector<float>&)>& totalRhythmSimilarityCallback =
            nullptr,
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

   private:
    std::vector<std::string> _directoriesPaths;  ///< List of directories containing score files.
    std::vector<Score> _scores;                  ///< Vector of loaded Score objects.
};

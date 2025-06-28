#pragma once
#include <string>

/**
 * @brief Represents a musical key signature (tonality), including circle of fifths and mode (major/minor).
 *
 * The Key class provides methods for setting and querying the key signature, including conversion between
 * fifth circle values, key names, and relative keys. It is designed for music analysis, computational musicology,
 * and MusicXML processing.
 */
class Key {
   public:
    /**
     * @brief Constructs a Key from a fifth circle value and mode.
     * @param fifthCircle Number of accidentals in the circle of fifths (-6 to +11).
     * @param isMajorMode True for major mode, false for minor mode.
     */
    Key(int fifthCircle = 0, bool isMajorMode = true);

    /**
     * @brief Constructs a Key from a key name string (e.g., "C", "Gm").
     * @param key Key name string.
     */
    Key(const std::string& key);

    /**
     * @brief Sets the fifth circle value (number of accidentals).
     * @param fifthCircle Number of accidentals (-6 to +11).
     */
    void setFifthCircle(int fifthCircle);

    /**
     * @brief Returns the fifth circle value (number of accidentals).
     * @return Integer in range -6 (Gb) to +11 (B#).
     */
    int getFifthCircle() const;

    /**
     * @brief Sets the mode of the key (major or minor).
     * @param isMajorMode True for major, false for minor.
     */
    void setIsMajorMode(bool isMajorMode);

    /**
     * @brief Returns true if the key is major, false if minor.
     * @return 1 if major, 0 if minor.
     */
    int isMajorMode() const;

    /**
     * @brief Returns the key name as a string (e.g., "C", "Gm").
     * @return Key name string.
     */
    std::string getName() const;

    /**
     * @brief Returns the relative key name (e.g., for C major returns "Am", for Am returns "C").
     * @return Relative key name string.
     */
    std::string getRelativeKeyName() const;

   private:
    int _fifthCircle; ///< Number of accidentals in the circle of fifths.
    bool _isMajorMode; ///< True for major, false for minor.
};
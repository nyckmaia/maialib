#pragma once

#include <string>

/**
 * @brief Enumeration for clef signs.
 * G: Treble clef, F: Bass clef, C: Alto/Tenor clef, PERCUSSION: Percussion clef.
 */
enum class ClefSign { G, F, C, PERCUSSION };

/**
 * @brief Represents a musical clef for a staff, including sign, line, and change status.
 *
 * The Clef class provides methods for setting and querying the clef sign (G, F, C, percussion),
 * the staff line on which the clef appears, and whether the clef has changed in the current measure.
 * It supports MusicXML serialization and is designed for music analysis, computational musicology,
 * and MusicXML processing.
 */
class Clef {
   private:
    ClefSign _sign; ///< Clef sign (G, F, C, percussion).
    int _line; ///< Staff line for the clef.
    bool _isClefChanged; ///< True if the clef changes in the current measure.

    /**
     * @brief Sets the clef changed status.
     * @param isClefChanged True if the clef has changed.
     */
    void setClefChanged(bool isClefChanged);

   public:
    /**
     * @brief Constructs a Clef with a given sign and line.
     * @param sign Clef sign (default: G).
     * @param line Staff line for the clef (default: -1, auto-detect).
     */
    Clef(const ClefSign sign = ClefSign::G, int line = -1);

    /**
     * @brief Destructor.
     */
    ~Clef();

    /**
     * @brief Returns the staff line for the clef.
     * @return Integer representing the staff line.
     */
    int getLine() const;

    /**
     * @brief Sets the staff line for the clef.
     * @details Only applicable for C and percussion clefs.
     * @param line New staff line.
     */
    void setLine(int line);

    /**
     * @brief Returns the clef sign (G, F, C, percussion).
     * @return ClefSign enum value.
     */
    ClefSign getSign() const;

    /**
     * @brief Sets the clef sign and updates the staff line accordingly.
     * @param sign New clef sign.
     */
    void setSign(const ClefSign sign);

    /**
     * @brief Returns true if the clef has changed in the current measure.
     * @return True if clef changed.
     */
    bool isClefChanged() const;

    /**
     * @brief Returns the clef sign as a string ("G", "F", "C", "percussion").
     * @return String representation of the clef sign.
     */
    std::string getClefSignStr() const;

    /**
     * @brief Converts a clef sign string to a ClefSign enum value.
     * @param clefStr Clef sign string ("G", "F", "C", "percussion").
     * @return ClefSign enum value.
     */
    static ClefSign clefSignStr2ClefSign(const std::string& clefStr);

    /**
     * @brief Serializes the clef to MusicXML format.
     * @param clefNumber Clef number (default: -1 for single staff).
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string for the clef.
     */
    std::string toXML(const int clefNumber = -1, const int identSize = 2) const;
};

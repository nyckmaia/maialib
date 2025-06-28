#pragma once

#include <string>

/**
 * @brief Represents a barline in a musical score, including style, direction, and location.
 *
 * The Barline class provides methods for setting and querying the barline style (e.g., single, double, repeat),
 * direction (forward, backward), and location (left, right, middle). It supports MusicXML serialization and is
 * designed for music analysis, computational musicology, and MusicXML processing.
 */
class Barline {
   private:
    std::string _barStyle;   ///< Barline style (e.g., "light-heavy", "heavy-light").
    std::string _direction;  ///< Repeat direction ("forward", "backward").
    std::string _location;   ///< Barline location ("left", "right", "middle").

   public:
    /**
     * @brief Constructs a new Barline object with default values.
     */
    Barline();

    /**
     * @brief Clears all barline properties, resetting to default state.
     */
    void clean();

    /**
     * @brief Returns the barline style as a string.
     * @return Barline style string.
     */
    std::string getBarStyle() const;

    /**
     * @brief Sets the barline style.
     * @param barStyle Barline style string (e.g., "light-heavy").
     */
    void setBarStyle(const std::string& barStyle);

    /**
     * @brief Returns the repeat direction as a string.
     * @return Direction string ("forward", "backward", or empty).
     */
    std::string getDirection() const;

    /**
     * @brief Sets the repeat direction.
     * @param direction Direction string ("forward" or "backward").
     */
    void setDirection(const std::string& direction);

    /**
     * @brief Returns the barline location as a string.
     * @return Location string ("left", "right", "middle", or empty).
     */
    std::string getLocation() const;

    /**
     * @brief Sets the barline location.
     * @param location Location string ("left", "right", "middle").
     */
    void setLocation(const std::string& location);

    /**
     * @brief Sets the barline as a repeat start (heavy-light, forward).
     */
    void setRepeatStart();

    /**
     * @brief Sets the barline as a repeat end (light-heavy, backward).
     */
    void setRepeatEnd();

    /**
     * @brief Serializes the barline to MusicXML format.
     * @param identSize Indentation size (default: 2).
     * @return MusicXML string for the barline.
     */
    std::string toXML(const int identSize = 2) const;
};

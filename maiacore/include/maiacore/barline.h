#pragma once

#include <string>

class Barline {
   private:
    std::string _barStyle;
    std::string _direction;
    std::string _location;

   public:
    /**
     * @brief Construct a new Barline object
     *
     */
    Barline();

    /**
     * @brief
     *
     */
    void clean();

    /**
     * @brief Get the Bar Style object
     *
     * @return std::string
     */
    std::string getBarStyle() const;

    /**
     * @brief Set the Bar Style object
     *
     * @param barStyle
     */
    void setBarStyle(const std::string& barStyle);

    /**
     * @brief Get the Direction object
     *
     * @return std::string
     */
    std::string getDirection() const;

    /**
     * @brief Set the Direction object
     *
     * @param direction
     */
    void setDirection(const std::string& direction);

    /**
     * @brief Get the Location object
     *
     * @return std::string
     */
    std::string getLocation() const;

    /**
     * @brief Set the Location object
     *
     * @param location
     */
    void setLocation(const std::string& location);

    /**
     * @brief Set the Repeat Start object
     *
     */
    void setRepeatStart();

    /**
     * @brief Set the Repeat End object
     *
     */
    void setRepeatEnd();

    /**
     * @brief
     *
     * @param identSize
     * @return std::string
     */
    std::string toXML(const int identSize = 2) const;
};

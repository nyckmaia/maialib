#pragma once

#include <string>

enum class ClefSign { G, F, C, PERCUSSION };

class Clef {
   private:
    ClefSign _sign;
    int _line;
    bool _isClefChanged;

    /**
     * @brief Set the Clef Changed object
     *
     * @param isClefChanged
     */
    void setClefChanged(bool isClefChanged);

   public:
    /**
     * @brief Construct a new Clef object
     *
     * @param sign
     * @param line
     */
    Clef(const ClefSign sign = ClefSign::G, int line = -1);

    /**
     * @brief Destroy the Clef object
     *
     */
    ~Clef();

    /**
     * @brief Get the Line object
     *
     * @return int
     */
    int getLine() const;

    /**
     * @brief Set the Line object
     *
     * @param line
     */
    void setLine(int line);

    /**
     * @brief Get the Sign object
     *
     * @return ClefSign
     */
    ClefSign getSign() const;

    /**
     * @brief Set the Sign object
     *
     * @param sign
     */
    void setSign(const ClefSign sign);

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isClefChanged() const;

    /**
     * @brief Get the Clef Sign Str object
     *
     * @return std::string
     */
    std::string getClefSignStr() const;

    /**
     * @brief
     *
     * @param clefStr
     * @return ClefSign
     */
    static ClefSign clefSignStr2ClefSign(const std::string& clefStr);

    /**
     * @brief
     *
     * @param clefNumber
     * @param identSize
     * @return std::string
     */
    std::string toXML(const int clefNumber = -1, const int identSize = 2) const;
};

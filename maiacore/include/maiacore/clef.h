#pragma once

#include <string>

class Clef {
   private:
    std::string _sign;
    int _line;

   public:
    /**
     * @brief Construct a new Clef object
     *
     * @param sign
     * @param line
     */
    Clef(const std::string& sign = "G", int line = 2);

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
     * @return std::string
     */
    std::string getSign() const;

    /**
     * @brief Set the Sign object
     *
     * @param sign
     */
    void setSign(const std::string& sign);
};

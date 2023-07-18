#pragma once
#include <string>

class Key {
   public:
    Key(int fifthCircle = 0, bool isMajorMode = true);

    Key(const std::string& key);

    void setFifthCircle(int fifthCircle);
    int getFifthCircle() const;

    void setIsMajorMode(bool isMajorMode);
    int isMajorMode() const;

    std::string getName() const;

    std::string getRelativeKeyName() const;

   private:
    int _fifthCircle;
    bool _isMajorMode;
};
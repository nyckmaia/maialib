#include <maiacore/constants.h>
#include <maiacore/key.h>
#include <maiacore/log.h>

Key::Key(int fifthCircle, bool isMajorMode) : _fifthCircle(fifthCircle), _isMajorMode(isMajorMode) {
    if (fifthCircle > 11 || fifthCircle < -6) {
        LOG_ERROR("'fifthCircle' value must be between -6 and 11");
    }
}

Key::Key(const std::string& key) : _fifthCircle(0), _isMajorMode(true) {
    const auto it = c_mapKeyName2FifthCirCleMode.find(key);

    if (it == c_mapKeyName2FifthCirCleMode.end()) {
        LOG_ERROR("Invalid key name: " + key);
    }

    const auto pair = it->second;
    _fifthCircle = pair.first;
    _isMajorMode = pair.second;
}

void Key::setFifthCircle(int fifthCircle) { _fifthCircle = fifthCircle; }

int Key::getFifthCircle() const { return _fifthCircle; }

void Key::setIsMajorMode(bool isMajorMode) { _isMajorMode = isMajorMode; }

int Key::isMajorMode() const { return _isMajorMode; }

std::string Key::getName() const {
    const int index = (_fifthCircle < 0) ? 12 + _fifthCircle : _fifthCircle;

    const auto pair =
        (_fifthCircle < 0) ? c_mapDescFifthTonality.at(index) : c_mapAscFifthTonality.at(index);
    const std::string keyName = (_isMajorMode) ? pair.first : pair.second;

    return keyName;
}

std::string Key::getRelativeKeyName() const {
    const int index = (_fifthCircle < 0) ? 12 + _fifthCircle : _fifthCircle;

    const auto pair =
        (_fifthCircle < 0) ? c_mapDescFifthTonality.at(index) : c_mapAscFifthTonality.at(index);
    const std::string relativeKeyName = (_isMajorMode) ? pair.second : pair.first;

    return relativeKeyName;
}
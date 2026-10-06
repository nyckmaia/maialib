#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

// A file holding 'content', byte for byte, in a directory of its own under the system's temporary
// directory; the directory is removed with the file when the object is destroyed. 'name' is the
// file's name in UTF-8, extension included: maialib decides from it how to read the file.
class TemporaryFile {
   public:
    TemporaryFile(const std::string& name, const std::string& content)
        : _directory(std::filesystem::temp_directory_path() /
                     ("maialib-test-" +
                      std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))),
          _path(_directory / std::filesystem::u8path(name)) {
        std::filesystem::create_directories(_directory);
        std::ofstream out(_path, std::ios::binary);
        out << content;
    }
    ~TemporaryFile() {
        std::error_code ignored;
        std::filesystem::remove_all(_directory, ignored);
    }
    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    // The file's path in UTF-8, as Score takes it.
    std::string path() const { return _path.u8string(); }

   private:
    std::filesystem::path _directory;
    std::filesystem::path _path;
};

// The <attributes> of the first measure of minimalScore() unless it is given others.
inline const std::string kMinimalAttributes =
    "<attributes><divisions>1</divisions><key><fifths>0</fifths></key><time><beats>4</beats>"
    "<beat-type>4</beat-type></time><clef><sign>G</sign><line>2</line></clef></attributes>";

// A whole-note C4 of voice 1.
inline const std::string kWholeC4 =
    "<note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><voice>1</voice>"
    "<type>whole</type></note>";

// A MusicXML 4.0 score of one part, "Music", and one measure, numbered "1", holding 'content'
// after 'attributes'. Every element it adds is one the model holds.
inline std::string minimalScore(const std::string& content,
                                const std::string& attributes = kMinimalAttributes) {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<score-partwise version=\"4.0\"><part-list><score-part id=\"P1\"><part-name>Music"
           "</part-name></score-part></part-list><part id=\"P1\"><measure number=\"1\">" +
           attributes + content + "</measure></part></score-partwise>\n";
}

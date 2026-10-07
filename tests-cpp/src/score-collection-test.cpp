#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/score_collection.h"
#include "test-capture.h"

// Test directories with XML files
const std::string BACH_DIR = "./test/xml_examples/Bach";
const std::string BEETHOVEN_DIR = "./test/xml_examples/Beethoven";
const std::string UNIT_TEST_DIR = "./test/xml_examples/unit_test";

// ============================================================================
// Constructor Tests
// ============================================================================

namespace {
const std::string LAST_WINDOW = "./test/xml_examples/unit_test/melody_last_window.musicxml";
const std::string DUPLICATES = "./test/xml_examples/unit_test/melody_duplicate_patterns.musicxml";

// A directory of its own under the system's temporary directory, removed with its contents when
// the object is destroyed.
class TemporaryDirectory {
   public:
    TemporaryDirectory()
        : _path(std::filesystem::temp_directory_path() /
                ("maialib-collection-test-" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))) {
        std::filesystem::create_directories(_path);
    }
    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(_path, ignored);
    }
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    const std::filesystem::path& path() const { return _path; }

    // The directory's path in UTF-8, as ScoreCollection takes it.
    std::string utf8() const { return _path.u8string(); }

    // Copies a score into the directory as 'name', UTF-8, which may name a subdirectory.
    void addCopy(const std::string& source, const std::string& name) const {
        const std::filesystem::path target = _path / std::filesystem::u8path(name);
        std::filesystem::create_directories(target.parent_path());
        std::filesystem::copy_file(source, target);
    }

   private:
    std::filesystem::path _path;
};

// The file names of a collection's scores, in collection order.
std::vector<std::string> fileNamesOf(const ScoreCollection& collection) {
    std::vector<std::string> names;
    for (const Score& score : collection.getScores()) {
        names.push_back(score.getFileName());
    }
    return names;
}

// The file name and the written pitches of each row of a table.
std::vector<std::pair<std::string, std::vector<std::string>>> rowsOf(
    const ScoreCollection::MelodyPatternTable& table) {
    std::vector<std::pair<std::string, std::vector<std::string>>> rows;
    for (const ScoreCollection::MelodyPatternRow& row : table) {
        rows.emplace_back(row.fileName, row.match.writtenPitches);
    }
    return rows;
}
}  // namespace

TEST(ScoreCollectionConstructor, DefaultConstructor) {
    ScoreCollection collection;
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_EQ(collection.getNumDirectories(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionConstructor, SingleDirectoryConstructor) {
    ScoreCollection collection(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);
    EXPECT_GT(collection.getNumScores(), 0);  // Should load Bach files
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionConstructor, MultipleDirectoriesConstructor) {
    std::vector<std::string> dirs = {BACH_DIR, BEETHOVEN_DIR};
    ScoreCollection collection(dirs);

    EXPECT_EQ(collection.getNumDirectories(), 2);
    EXPECT_GT(collection.getNumScores(), 0);  // Should load from both directories
}

TEST(ScoreCollectionConstructor, EmptyDirectoryPath) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

// A path that does not exist, or that is not a directory, raises a std::runtime_error whose
// message, in English, names it.
TEST(ScoreCollectionConstructor, APathThatIsNotADirectoryRaises) {
    for (const std::string& path :
         {std::string("./test/xml_examples/no-such-directory"), LAST_WINDOW, std::string()}) {
        EXPECT_EQ(
            thrownFirstLine([&] { ScoreCollection collection(path); }),
            "[maiacore] ScoreCollection: '" + path + "' is not a directory, or does not exist")
            << path;
    }
    EXPECT_EQ(thrownFirstLine([&] {
                  ScoreCollection collection(std::vector<std::string>{BACH_DIR, "missing"});
              }),
              "[maiacore] ScoreCollection: 'missing' is not a directory, or does not exist");
}

// Extensions match without regard to case, other files are skipped, subdirectories are read only
// when recursive, and the files load in sorted path order.
TEST(ScoreCollectionConstructor, DiscoveryIgnoresCaseSortsAndRecursesOnRequest) {
    TemporaryDirectory directory;
    directory.addCopy(LAST_WINDOW, "c.MusicXML");
    directory.addCopy(LAST_WINDOW, "a.xml");
    directory.addCopy(LAST_WINDOW, "B.XML");
    directory.addCopy(LAST_WINDOW, "notes.txt");
    directory.addCopy(LAST_WINDOW, "sub/d.xml");
    const std::string path = directory.utf8();

    StdoutCapture quiet;
    EXPECT_EQ(fileNamesOf(ScoreCollection(path)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML"}));
    EXPECT_EQ(fileNamesOf(ScoreCollection(path, true)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML", "d.xml"}));
    EXPECT_EQ(fileNamesOf(ScoreCollection(std::vector<std::string>{path}, true)),
              (std::vector<std::string>{"B.XML", "a.xml", "c.MusicXML", "d.xml"}));
}

// Files whose names have characters outside every ANSI code page load, and keep their names and
// paths as UTF-8.
TEST(ScoreCollectionConstructor, ANonAsciiFileNameLoadsWithItsUtf8Name) {
    TemporaryDirectory directory;
    const std::string portuguese = "can\xC3\xA7\xC3\xA3o.xml";
    const std::string japanese = "\xE6\x97\xA5\xE6\x9C\xAC.xml";
    directory.addCopy(LAST_WINDOW, japanese);
    directory.addCopy(LAST_WINDOW, portuguese);

    StdoutCapture quiet;
    const ScoreCollection collection(directory.utf8());
    EXPECT_EQ(fileNamesOf(collection), (std::vector<std::string>{portuguese, japanese}));
    EXPECT_EQ(collection.getScores().at(1).getFilePath(),
              (directory.path() / std::filesystem::u8path(japanese)).u8string());
}

TEST(ScoreCollectionConstructor, EmptyDirectoryList) {
    ScoreCollection collection(std::vector<std::string>{});

    EXPECT_EQ(collection.getNumDirectories(), 0);
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

// ============================================================================
// Directory Management Tests
// ============================================================================

TEST(ScoreCollectionDirectories, GetDirectoriesPaths) {
    std::vector<std::string> dirs = {BACH_DIR, BEETHOVEN_DIR};
    ScoreCollection collection(dirs);

    std::vector<std::string> retrieved = collection.getDirectoriesPaths();
    EXPECT_EQ(retrieved.size(), 2);
    EXPECT_EQ(retrieved[0], BACH_DIR);
    EXPECT_EQ(retrieved[1], BEETHOVEN_DIR);
}

TEST(ScoreCollectionDirectories, SetDirectoriesPaths) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumDirectories(), 0);

    std::vector<std::string> dirs = {BACH_DIR};
    collection.setDirectoriesPaths(dirs);

    EXPECT_EQ(collection.getNumDirectories(), 1);
    EXPECT_GT(collection.getNumScores(), 0);  // Should auto-load
}

// setDirectoriesPaths() replaces the directories and the scores, those added with addScore()
// included: the Bach directory's two files, then the Beethoven directory's three.
TEST(ScoreCollectionDirectories, SetDirectoriesReloads) {
    ScoreCollection collection(BACH_DIR);
    collection.addScore(LAST_WINDOW);
    ASSERT_EQ(collection.getNumScores(), 3);

    collection.setDirectoriesPaths({BEETHOVEN_DIR});

    EXPECT_EQ(collection.getDirectoriesPaths(), (std::vector<std::string>{BEETHOVEN_DIR}));
    EXPECT_EQ(fileNamesOf(collection),
              (std::vector<std::string>{"Beethoven_quartet_133.xml", "Beethoven_quartet_Op133.xml",
                                        "Symphony_5th_1Mov.xml"}));
}

// A directory that cannot be loaded leaves the collection as it was.
TEST(ScoreCollectionDirectories, AFailedReloadChangesNothing) {
    ScoreCollection collection(BACH_DIR);
    {
        StdoutCapture quiet;
        collection.addScore("./missing.xml");
    }
    const std::vector<std::pair<std::string, std::string>> errors = collection.getLoadErrors();
    ASSERT_EQ(errors.size(), 1u);

    EXPECT_THROW(collection.setDirectoriesPaths({BEETHOVEN_DIR, "missing"}), std::runtime_error);

    EXPECT_EQ(collection.getDirectoriesPaths(), (std::vector<std::string>{BACH_DIR}));
    EXPECT_EQ(fileNamesOf(collection),
              (std::vector<std::string>{"cello_suite_1_violin.xml", "prelude_1_BWV_846.xml"}));
    EXPECT_EQ(collection.getLoadErrors(), errors);
}

namespace {
// The first line of the error that loading a file that is not XML, 'not a score', raises.
std::string notXml(const std::string& path) {
    return "[maiacore] Score: '" + path +
           "' is not well-formed XML: No document element found (byte offset 11)";
}

// The line a load prints when 'failed' of its 'total' files fail.
std::string failedLine(const int failed, const int total) {
    return "[maiacore] ScoreCollection: " + std::to_string(failed) + " of " +
           std::to_string(total) + " files failed to load; see ScoreCollection.getLoadErrors()\n";
}

typedef std::vector<std::pair<std::string, std::string>> LoadErrors;
}  // namespace

// A file that fails to load is skipped and listed with the first line of its error; the files
// that load replace the collection's scores, and one line says how many failed. A load in which
// every file loads empties the list.
TEST(ScoreCollectionDirectories, AFileThatFailsToLoadIsSkippedAndListed) {
    TemporaryDirectory directory;
    directory.addCopy(LAST_WINDOW, "a.xml");
    std::ofstream(directory.path() / "b.xml") << "not a score";
    const std::string broken = (directory.path() / "b.xml").u8string();
    ScoreCollection collection(BACH_DIR);
    EXPECT_EQ(collection.getLoadErrors(), LoadErrors{});

    {
        StdoutCapture capture;
        collection.setDirectoriesPaths({directory.utf8()});
        EXPECT_NE(capture.str().find(failedLine(1, 2)), std::string::npos) << capture.str();
    }

    EXPECT_EQ(collection.getDirectoriesPaths(), (std::vector<std::string>{directory.utf8()}));
    EXPECT_EQ(fileNamesOf(collection), (std::vector<std::string>{"a.xml"}));
    EXPECT_EQ(collection.getLoadErrors(), (LoadErrors{{broken, notXml(broken)}}));

    StdoutCapture quiet;
    collection.setDirectoriesPaths({BACH_DIR});
    EXPECT_EQ(collection.getLoadErrors(), LoadErrors{});
}

// addScore() with a path, or with several, skips each file that fails to load and lists it; the
// list is that of the last load. addScore() with a Score, which loads nothing, leaves it.
TEST(ScoreCollectionScores, AddScoreSkipsAndListsTheFilesThatFailToLoad) {
    ScoreCollection collection;
    StdoutCapture capture;

    collection.addScore("./missing.xml");
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_EQ(collection.getLoadErrors(),
              (LoadErrors{{"./missing.xml", "[maiacore] Score: cannot open './missing.xml'"}}));
    EXPECT_EQ(capture.str(), failedLine(1, 1));

    collection.addScore(std::vector<std::string>{LAST_WINDOW, "./missing.xml", DUPLICATES});
    EXPECT_EQ(fileNamesOf(collection),
              (std::vector<std::string>{"melody_last_window.musicxml",
                                        "melody_duplicate_patterns.musicxml"}));
    EXPECT_EQ(collection.getLoadErrors(),
              (LoadErrors{{"./missing.xml", "[maiacore] Score: cannot open './missing.xml'"}}));

    collection.addScore(Score({"Piano"}, 1));
    EXPECT_EQ(collection.getLoadErrors().size(), 1u);
    collection.addScore(LAST_WINDOW);
    EXPECT_EQ(collection.getLoadErrors(), LoadErrors{});
    EXPECT_EQ(collection.getNumScores(), 4);
}

// A directory whose name and files' names are outside every ANSI code page: its scores load, and
// the paths of the scores and of the files that fail are UTF-8.
TEST(ScoreCollectionDirectories, ANonAsciiDirectoryListsItsFailuresInUtf8) {
    TemporaryDirectory directory;
    const std::string music = "m\xC3\xBAsicas";
    const std::string japanese = "\xE6\x97\xA5\xE6\x9C\xAC.xml";
    directory.addCopy(LAST_WINDOW, music + "/" + japanese);
    directory.addCopy(LAST_WINDOW, music + "/ruim.xml");
    const std::filesystem::path folder = directory.path() / std::filesystem::u8path(music);
    std::ofstream(folder / "ruim.xml", std::ios::trunc) << "not a score";
    const std::string broken = (folder / "ruim.xml").u8string();

    StdoutCapture quiet;
    const ScoreCollection collection(folder.u8string());
    EXPECT_EQ(fileNamesOf(collection), (std::vector<std::string>{japanese}));
    EXPECT_EQ(collection.getScores().at(0).getFilePath(),
              (folder / std::filesystem::u8path(japanese)).u8string());
    EXPECT_EQ(collection.getLoadErrors(), (LoadErrors{{broken, notXml(broken)}}));
}

TEST(ScoreCollectionDirectories, AddDirectory) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.addDirectory(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);

    collection.addDirectory(BEETHOVEN_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 2);
}

TEST(ScoreCollectionDirectories, AddDirectoryDoesNotAutoLoad) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addDirectory(BACH_DIR);

    // addDirectory only adds to list, doesn't load files
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionDirectories, GetNumDirectories) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_EQ(collection.getNumDirectories(), 0);

    collection.addDirectory(BACH_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 1);

    collection.addDirectory(BEETHOVEN_DIR);
    EXPECT_EQ(collection.getNumDirectories(), 2);
}

// ============================================================================
// Score Management Tests
// ============================================================================

TEST(ScoreCollectionScores, AddScoreByObject) {
    ScoreCollection collection(std::vector<std::string>{});
    Score score("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    collection.addScore(score);
    EXPECT_EQ(collection.getNumScores(), 1);
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionScores, AddScoreByFilePath) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, AddScoreByFilePathList) {
    ScoreCollection collection(std::vector<std::string>{});
    std::vector<std::string> files = {"./test/xml_examples/Bach/prelude_1_BWV_846.xml",
                                      "./test/xml_examples/Bach/cello_suite_1_violin.xml"};

    collection.addScore(files);
    EXPECT_EQ(collection.getNumScores(), 2);
}

TEST(ScoreCollectionScores, AddMultipleScoresByObject) {
    ScoreCollection collection(std::vector<std::string>{});

    Score score1("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    Score score2("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection.addScore(score1);
    collection.addScore(score2);

    EXPECT_EQ(collection.getNumScores(), 2);
}

TEST(ScoreCollectionScores, GetNumScores) {
    ScoreCollection collection(BACH_DIR);
    int num_scores = collection.getNumScores();

    EXPECT_GT(num_scores, 0);  // Bach dir should have at least 1 file
}

TEST(ScoreCollectionScores, GetScoresNonConst) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    std::vector<Score>& scores = collection.getScores();
    EXPECT_EQ(scores.size(), 1);

    // Can modify through reference
    scores.clear();
    EXPECT_EQ(collection.getNumScores(), 0);
}

TEST(ScoreCollectionScores, GetScoresConst) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    const ScoreCollection& const_ref = collection;
    const std::vector<Score>& scores = const_ref.getScores();

    EXPECT_EQ(scores.size(), 1);
}

TEST(ScoreCollectionScores, IsEmpty) {
    ScoreCollection collection(std::vector<std::string>{});
    EXPECT_TRUE(collection.isEmpty());

    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    EXPECT_FALSE(collection.isEmpty());
}

TEST(ScoreCollectionScores, Clear) {
    ScoreCollection collection(BACH_DIR);
    EXPECT_GT(collection.getNumScores(), 0);

    collection.clear();
    EXPECT_EQ(collection.getNumScores(), 0);
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionScores, ClearDoesNotAffectDirectories) {
    ScoreCollection collection(BACH_DIR);
    int num_dirs = collection.getNumDirectories();

    collection.clear();

    EXPECT_EQ(collection.getNumDirectories(), num_dirs);  // Directories unchanged
}

TEST(ScoreCollectionScores, RemoveScoreByIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    collection.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    EXPECT_EQ(collection.getNumScores(), 2);

    collection.removeScore(0);
    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, RemoveScoreLastIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
    collection.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection.removeScore(1);  // Remove last
    EXPECT_EQ(collection.getNumScores(), 1);
}

TEST(ScoreCollectionScores, RemoveScoreInvalidIndex) {
    ScoreCollection collection(std::vector<std::string>{});
    collection.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    EXPECT_THROW(collection.removeScore(10), std::out_of_range);
    EXPECT_THROW(collection.removeScore(1), std::out_of_range);
    EXPECT_THROW(collection.removeScore(-1), std::out_of_range);
    EXPECT_EQ(collection.getNumScores(), 1);  // Unchanged
}

// ============================================================================
// Merge Tests
// ============================================================================

TEST(ScoreCollectionMerge, MergeCollections) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumScores(), 2);
}

TEST(ScoreCollectionMerge, MergeDirectories) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addDirectory(BACH_DIR);

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addDirectory(BEETHOVEN_DIR);

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumDirectories(), 2);
}

TEST(ScoreCollectionMerge, MergeBothScoresAndDirectories) {
    ScoreCollection collection1(BACH_DIR);
    int scores1 = collection1.getNumScores();
    int dirs1 = collection1.getNumDirectories();

    ScoreCollection collection2(BEETHOVEN_DIR);
    int scores2 = collection2.getNumScores();
    int dirs2 = collection2.getNumDirectories();

    collection1.merge(collection2);

    EXPECT_EQ(collection1.getNumScores(), scores1 + scores2);
    EXPECT_EQ(collection1.getNumDirectories(), dirs1 + dirs2);
}

TEST(ScoreCollectionMerge, MergeEmptyCollection) {
    ScoreCollection collection1(BACH_DIR);
    int original_count = collection1.getNumScores();

    ScoreCollection empty_collection(std::vector<std::string>{});
    collection1.merge(empty_collection);

    EXPECT_EQ(collection1.getNumScores(), original_count);  // Unchanged
}

TEST(ScoreCollectionMerge, MergeIntoEmptyCollection) {
    ScoreCollection empty_collection(std::vector<std::string>{});
    ScoreCollection collection(BACH_DIR);
    int count = collection.getNumScores();

    empty_collection.merge(collection);

    EXPECT_EQ(empty_collection.getNumScores(), count);
}

// ============================================================================
// Operator Tests
// ============================================================================

TEST(ScoreCollectionOperator, PlusOperator) {
    ScoreCollection collection1(std::vector<std::string>{});
    collection1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection collection2(std::vector<std::string>{});
    collection2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    ScoreCollection merged = collection1 + collection2;

    EXPECT_EQ(merged.getNumScores(), 2);
    // Original collections should be unchanged
    EXPECT_EQ(collection1.getNumScores(), 1);
    EXPECT_EQ(collection2.getNumScores(), 1);
}

TEST(ScoreCollectionOperator, PlusOperatorChaining) {
    ScoreCollection c1(std::vector<std::string>{});
    c1.addScore("./test/xml_examples/Bach/prelude_1_BWV_846.xml");

    ScoreCollection c2(std::vector<std::string>{});
    c2.addScore("./test/xml_examples/Bach/cello_suite_1_violin.xml");

    ScoreCollection c3(std::vector<std::string>{});
    c3.addScore("./test/xml_examples/Beethoven/Beethoven_quartet_133.xml");

    ScoreCollection merged = c1 + c2 + c3;

    EXPECT_EQ(merged.getNumScores(), 3);
}

// ============================================================================
// Pattern Finding Tests - Single Pattern
// ============================================================================

// Each score's matches, with its file name; scores with equal titles keep the collection's order.
TEST(ScoreCollectionPatternFinding, FindMelodyPatternBasic) {
    ScoreCollection collection;
    collection.addScore(LAST_WINDOW);
    collection.addScore(DUPLICATES);

    const auto table = collection.findMelodyPattern({Note("C4"), Note("D4")}, 1.0f, 1.0f);

    EXPECT_EQ(rowsOf(table), (std::vector<std::pair<std::string, std::vector<std::string>>>{
                                 {"melody_last_window.musicxml", {"C4", "D4"}},
                                 {"melody_last_window.musicxml", {"D4", "E4"}},
                                 {"melody_duplicate_patterns.musicxml", {"C4", "D4"}},
                                 {"melody_duplicate_patterns.musicxml", {"C4", "D4"}},
                                 {"melody_duplicate_patterns.musicxml", {"E4", "F#4"}}}));
}

// The rows are sorted stably by score title.
TEST(ScoreCollectionPatternFinding, FindMelodyPatternSortsByScoreTitle) {
    ScoreCollection collection;
    collection.addScore(LAST_WINDOW);
    collection.addScore(DUPLICATES);
    collection.getScores()[0].setTitle("B");
    collection.getScores()[1].setTitle("A");

    const auto table = collection.findMelodyPattern({Note("C4"), Note("D4")}, 1.0f, 1.0f);

    std::vector<std::string> titles;
    for (const auto& row : table) {
        titles.push_back(row.scoreTitle);
    }
    EXPECT_EQ(titles, (std::vector<std::string>{"A", "A", "A", "B", "B"}));
}

// Thresholds of 1 keep only exact matches: D4-E4 in a quarter and a half is no C4-D4 in quarters.
TEST(ScoreCollectionPatternFinding, FindMelodyPatternHighThresholds) {
    ScoreCollection collection;
    collection.addScore(DUPLICATES);

    EXPECT_EQ(collection.findMelodyPattern({Note("C4"), Note("D4")}, 1.0f, 1.0f).size(), 3u);
    EXPECT_EQ(collection.findMelodyPattern({Note("C4"), Note("D4")}, 0.5f, 0.5f).size(), 4u);
}

// An empty collection finds nothing, but a pattern of fewer than 2 notes is rejected all the
// same.
TEST(ScoreCollectionPatternFinding, FindMelodyPatternEmptyCollection) {
    ScoreCollection collection;

    EXPECT_TRUE(collection.findMelodyPattern({Note("C4"), Note("D4")}).empty());
    EXPECT_EQ(thrownFirstLine([&] { collection.findMelodyPattern(std::vector<Note>{Note("C4")}); }),
              "[maiacore] ScoreCollection::findMelodyPattern: a melody pattern needs at least 2 "
              "notes, and this one has 1");
}

// ============================================================================
// Pattern Finding Tests - Multiple Patterns
// ============================================================================

// One table per pattern, each with the pattern's matches in every score.
TEST(ScoreCollectionMultiPattern, FindMultipleMelodyPatterns) {
    ScoreCollection collection;
    collection.addScore(LAST_WINDOW);
    collection.addScore(DUPLICATES);
    const std::vector<std::vector<Note>> patterns = {
        {Note("C4"), Note("D4")}, {Note("E4"), Note("G4")}, {Note("C4"), Note("B3"), Note("A3")}};

    const auto tables = collection.findMelodyPattern(patterns, 1.0f, 1.0f);

    ASSERT_EQ(tables.size(), 3u);
    EXPECT_EQ(tables[0].size(), 5u);
    EXPECT_EQ(rowsOf(tables[1]), (std::vector<std::pair<std::string, std::vector<std::string>>>{
                                     {"melody_last_window.musicxml", {"E4", "G4"}}}));
    EXPECT_TRUE(tables[2].empty());
}

// Each table holds what the single-pattern search of its pattern finds, in the same order.
TEST(ScoreCollectionMultiPattern, EachTableIsTheSinglePatternSearch) {
    ScoreCollection collection;
    collection.addScore(LAST_WINDOW);
    collection.addScore(DUPLICATES);
    collection.getScores()[0].setTitle("B");
    collection.getScores()[1].setTitle("A");
    const std::vector<std::vector<Note>> patterns = {{Note("C4"), Note("D4")},
                                                     {Note("D4"), Note("C4")}};

    const auto tables = collection.findMelodyPattern(patterns, 0.5f, 0.5f);

    ASSERT_EQ(tables.size(), 2u);
    for (size_t p = 0; p < patterns.size(); p++) {
        EXPECT_EQ(rowsOf(tables[p]), rowsOf(collection.findMelodyPattern(patterns[p], 0.5f, 0.5f)))
            << p;
    }
}

// A pattern of fewer than 2 notes is rejected, even in an empty collection.
TEST(ScoreCollectionMultiPattern, AShortPatternIsRejected) {
    ScoreCollection collection;
    EXPECT_THROW(collection.findMelodyPattern(std::vector<std::vector<Note>>{{Note("C4")}}),
                 std::runtime_error);
}

TEST(ScoreCollectionMultiPattern, EmptyPatternList) {
    ScoreCollection collection(BACH_DIR);

    std::vector<std::vector<Note>> empty_patterns;

    auto results = collection.findMelodyPattern(empty_patterns);

    EXPECT_EQ(results.size(), 0);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(ScoreCollectionIntegration, FullWorkflow) {
    // Create collection with directory
    ScoreCollection collection(BACH_DIR);
    int initial_count = collection.getNumScores();
    EXPECT_GT(initial_count, 0);

    // Add more scores manually
    collection.addScore("./test/xml_examples/Beethoven/Beethoven_quartet_133.xml");
    EXPECT_EQ(collection.getNumScores(), initial_count + 1);

    // Remove a score
    collection.removeScore(0);
    EXPECT_EQ(collection.getNumScores(), initial_count);

    // Clear all
    collection.clear();
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionIntegration, MergeAndSearch) {
    ScoreCollection collection1(BACH_DIR);
    ScoreCollection collection2(BEETHOVEN_DIR);

    ScoreCollection merged = collection1 + collection2;

    EXPECT_GT(merged.getNumScores(), 0);
    EXPECT_EQ(merged.getNumDirectories(), 2);

    // A merged collection searches the scores of both
    ScoreCollection first;
    first.addScore(LAST_WINDOW);
    ScoreCollection second;
    second.addScore(DUPLICATES);
    const auto table = (first + second).findMelodyPattern({Note("C4"), Note("D4")}, 1.0f, 1.0f);
    EXPECT_EQ(table.size(), 5u);
}

TEST(ScoreCollectionIntegration, SetDirectoriesMultipleTimes) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.setDirectoriesPaths({BACH_DIR});
    EXPECT_EQ(collection.getNumScores(), 2);

    collection.setDirectoriesPaths({BEETHOVEN_DIR});
    EXPECT_EQ(collection.getNumScores(), 3);

    collection.setDirectoriesPaths({BEETHOVEN_DIR});
    EXPECT_EQ(collection.getNumScores(), 3);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(ScoreCollectionEdgeCases, RemoveFromEmptyCollection) {
    ScoreCollection collection(std::vector<std::string>{});

    EXPECT_THROW(collection.removeScore(0), std::out_of_range);
    EXPECT_EQ(collection.getNumScores(), 0);
}

TEST(ScoreCollectionEdgeCases, ClearEmptyCollection) {
    ScoreCollection collection(std::vector<std::string>{});

    collection.clear();  // Should not crash
    EXPECT_TRUE(collection.isEmpty());
}

TEST(ScoreCollectionEdgeCases, MergeWithSelf) {
    ScoreCollection collection(BACH_DIR);
    int original_count = collection.getNumScores();

    collection.merge(collection);  // Merge with itself

    EXPECT_EQ(collection.getNumScores(), original_count * 2);  // Should duplicate
}

TEST(ScoreCollectionEdgeCases, LargeCollection) {
    ScoreCollection collection(UNIT_TEST_DIR);

    // Unit test dir has many files - test performance doesn't degrade
    EXPECT_GT(collection.getNumScores(), 10);

    // Clear should work efficiently
    collection.clear();
    EXPECT_TRUE(collection.isEmpty());
}

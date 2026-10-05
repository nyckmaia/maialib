#include "maiacore/score.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <set>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "maiacore/helper.h"
#include "maiacore/note.h"
#include "maiacore/part.h"
#include "test-capture.h"
#include "test-locale.h"
#include "transposing-instruments.h"

using namespace testing;

// ====================
// Constructor Tests
// ====================

TEST(ScoreConstructor, DefaultConstructorWithInitializerList) {
    Score score({"Piano", "Violin"}, 10);

    EXPECT_EQ(score.getNumParts(), 2);
    EXPECT_EQ(score.getNumMeasures(), 10);
    EXPECT_EQ(score.getTitle(), "");
    EXPECT_EQ(score.getComposerName(), "");

    std::vector<std::string> partNames = score.getPartsNames();
    EXPECT_EQ(partNames.size(), 2);
    EXPECT_EQ(partNames[0], "Piano");
    EXPECT_EQ(partNames[1], "Violin");
}

TEST(ScoreConstructor, DefaultConstructorWithVector) {
    std::vector<std::string> parts = {"Flute", "Clarinet", "Bassoon"};
    Score score(parts, 5);

    EXPECT_EQ(score.getNumParts(), 3);
    EXPECT_EQ(score.getNumMeasures(), 5);

    std::vector<std::string> partNames = score.getPartsNames();
    EXPECT_EQ(partNames.size(), 3);
    EXPECT_EQ(partNames[0], "Flute");
    EXPECT_EQ(partNames[1], "Clarinet");
    EXPECT_EQ(partNames[2], "Bassoon");
}

TEST(ScoreConstructor, LoadFromXMLFile) {
    Score score("./test/xml_examples/unit_test/test_chord.xml");

    EXPECT_TRUE(score.isValid());
    EXPECT_GT(score.getNumParts(), 0);
    EXPECT_GT(score.getNumMeasures(), 0);
    EXPECT_EQ(score.getFileName(), "test_chord.xml");
}

TEST(ScoreConstructor, LoadFromXMLFileSimpleMelody) {
    Score score("./test/xml_examples/unit_test/test_musical_scale.xml");

    EXPECT_TRUE(score.isValid());
    EXPECT_GT(score.getNumNotes(), 0);
}

// ====================
// Part Management Tests
// ====================

TEST(ScorePartManagement, AddPart) {
    Score score({"Piano"}, 5);

    EXPECT_EQ(score.getNumParts(), 1);

    score.addPart("Violin");
    EXPECT_EQ(score.getNumParts(), 2);

    std::vector<std::string> partNames = score.getPartsNames();
    EXPECT_EQ(partNames[1], "Violin");
}

TEST(ScorePartManagement, AddPartWithMultipleStaves) {
    Score score({"Melody"}, 4);

    score.addPart("Piano", 2);  // Piano with 2 staves
    EXPECT_EQ(score.getNumParts(), 2);

    Part& pianoPart = score.getPart(1);
    EXPECT_EQ(pianoPart.getName(), "Piano");
}

TEST(ScorePartManagement, RemovePart) {
    Score score({"Piano", "Violin", "Cello"}, 5);

    EXPECT_EQ(score.getNumParts(), 3);

    score.removePart(1);  // Remove Violin
    EXPECT_EQ(score.getNumParts(), 2);

    std::vector<std::string> partNames = score.getPartsNames();
    EXPECT_EQ(partNames[0], "Piano");
    EXPECT_EQ(partNames[1], "Cello");
}

TEST(ScorePartManagement, GetPartByIndex) {
    Score score({"Piano", "Violin"}, 5);

    Part& part0 = score.getPart(0);
    EXPECT_EQ(part0.getName(), "Piano");

    Part& part1 = score.getPart(1);
    EXPECT_EQ(part1.getName(), "Violin");
}

TEST(ScorePartManagement, GetPartByName) {
    Score score({"Piano", "Violin", "Cello"}, 5);

    Part& pianoPart = score.getPart("Piano");
    EXPECT_EQ(pianoPart.getName(), "Piano");

    Part& celloPart = score.getPart("Cello");
    EXPECT_EQ(celloPart.getName(), "Cello");
}

// ====================
// Measure Management Tests
// ====================

TEST(ScoreMeasureManagement, AddMeasures) {
    Score score({"Piano"}, 5);

    EXPECT_EQ(score.getNumMeasures(), 5);

    score.addMeasure(3);
    EXPECT_EQ(score.getNumMeasures(), 8);

    score.addMeasure(2);
    EXPECT_EQ(score.getNumMeasures(), 10);
}

TEST(ScoreMeasureManagement, RemoveMeasures) {
    Score score({"Piano"}, 10);

    EXPECT_EQ(score.getNumMeasures(), 10);

    score.removeMeasure(5, 7);  // Remove measures 5, 6, 7
    EXPECT_EQ(score.getNumMeasures(), 7);
}

TEST(ScoreMeasureManagement, RemoveSingleMeasure) {
    Score score({"Piano"}, 10);

    score.removeMeasure(3, 3);  // Remove only measure 3
    EXPECT_EQ(score.getNumMeasures(), 9);
}

// ====================
// Metadata Tests
// ====================

TEST(ScoreMetadata, SetAndGetTitle) {
    Score score({"Piano"}, 5);

    EXPECT_EQ(score.getTitle(), "");

    score.setTitle("Moonlight Sonata");
    EXPECT_EQ(score.getTitle(), "Moonlight Sonata");

    score.setTitle("Symphony No. 5");
    EXPECT_EQ(score.getTitle(), "Symphony No. 5");
}

TEST(ScoreMetadata, SetAndGetComposer) {
    Score score({"Piano"}, 5);

    EXPECT_EQ(score.getComposerName(), "");

    score.setComposerName("Ludwig van Beethoven");
    EXPECT_EQ(score.getComposerName(), "Ludwig van Beethoven");

    score.setComposerName("Wolfgang Amadeus Mozart");
    EXPECT_EQ(score.getComposerName(), "Wolfgang Amadeus Mozart");
}

// NOTE: This test is temporarily disabled due to pugixml issues with this specific Bach file
// TODO: Investigate and fix the remaining pugixml access issues
// TEST(ScoreMetadata, TitleAndComposerFromLoadedXML) {
//   Score score("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
//
//   EXPECT_TRUE(score.isValid());
//   // The XML file may or may not have title/composer metadata
//   // Just verify the getters work without crashing
//   std::string title = score.getTitle();
//   std::string composer = score.getComposerName();
// }

// ====================
// Key Signature Tests
// ====================

TEST(ScoreKeySignature, SetKeySignatureWithFifths) {
    Score score({"Piano"}, 5);

    // C major (0 accidentals)
    score.setKeySignature(0, true, 0);

    // G major (1 sharp)
    score.setKeySignature(1, true, 0);

    // D minor (1 flat)
    score.setKeySignature(-1, false, 0);
}

TEST(ScoreKeySignature, SetKeySignatureWithKeyName) {
    Score score({"Piano"}, 5);

    score.setKeySignature("C", 0);
    score.setKeySignature("G", 0);
    score.setKeySignature("Am", 0);
    score.setKeySignature("Dm", 0);
}

// ====================
// Time Signature Tests
// ====================

TEST(ScoreTimeSignature, SetTimeSignatureAllMeasures) {
    Score score({"Piano"}, 10);

    // 4/4 time for all measures
    score.setTimeSignature(4, 4, -1);

    // Verify first measure has correct time signature
    Part& part = score.getPart(0);
    Measure& measure = part.getMeasure(0);
    EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 4);
    EXPECT_EQ(measure.getTimeSignature().getLowerValue(), 4);
}

TEST(ScoreTimeSignature, SetTimeSignatureFromSpecificMeasure) {
    Score score({"Piano"}, 10);

    // 4/4 for all measures
    score.setTimeSignature(4, 4, -1);

    // Change to 3/4 starting from measure 5
    score.setTimeSignature(3, 4, 5);

    Part& part = score.getPart(0);

    // Measure 4 should still be 4/4
    Measure& measure4 = part.getMeasure(4);
    EXPECT_EQ(measure4.getTimeSignature().getUpperValue(), 4);

    // Measure 5 should be 3/4
    Measure& measure5 = part.getMeasure(5);
    EXPECT_EQ(measure5.getTimeSignature().getUpperValue(), 3);
}

TEST(ScoreTimeSignature, VariousTimeSignatures) {
    Score score({"Piano"}, 5);

    // Test various time signatures
    score.setTimeSignature(2, 4, 0);  // 2/4
    score.setTimeSignature(3, 8, 1);  // 3/8
    score.setTimeSignature(6, 8, 2);  // 6/8
    score.setTimeSignature(5, 4, 3);  // 5/4
    score.setTimeSignature(7, 8, 4);  // 7/8
}

// ====================
// Metronome Mark Tests
// ====================

TEST(ScoreMetronomeMark, SetMetronomeMarkQuarter) {
    Score score({"Piano"}, 5);

    // 120 BPM, quarter note
    score.setMetronomeMark(120, RhythmFigure::QUARTER, 0);
}

TEST(ScoreMetronomeMark, SetMetronomeMarkVariousRhythms) {
    Score score({"Piano"}, 5);

    score.setMetronomeMark(60, RhythmFigure::HALF, 0);
    score.setMetronomeMark(200, RhythmFigure::EIGHTH, 1);
    score.setMetronomeMark(90, RhythmFigure::QUARTER, 2);
}

// ====================
// Clear and Reset Tests
// ====================

TEST(ScoreClear, ClearScore) {
    Score score({"Piano", "Violin"}, 10);
    score.setTitle("Test Symphony");
    score.setComposerName("Test Composer");

    EXPECT_EQ(score.getNumParts(), 2);
    EXPECT_EQ(score.getNumMeasures(), 10);

    score.clear();

    EXPECT_EQ(score.getNumParts(), 0);
    EXPECT_EQ(score.getTitle(), "");
    EXPECT_EQ(score.getComposerName(), "");
}

// ====================
// Export Tests
// ====================

TEST(ScoreExport, ToXML) {
    Score score({"Piano"}, 2);
    score.setTitle("Test Score");
    score.setComposerName("Test Composer");
    score.setTimeSignature(4, 4, -1);

    std::string xml = score.toXML();

    // Verify XML contains expected elements
    EXPECT_NE(xml.find("<?xml"), std::string::npos);
    EXPECT_NE(xml.find("<score-partwise"), std::string::npos);
    EXPECT_NE(xml.find("Test Score"), std::string::npos);
    EXPECT_NE(xml.find("Test Composer"), std::string::npos);
}

// TODO: Implement Score::toJSON() method before enabling this test
// The current implementation returns an empty string (stub)
// TEST(ScoreExport, ToJSON) {
//   Score score({"Piano"}, 2);
//   score.setTitle("Test Score");
//
//   std::string json = score.toJSON();
//
//   // Verify JSON is not empty and contains braces
//   EXPECT_GT(json.length(), 0);
//   EXPECT_NE(json.find("{"), std::string::npos);
//   EXPECT_NE(json.find("}"), std::string::npos);
// }

// NOTE: This test is temporarily disabled due to "invalid vector subscript" error when loading
// the exported XML file. The file exports successfully but loading it back causes a crash,
// likely related to remaining pugixml bugs in the XML parsing code.
// TODO: Fix remaining vector access issues in Score::loadXMLFile()
// TEST(ScoreExport, ToFileXML) {
//   Score score({"Piano"}, 2);
//   score.setTitle("Test Export");
//   score.setComposerName("Test");
//
//   // Add a note to make the score valid
//   score.getPart(0).getMeasure(0).addNote(Note("C4"), 0);
//
//   // Export to XML file (toFile adds .xml extension automatically)
//   score.toFile("test_export_score", false);
//
//   // Load it back and verify
//   Score loadedScore("test_export_score.xml");
//   EXPECT_TRUE(loadedScore.isValid());
//   EXPECT_EQ(loadedScore.getTitle(), "Test Export");
// }

// ====================
// File Loading Tests
// ====================

TEST(ScoreFileLoading, LoadValidXMLFile) {
    Score score("./test/xml_examples/unit_test/test_chord.xml");

    EXPECT_TRUE(score.isValid());
    EXPECT_EQ(score.getFileName(), "test_chord.xml");
    EXPECT_NE(score.getFilePath(), "");
}

TEST(ScoreFileLoading, LoadXMLWithMultipleInstruments) {
    Score score("./test/xml_examples/unit_test/test_multiple_instruments1.xml");

    EXPECT_TRUE(score.isValid());
    EXPECT_GT(score.getNumParts(), 1);
}

TEST(ScoreFileLoading, LoadXMLWithMultipleVoices) {
    Score score("./test/xml_examples/unit_test/test_multiple_voices.xml");

    EXPECT_TRUE(score.isValid());
}

TEST(ScoreFileLoading, LoadXMLWithKeySignature) {
    Score score("./test/xml_examples/unit_test/test_key_signature.xml");

    EXPECT_TRUE(score.isValid());
}

TEST(ScoreFileLoading, LoadXMLWithTimeSignature) {
    Score score("./test/xml_examples/unit_test/test_time_signature.xml");

    EXPECT_TRUE(score.isValid());
}

TEST(ScoreFileLoading, LoadXMLWithTriplets) {
    Score score("./test/xml_examples/unit_test/test_triplets.xml");

    EXPECT_TRUE(score.isValid());
}

TEST(ScoreFileLoading, LoadXMLWithSlurs) {
    Score score("./test/xml_examples/unit_test/test_slur.xml");

    EXPECT_TRUE(score.isValid());
}

TEST(ScoreFileLoading, LoadXMLWithTies) {
    Score score("./test/xml_examples/unit_test/test_ties.xml");

    EXPECT_TRUE(score.isValid());
}

TEST(ScoreFileLoading, LoadXMLWithArticulations) {
    Score score("./test/xml_examples/unit_test/test_articulation.xml");

    EXPECT_TRUE(score.isValid());
}

TEST(ScoreFileLoading, LoadXMLWithDynamics) {
    Score score("./test/xml_examples/unit_test/test_dynamics.xml");

    EXPECT_TRUE(score.isValid());
}

// ====================
// Note Iteration Tests
// ====================

TEST(ScoreNoteIteration, ForEachNoteAllNotes) {
    Score score("./test/xml_examples/unit_test/test_musical_scale.xml");

    int noteCount = 0;
    score.forEachNote([&noteCount](Part*, Measure*, int, Note* note) {
        noteCount++;
        EXPECT_NE(note, nullptr);
    });

    EXPECT_GT(noteCount, 0);
    EXPECT_EQ(noteCount, score.getNumNotes());
}

TEST(ScoreNoteIteration, ForEachNoteMeasureRange) {
    Score score("./test/xml_examples/unit_test/test_multiples_measures.xml");

    int noteCount = 0;
    score.forEachNote([&noteCount](Part*, Measure*, int, Note*) { noteCount++; }, 0,
                      2);  // Only measures 0, 1, 2

    EXPECT_GT(noteCount, 0);
}

TEST(ScoreNoteIteration, ForEachNoteModifyPitch) {
    Score score({"Piano"}, 2);
    Part& part = score.getPart(0);

    // Add some notes
    part.getMeasure(0).addNote(Note("C4"), 0);
    part.getMeasure(0).addNote(Note("E4"), 0);
    part.getMeasure(1).addNote(Note("G4"), 0);

    // Transpose all notes up one octave
    score.forEachNote([](Part*, Measure*, int, Note* note) {
        if (note->isNoteOn()) {
            int currentOctave = note->getOctave().value();  // guarded by isNoteOn() above
            note->setOctave(currentOctave + 1);
        }
    });

    // Verify transposition (getNote signature is: noteId, staveId)
    EXPECT_EQ(part.getMeasure(0).getNote(0, 0).getPitch(), "C5");
    EXPECT_EQ(part.getMeasure(0).getNote(1, 0).getPitch(), "E5");
    EXPECT_EQ(part.getMeasure(1).getNote(0, 0).getPitch(), "G5");
}

// ====================
// Validation Tests
// ====================

TEST(ScoreValidation, IsValidAfterConstruction) {
    Score score({"Piano"}, 5);
    // Note: Score constructed programmatically may not set isValid flag
    // This depends on implementation
}

TEST(ScoreValidation, IsValidAfterLoadingXML) {
    Score score("./test/xml_examples/unit_test/test_chord.xml");
    EXPECT_TRUE(score.isValid());
}

TEST(ScoreValidation, HaveTypeTag) {
    Score score("./test/xml_examples/unit_test/test_musical_scale.xml");
    // Just verify the method doesn't crash
    score.haveTypeTag();
}

// ====================
// Note Count Tests
// ====================

TEST(ScoreNoteCount, GetNumNotesEmptyScore) {
    Score score({"Piano"}, 5);
    EXPECT_EQ(score.getNumNotes(), 0);
}

TEST(ScoreNoteCount, GetNumNotesAfterAddingNotes) {
    Score score({"Piano"}, 2);
    Part& part = score.getPart(0);

    part.getMeasure(0).addNote(Note("C4"), 0);
    part.getMeasure(0).addNote(Note("E4"), 0);
    part.getMeasure(0).addNote(Note("G4"), 0);
    part.getMeasure(1).addNote(Note("A4"), 0);

    EXPECT_EQ(score.getNumNotes(), 4);
}

TEST(ScoreNoteCount, GetNumNotesFromLoadedXML) {
    Score score("./test/xml_examples/unit_test/test_musical_scale.xml");
    EXPECT_GT(score.getNumNotes(), 0);
}

// ====================
// Instrument Fragmentation Tests
// ====================

TEST(InstrumentFragmentation, SucceedsWhenSectionHasNoNotesButAttributesArePresent) {
    // Pins the OOB guard at score.cpp ('maxNotes > 0 && get_sign[maxNotes - 1]').
    // 'maxNotes' -- the number of notes the section's XPath query matches -- can legitimately be
    // 0, and 'get_sign'/'activations_vec' are then empty vectors.
    //
    // A purely programmatic Score (no XML loaded) would not do. It also has 'maxNotes == 0', but
    // its pugixml document is entirely empty, so the function throws moments later on the
    // unrelated "beatNumber is empty" check whether or not the guard is present: such a test would
    // pass against unguarded code too, since 'get_sign[maxNotes - 1]' with 'maxNotes == 0' is
    // undefined behavior (not a guaranteed trap), and control can appear to "fall through" to that
    // same later throw either way.
    //
    // 'zero_notes_measure.xml' is hand-authored to separate the two cases: a single measure
    // with a complete <attributes> block (divisions/time/clef) but zero <note> children. maiacore
    // itself would never write such a measure -- Measure::toXML() always emits a whole-measure
    // rest <note> for an empty measure -- so this had to be crafted by hand to reach
    // 'maxNotes == 0' while still letting 'beatNumber'/'divisions'/'beatType' (which read
    // part[1]/measure[1] unconditionally) resolve successfully. That lets execution run all the
    // way to a normal return, so this test actually distinguishes guarded from unguarded code:
    // guarded code returns cleanly; unguarded code reads 'get_sign[-1]' -- index -1 into a
    // default-constructed, zero-size std::vector<int>, whose data() is null on every standard
    // library this project targets, i.e. a near-guaranteed segfault, not a maybe -- right here.
    Score score("./test/xml_examples/unit_test/zero_notes_measure.xml");
    ASSERT_TRUE(score.isValid());
    ASSERT_EQ(score.getNumNotes(), 0);

    nlohmann::json result;
    EXPECT_NO_THROW(result = score.instrumentFragmentation());

    ASSERT_TRUE(result.contains("element"));
    ASSERT_EQ(result["element"].size(), 1);
    EXPECT_EQ(result["element"][0]["Number of Activations"], 0);
}

// ====================
// Complex Score Tests
// ====================

TEST(ScoreComplex, MultiplePartsAndMeasures) {
    Score score({"Violin", "Viola", "Cello"}, 8);

    score.setTitle("String Trio");
    score.setComposerName("Test Composer");
    score.setTimeSignature(3, 4, -1);
    score.setKeySignature("G", 0);

    EXPECT_EQ(score.getNumParts(), 3);
    EXPECT_EQ(score.getNumMeasures(), 8);

    // Add notes to each part
    for (int partIdx = 0; partIdx < 3; partIdx++) {
        Part& part = score.getPart(partIdx);
        part.getMeasure(0).addNote(Note("G4"), 0);
    }

    EXPECT_EQ(score.getNumNotes(), 3);
}

// NOTE: Disabled - same Bach file has pugixml issues
// TODO: Fix remaining XML parsing issues
// TEST(ScoreComplex, LoadBachPrelude) {
//   Score score("./test/xml_examples/Bach/prelude_1_BWV_846.xml");
//
//   EXPECT_TRUE(score.isValid());
//   EXPECT_GT(score.getNumNotes(), 0);
//   EXPECT_GT(score.getNumMeasures(), 0);
// }

TEST(ScoreComplex, LoadAndExportRoundTrip) {
    Score score1("./test/xml_examples/unit_test/test_chord.xml");
    EXPECT_TRUE(score1.isValid());

    int originalNotes = score1.getNumNotes();
    int originalMeasures = score1.getNumMeasures();
    int originalParts = score1.getNumParts();

    // toFile appends the .xml extension. The export goes to the temporary directory, not to the
    // working directory, which is the repository root when the tests run.
    const std::filesystem::path exportBase =
        std::filesystem::temp_directory_path() / "maialib_load_export_round_trip";
    const std::string exportedFile = exportBase.string() + ".xml";
    score1.toFile(exportBase.string(), false);

    Score score2(exportedFile);
    EXPECT_TRUE(score2.isValid());

    EXPECT_EQ(score2.getNumNotes(), originalNotes);
    EXPECT_EQ(score2.getNumMeasures(), originalMeasures);
    EXPECT_EQ(score2.getNumParts(), originalParts);

    std::filesystem::remove(exportedFile);
}

// ====================
// Copy Constructor and Assignment Operator Tests
// ====================

TEST(ScoreCopySemantics, CopyConstructorBasicProperties) {
    // Create original score with properties
    Score original({"Piano", "Violin"}, 8);
    original.setTitle("Test Symphony");
    original.setComposerName("Test Composer");

    // Copy using copy constructor
    Score copy(original);

    // Verify all basic properties are copied
    EXPECT_EQ(copy.getTitle(), "Test Symphony");
    EXPECT_EQ(copy.getComposerName(), "Test Composer");
    EXPECT_EQ(copy.getNumParts(), 2);
    EXPECT_EQ(copy.getNumMeasures(), 8);

    // Verify part names are copied
    std::vector<std::string> copyPartNames = copy.getPartsNames();
    EXPECT_EQ(copyPartNames.size(), 2);
    EXPECT_EQ(copyPartNames[0], "Piano");
    EXPECT_EQ(copyPartNames[1], "Violin");
}

TEST(ScoreCopySemantics, CopyConstructorFromLoadedXML) {
    // Load a score from XML
    Score original("./test/xml_examples/unit_test/test_chord.xml");
    EXPECT_TRUE(original.isValid());

    std::string originalTitle = original.getTitle();
    std::string originalComposer = original.getComposerName();
    std::string originalFileName = original.getFileName();
    std::string originalFilePath = original.getFilePath();
    int originalNotes = original.getNumNotes();
    int originalMeasures = original.getNumMeasures();
    int originalParts = original.getNumParts();

    // Copy using copy constructor
    Score copy(original);

    // Verify all properties are copied
    EXPECT_EQ(copy.getTitle(), originalTitle);
    EXPECT_EQ(copy.getComposerName(), originalComposer);
    EXPECT_EQ(copy.getFileName(), originalFileName);
    EXPECT_EQ(copy.getFilePath(), originalFilePath);
    EXPECT_EQ(copy.getNumNotes(), originalNotes);
    EXPECT_EQ(copy.getNumMeasures(), originalMeasures);
    EXPECT_EQ(copy.getNumParts(), originalParts);
    EXPECT_TRUE(copy.isValid());
}

TEST(ScoreCopySemantics, CopyConstructorIndependence) {
    // Create original score
    Score original({"Piano"}, 4);
    original.setTitle("Original Title");

    // Copy using copy constructor
    Score copy(original);

    // Modify original
    original.setTitle("Modified Title");
    original.addPart("Violin");

    // Verify copy is independent (title should remain unchanged)
    EXPECT_EQ(copy.getTitle(), "Original Title");
    EXPECT_EQ(copy.getNumParts(), 1);

    // Verify original was modified
    EXPECT_EQ(original.getTitle(), "Modified Title");
    EXPECT_EQ(original.getNumParts(), 2);
}

TEST(ScoreCopySemantics, AssignmentOperatorBasicProperties) {
    // Create original score
    Score original({"Flute", "Clarinet"}, 6);
    original.setTitle("Wind Ensemble");
    original.setComposerName("Composer A");

    // Create another score and assign
    Score assigned({"Cello"}, 3);
    assigned.setTitle("String Piece");

    assigned = original;

    // Verify all properties are assigned
    EXPECT_EQ(assigned.getTitle(), "Wind Ensemble");
    EXPECT_EQ(assigned.getComposerName(), "Composer A");
    EXPECT_EQ(assigned.getNumParts(), 2);
    EXPECT_EQ(assigned.getNumMeasures(), 6);

    // Verify part names
    std::vector<std::string> assignedPartNames = assigned.getPartsNames();
    EXPECT_EQ(assignedPartNames.size(), 2);
    EXPECT_EQ(assignedPartNames[0], "Flute");
    EXPECT_EQ(assignedPartNames[1], "Clarinet");
}

TEST(ScoreCopySemantics, AssignmentOperatorFromLoadedXML) {
    // Load a score from XML
    Score original("./test/xml_examples/unit_test/test_chord.xml");
    EXPECT_TRUE(original.isValid());

    std::string originalTitle = original.getTitle();
    std::string originalFileName = original.getFileName();
    int originalNotes = original.getNumNotes();

    // Create another score and assign
    Score assigned({"Piano"}, 1);
    assigned = original;

    // Verify all properties are assigned
    EXPECT_EQ(assigned.getTitle(), originalTitle);
    EXPECT_EQ(assigned.getFileName(), originalFileName);
    EXPECT_EQ(assigned.getNumNotes(), originalNotes);
    EXPECT_TRUE(assigned.isValid());
}

TEST(ScoreCopySemantics, AssignmentOperatorIndependence) {
    // Create original score
    Score original({"Trumpet"}, 5);
    original.setTitle("Brass Fanfare");

    // Create another score and assign
    Score assigned({"Tuba"}, 2);
    assigned = original;

    // Modify original
    original.setTitle("Modified Fanfare");
    original.addPart("Trombone");

    // Verify assigned score is independent
    EXPECT_EQ(assigned.getTitle(), "Brass Fanfare");
    EXPECT_EQ(assigned.getNumParts(), 1);

    // Verify original was modified
    EXPECT_EQ(original.getTitle(), "Modified Fanfare");
    EXPECT_EQ(original.getNumParts(), 2);
}

TEST(ScoreCopySemantics, AssignmentOperatorSelfAssignment) {
    // Create a score
    Score score({"Piano"}, 4);
    score.setTitle("Self Test");

    // Self-assignment should be safe. The cast changes nothing; it only keeps clang's
    // -Wself-assign-overloaded from reporting this intended self-assignment.
    score = static_cast<const Score&>(score);

    // Verify score is unchanged
    EXPECT_EQ(score.getTitle(), "Self Test");
    EXPECT_EQ(score.getNumParts(), 1);
    EXPECT_EQ(score.getNumMeasures(), 4);
}

// ====================
// Quarter Tone MusicXML Read Tests
// ====================

TEST(ScoreQuarterToneRead, TartiniAccidentalWithMatchingAlter) {
    // <alter>0.5</alter> + <accidental>quarter-sharp</accidental> together: the
    // <accidental> branch must win the spelling AND still produce the numeric value.
    Score score("./test/xml_examples/unit_test/quarter_tone_tartini.xml");

    ASSERT_TRUE(score.isValid());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
}

TEST(ScoreQuarterToneRead, ArrowAccidentalNoAlter) {
    // <accidental>sharp-down</accidental>, no <alter> at all (the arrow family).
    Score score("./test/xml_examples/unit_test/quarter_tone_arrow.xml");

    ASSERT_TRUE(score.isValid());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
}

TEST(ScoreQuarterToneRead, AccidentalOnlyNoAlterMuseScoreCase) {
    // <accidental>quarter-sharp</accidental>, no <alter> at all (the MuseScore case).
    Score score("./test/xml_examples/unit_test/quarter_tone_accidental_only.xml");

    ASSERT_TRUE(score.isValid());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
}

TEST(ScoreQuarterToneRead, UnrecognisedAccidentalNameFallsBackToAlterInsteadOfAborting) {
    // <accidental>natural-sharp</accidental> is a real MusicXML name outside the 13 this
    // library spells. It carries a usable <alter>1</alter>, so the whole load must not abort;
    // it must degrade to the <alter> value, same as if no <accidental> had been present.
    Score score("./test/xml_examples/unit_test/quarter_tone_unknown_accidental_name.xml");

    ASSERT_TRUE(score.isValid());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C#4");
}

TEST(ScoreQuarterToneRead, UnrepresentableAlterTripleSharpFallsBackToNatural) {
    // <alter>3</alter>, no <accidental> at all. A triple sharp is outside the nine values
    // Helper::alterValue2symbol() can spell -- must load as natural with a warning, not throw.
    Score score("./test/xml_examples/unit_test/unrepresentable_alter_triple_sharp.xml");

    ASSERT_TRUE(score.isValid());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C4");
}

TEST(ScoreQuarterToneRead, UnrepresentableAlterEighthToneFallsBackToNatural) {
    // <alter>0.25</alter>, no <accidental> at all: a non-quarter-tone microtonal value,
    // also outside the nine spellable values -- same expectation as the triple-sharp case.
    Score score("./test/xml_examples/unit_test/unrepresentable_alter_eighth_tone.xml");

    ASSERT_TRUE(score.isValid());
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C4");
}

namespace {
// The written pitches of every note on stave 0 of a score's first part, measure by measure.
std::vector<std::string> writtenPitches(Score& score) {
    Part& part = score.getPart(0);
    std::vector<std::string> pitches;
    for (int m = 0; m < part.getNumMeasures(); m++) {
        const Measure& measure = part.getMeasure(m);
        for (int i = 0; i < measure.getNumNotes(0); i++) {
            pitches.push_back(measure.getNote(i, 0).getWrittenPitch());
        }
    }
    return pitches;
}

const std::vector<std::string> kAlterOnlyPitches = {"C1x4", "E3b4"};

// Loads a one-note score whose only note is a C4 with the given <alter> text and no
// <accidental>, and returns its pitch and what the load printed.
std::pair<std::string, std::string> loadWithAlterText(const std::string& alterText) {
    std::ifstream in("./test/xml_examples/unit_test/unrepresentable_alter_eighth_tone.xml");
    std::stringstream source;
    source << in.rdbuf();
    std::string xml = source.str();
    const std::string original = "<alter>0.25</alter>";
    for (size_t at = xml.find(original); at != std::string::npos; at = xml.find(original, at)) {
        xml.replace(at, original.size(), "<alter>" + alterText + "</alter>");
        at += alterText.size();
    }

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "maialib_alter_text.xml";
    {
        std::ofstream out(path);
        out << xml;
    }

    std::string pitch;
    std::string printed;
    {
        StdoutCapture capture;
        Score score(path.string());
        pitch = score.getPart(0).getMeasure(0).getNote(0, 0).getPitch();
        printed = capture.str();
    }
    std::filesystem::remove(path);
    return {pitch, printed};
}
}  // namespace

// A quarter tone given by <alter> alone, with no <accidental>: the form a quarter tone takes when
// its accidental carries through the measure.
TEST(ScoreQuarterToneRead, AlterWithoutAccidentalIsReadAsAQuarterTone) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// The C library's atof() follows setlocale(): under a comma-decimal LC_NUMERIC it stops at the
// '.' of "0.5". The reader parses <alter> in the classic locale instead.
TEST(ScoreQuarterToneRead, AlterIsReadTheSameUnderACommaDecimalCLocale) {
    const std::string localeName = installedCommaDecimalLocale();
    if (localeName.empty()) {
        GTEST_SKIP() << "no comma-decimal locale is installed";
    }

    const ScopedCNumericLocale commaLocale(localeName);
    ASSERT_TRUE(commaLocale.active());
    char formatted[16];
    std::snprintf(formatted, sizeof(formatted), "%.1f", 0.5);
    ASSERT_EQ(std::string(formatted), "0,5") << "the locale does not use a decimal comma";

    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// The reader's string stream is imbued with the classic locale, so a comma-decimal global C++
// locale, which a stream would otherwise pick up, does not change how <alter> is read either.
TEST(ScoreQuarterToneRead, AlterIsReadTheSameUnderACommaDecimalGlobalCppLocale) {
    const std::string localeName = installedCommaDecimalLocale();
    if (localeName.empty()) {
        GTEST_SKIP() << "no comma-decimal locale is installed";
    }

    const ScopedGlobalLocale commaLocale(localeName);
    std::istringstream probe("0,5");
    double parsed = 0.0;
    probe >> parsed;
    ASSERT_EQ(parsed, 0.5) << "the locale does not use a decimal comma";

    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_alter_only.xml");
    EXPECT_EQ(writtenPitches(score), kAlterOnlyPitches);
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// The whole <alter> text must be the number, surrounding whitespace aside: "1,5" is not a sharp
// followed by noise, and a number with trailing text is not a number.
TEST(ScoreQuarterToneRead, AlterTextMustBeANumberAndNothingElse) {
    for (const std::string text : {"1,5", "0.5abc", "0.5.5", "sharp"}) {
        const auto [pitch, printed] = loadWithAlterText(text);
        EXPECT_EQ(pitch, "C4") << "<alter>" << text << "</alter>";
        EXPECT_NE(printed.find("[WARN] Unrepresentable <alter> value '" + text + "'"),
                  std::string::npos)
            << printed;
    }

    for (const std::string text : {" 0.5 ", "\n-1.5\n", "+0.5"}) {
        const auto [pitch, printed] = loadWithAlterText(text);
        EXPECT_EQ(pitch, text.find("-1.5") != std::string::npos ? "C3b4" : "C1x4")
            << "<alter>" << text << "</alter>";
        EXPECT_EQ(printed.find("[WARN]"), std::string::npos) << printed;
    }
}

// Near a quarter-tone sharp is not a quarter-tone sharp: the reader never rounds to the nearest
// representable pitch, it reads the note as natural and says so, naming the value.
TEST(ScoreQuarterToneRead, AlterNearAQuarterToneIsNotSnappedOntoIt) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/unrepresentable_alter_near_quarter_tone.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C4");
    EXPECT_NE(capture.str().find("[WARN] Unrepresentable <alter> value '0.46'"), std::string::npos)
        << capture.str();
}

// A recognised <accidental> wins over a disagreeing <alter>, and the disagreement is reported.
TEST(ScoreQuarterToneRead, DisagreeingAccidentalWinsWithAWarning) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_accidental_alter_disagree.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
    EXPECT_NE(capture.str().find("[WARN] The <accidental> 'quarter-sharp' and the <alter> '1' of "
                                 "this note disagree"),
              std::string::npos)
        << capture.str();
}

TEST(ScoreQuarterToneRead, AgreeingAccidentalAndAlterReadWithoutAWarning) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/quarter_tone_tartini.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "C1x4");
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// "sharp-sharp" is MusicXML's double sharp drawn as two sharp signs: a recognised name.
TEST(ScoreQuarterToneRead, SharpSharpAccidentalIsADoubleSharp) {
    StdoutCapture capture;
    Score score("./test/xml_examples/unit_test/accidental_sharp_sharp.xml");
    EXPECT_EQ(score.getPart(0).getMeasure(0).getNote(0, 0).getPitch(), "Cx4");
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// A quarter-tone score written by maialib itself reads back unchanged, with no warning: the
// <alter> and the <accidental> it writes for each quarter tone agree. The source spells every
// quarter tone with an arrow glyph and no <alter>, so each <alter> in the written file is
// maialib's own.
TEST(ScoreQuarterToneRoundTrip, AQuarterToneScoreWrittenByMaialibReadsBackUnchanged) {
    Score original("./test/xml_examples/unit_test/test_quarter_tones.musicxml");
    const std::vector<std::string> pitches = writtenPitches(original);
    for (const char* quarterTone : {"C1x4", "C3x4", "C1b4", "C3b4"}) {
        ASSERT_NE(std::find(pitches.begin(), pitches.end(), quarterTone), pitches.end())
            << quarterTone;
    }

    const std::filesystem::path base =
        std::filesystem::temp_directory_path() / "maialib_quarter_tone_round_trip";
    original.toFile(base.string(), false);
    const std::string written = base.string() + ".xml";

    std::vector<std::string> readBack;
    std::string printed;
    {
        StdoutCapture capture;
        Score reread(written);
        readBack = writtenPitches(reread);
        printed = capture.str();
    }
    std::filesystem::remove(written);

    EXPECT_EQ(readBack, pitches);
    EXPECT_EQ(printed.find("[WARN]"), std::string::npos) << printed;
}

// ====================
// Melody Pattern Search
// ====================

namespace {
// The written pitches of a table's matches, one list per row.
std::vector<std::vector<std::string>> writtenPitchesOf(const Score::MelodyPatternTable& table) {
    std::vector<std::vector<std::string>> pitches;
    for (const Score::MelodyPatternRow& row : table) {
        pitches.push_back(row.writtenPitches);
    }
    return pitches;
}

// The (measure, staff, voice) of a table's matches, one per row.
std::vector<std::tuple<int, int, int>> placesOf(const Score::MelodyPatternTable& table) {
    std::vector<std::tuple<int, int, int>> places;
    for (const Score::MelodyPatternRow& row : table) {
        places.emplace_back(row.measure, row.staff, row.voice);
    }
    return places;
}

// Notes of the given pitches, each a quarter note.
std::vector<Note> quarters(const std::vector<std::string>& pitches) {
    std::vector<Note> notes;
    for (const std::string& pitch : pitches) {
        notes.emplace_back(pitch);
    }
    return notes;
}
}  // namespace

// A melodic interval needs two notes: a shorter pattern is rejected, naming the method and the
// length, also when it is empty.
TEST(ScoreMelodyPatternSearch, APatternOfFewerThanTwoNotesIsRejected) {
    Score score("./test/xml_examples/unit_test/melody_last_window.musicxml");

    EXPECT_EQ(thrownFirstLine([&] { score.findMelodyPattern(quarters({"C4"})); }),
              "[maiacore] Score::findMelodyPattern: a melody pattern needs at least 2 notes, and "
              "this one has 1");
    EXPECT_EQ(thrownFirstLine([&] { score.findMelodyPattern(std::vector<Note>{}); }),
              "[maiacore] Score::findMelodyPattern: a melody pattern needs at least 2 notes, and "
              "this one has 0");
}

// The list overload searches each pattern on a worker thread. A pattern whose search raises fails
// the whole call with that error, exactly as the single-pattern overload does, instead of leaving
// an empty table behind.
TEST(ScoreMelodyPatternSearch, AFailingPatternFailsTheListOverloadToo) {
    Score score("./test/xml_examples/unit_test/melody_last_window.musicxml");
    const std::vector<std::vector<Note>> patterns = {quarters({"C4", "D4"}), quarters({"C4"})};

    const std::string single = thrownFirstLine([&] { score.findMelodyPattern(patterns[1]); });
    ASSERT_FALSE(single.empty());

    const std::string list = thrownFirstLine([&] { score.findMelodyPattern(patterns); });
    EXPECT_EQ(list, single);
}

// The last window of a line is searched: E4 G4 is the end of C4 D4 E4 G4, and C4 D4 E4 G4 is the
// whole line.
TEST(ScoreMelodyPatternSearch, TheLastWindowIsSearched) {
    Score score("./test/xml_examples/unit_test/melody_last_window.musicxml");

    EXPECT_EQ(writtenPitchesOf(score.findMelodyPattern(quarters({"E4", "G4"}), 1.0f, 1.0f)),
              (std::vector<std::vector<std::string>>{{"E4", "G4"}}));
    EXPECT_EQ(
        writtenPitchesOf(score.findMelodyPattern(quarters({"C4", "D4", "E4", "G4"}), 1.0f, 1.0f)),
        (std::vector<std::vector<std::string>>{{"C4", "D4", "E4", "G4"}}));
}

// Every voice of every staff is a line, the lower staff's voice 5 included, and no window spans
// two lines. The matches are sorted by measure; the two of measure 0 keep the order of their
// lines (staff 0 before staff 1).
TEST(ScoreMelodyPatternSearch, EveryVoiceOfEveryStaffIsSearched) {
    Score score("./test/xml_examples/unit_test/melody_staves_and_voices.musicxml");
    const auto table = score.findMelodyPattern(quarters({"C4", "D4", "E4", "F4"}), 1.0f, 1.0f);

    EXPECT_EQ(placesOf(table),
              (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {0, 1, 5}, {1, 1, 5}}));
    EXPECT_EQ(writtenPitchesOf(table),
              (std::vector<std::vector<std::string>>{
                  {"G5", "A5", "B5", "C6"}, {"C3", "D3", "E3", "F3"}, {"G3", "A3", "B3", "C4"}}));
}

// Rows are sorted by measure; rows of one measure keep the order of their lines. E4-F4 rises a
// semitone: in staff 0 voice 1 from E5 (measure 0) and B5 (measure 1), in staff 1 voice 5 from E3
// (measure 0) and B3 (measure 1).
TEST(ScoreMelodyPatternSearch, RowsAreSortedByMeasureThenLine) {
    Score score("./test/xml_examples/unit_test/melody_staves_and_voices.musicxml");
    const auto table = score.findMelodyPattern(quarters({"E4", "F4"}), 1.0f, 1.0f);

    EXPECT_EQ(placesOf(table),
              (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {0, 1, 5}, {1, 0, 1}, {1, 1, 5}}));
}

// A chord is one event, represented by its highest sounding note whatever order its notes are
// written in; a grace note is no event.
TEST(ScoreMelodyPatternSearch, AChordIsSearchedByItsHighestNote) {
    Score score("./test/xml_examples/unit_test/melody_chord_top_note.musicxml");
    const auto table = score.findMelodyPattern(quarters({"G4", "D5", "B4"}), 1.0f, 1.0f);

    EXPECT_EQ(writtenPitchesOf(table), (std::vector<std::vector<std::string>>{{"G4", "D5", "B4"}}));
}

// A tied note is one event with the summed duration, in the measure where it starts: the window
// D4 E4 F4 has the rhythm quarter, dotted half, quarter.
TEST(ScoreMelodyPatternSearch, ATiedNoteIsOneEvent) {
    Score score("./test/xml_examples/unit_test/melody_tie_across_barline.musicxml");
    std::vector<Note> pattern = quarters({"D4", "E4", "F4"});
    pattern[1].setDuration(3.0f);

    const auto table = score.findMelodyPattern(pattern, 1.0f, 1.0f);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(table[0].measure, 0);
    EXPECT_EQ(table[0].writtenPitches, (std::vector<std::string>{"D4", "E4", "F4"}));
    EXPECT_EQ(table[0].rhythmDiff, (std::vector<float>{0.0f, 0.0f, 0.0f}));
}

// A transposition without a name -- C4 to Cx5, an augmented ninth, or to the quarter tone C1x4
// -- leaves transposeInterval empty and the search goes on; transposeSemitones holds the exact
// interval.
TEST(ScoreMelodyPatternSearch, ATranspositionWithoutANameLeavesItEmpty) {
    Score score("./test/xml_examples/unit_test/melody_unnameable_transposition.musicxml");
    const auto table = score.findMelodyPattern(quarters({"C4", "D4", "E4"}), 1.0f, 1.0f);

    ASSERT_EQ(table.size(), 2u);
    EXPECT_EQ(table[0].writtenPitches, (std::vector<std::string>{"Cx5", "Dx5", "Ex5"}));
    EXPECT_EQ(table[0].transposeInterval, "");
    EXPECT_FLOAT_EQ(table[0].transposeSemitones, 14.0f);
    EXPECT_EQ(table[1].writtenPitches, (std::vector<std::string>{"C1x4", "D1x4", "E1x4"}));
    EXPECT_EQ(table[1].transposeInterval, "");
    EXPECT_FLOAT_EQ(table[1].transposeSemitones, 0.5f);
}

// A pattern that starts on a quarter tone is searched like any other: its interval contour is
// compared exactly, and no transposition from it has a name.
TEST(ScoreMelodyPatternSearch, APatternStartingOnAQuarterToneIsSearched) {
    Score score("./test/xml_examples/unit_test/melody_unnameable_transposition.musicxml");
    const auto table = score.findMelodyPattern(quarters({"C1x4", "D1x4"}), 1.0f, 1.0f);

    EXPECT_EQ(writtenPitchesOf(table),
              (std::vector<std::vector<std::string>>{
                  {"Cx5", "Dx5"}, {"Dx5", "Ex5"}, {"C1x4", "D1x4"}, {"D1x4", "E1x4"}}));
    for (const Score::MelodyPatternRow& row : table) {
        EXPECT_EQ(row.transposeInterval, "");
    }
    EXPECT_FLOAT_EQ(table[2].transposeSemitones, 0.0f);
}

// A pattern or a window without a sounding note has no transposition: NaN, and no name.
TEST(ScoreMelodyPatternSearch, AWindowOfRestsHasNoTransposition) {
    Score score({"Flute"}, 1);
    score.getPart(0).getMeasure(0).addNote(quarters({"rest", "rest"}));

    const auto table = score.findMelodyPattern(quarters({"rest", "rest"}), 1.0f, 1.0f);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(table[0].writtenPitches, (std::vector<std::string>{"rest", "rest"}));
    EXPECT_EQ(table[0].transposeInterval, "");
    EXPECT_TRUE(std::isnan(table[0].transposeSemitones));
}

// Each search reads the score as it is: a note added after a search is found by the next one.
TEST(ScoreMelodyPatternSearch, ASearchSeesTheEditsMadeBeforeIt) {
    Score score({"Flute"}, 2);
    score.getPart(0).getMeasure(0).addNote(quarters({"C4", "D4"}));
    ASSERT_EQ(score.findAnyMelodyPattern(2, 1.0f, 1.0f, 1).size(), 1u);
    ASSERT_TRUE(score.findMelodyPattern(quarters({"D4", "C#5"}), 1.0f, 1.0f).empty());

    score.getPart(0).getMeasure(1).addNote(Note("C#5"));

    EXPECT_EQ(score.findAnyMelodyPattern(2, 1.0f, 1.0f, 1).size(), 2u);
    EXPECT_EQ(writtenPitchesOf(score.findMelodyPattern(quarters({"D4", "C#5"}), 1.0f, 1.0f)),
              (std::vector<std::vector<std::string>>{{"D4", "C#5"}}));
}

namespace {
// Captures everything written to 'stream' for its lifetime and restores the stream's own buffer
// in the destructor. Every write goes through overflow() or xsputn(), which a mutex serialises, so
// text written from several threads at once is captured without a data race on the buffer.
class ConcurrentStreamCapture : public std::streambuf {
   public:
    explicit ConcurrentStreamCapture(std::ostream& stream)
        : _stream(stream), _previous(stream.rdbuf()) {
        _stream.rdbuf(this);
    }
    ~ConcurrentStreamCapture() override { _stream.rdbuf(_previous); }
    ConcurrentStreamCapture(const ConcurrentStreamCapture&) = delete;
    ConcurrentStreamCapture& operator=(const ConcurrentStreamCapture&) = delete;

    std::string str() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _text;
    }

   protected:
    int_type overflow(int_type character) override {
        if (!traits_type::eq_int_type(character, traits_type::eof())) {
            std::lock_guard<std::mutex> lock(_mutex);
            _text.push_back(traits_type::to_char_type(character));
        }
        return traits_type::not_eof(character);
    }

    std::streamsize xsputn(const char* text, std::streamsize count) override {
        std::lock_guard<std::mutex> lock(_mutex);
        _text.append(text, static_cast<size_t>(count));
        return count;
    }

   private:
    std::ostream& _stream;
    std::streambuf* _previous;
    mutable std::mutex _mutex;
    std::string _text;
};
}  // namespace

// The list overload's worker threads write nothing to std::cout or std::cerr. Several workers
// writing at once would share the one stream buffer -- a single Python sys.stdout buffer when
// std::cout is redirected into Python -- and garble what they wrote, or race on it.
TEST(ScoreMelodyPatternSearch, TheListOverloadWritesNothingToTheConsole) {
    Score score("./test/xml_examples/Bach/cello_suite_1_violin.xml");
    const std::vector<std::vector<Note>> patterns(
        8, std::vector<Note>{Note("G2"), Note("D3"), Note("B3")});

    std::vector<Score::MelodyPatternTable> tables;
    std::string printed;
    std::string printedToStderr;
    {
        ConcurrentStreamCapture out(std::cout);
        ConcurrentStreamCapture err(std::cerr);
        tables = score.findMelodyPattern(patterns);
        printed = out.str();
        printedToStderr = err.str();
    }

    EXPECT_EQ(printed, "");
    EXPECT_EQ(printedToStderr, "");
    ASSERT_EQ(tables.size(), patterns.size());
    EXPECT_FALSE(tables[0].empty());  // the search itself ran
}

// More patterns than hardware threads: every pattern is searched, and each finds exactly what it
// finds when searched on its own.
TEST(ScoreMelodyPatternSearch, EveryPatternIsSearchedWhateverTheThreadCount) {
    Score score("./test/xml_examples/Bach/cello_suite_1_violin.xml");
    const size_t numThreads = std::max(1u, std::thread::hardware_concurrency());

    // Three-note windows of the score's own opening melody, so every pattern matches somewhere.
    std::vector<std::string> opening;
    Part& part = score.getPart(0);
    for (int m = 0; m < part.getNumMeasures() && opening.size() < numThreads + 5; m++) {
        const Measure& measure = part.getMeasure(m);
        for (int n = 0; n < measure.getNumNotes(0); n++) {
            const Note& note = measure.getNote(n, 0);
            if (note.isNoteOn() && note.getVoice() == 1 && !note.inChord()) {
                opening.push_back(note.getWrittenPitch());
            }
        }
    }
    std::vector<std::vector<Note>> patterns;
    for (size_t i = 0; i < numThreads + 3; i++) {
        patterns.push_back({Note(opening[i]), Note(opening[i + 1]), Note(opening[i + 2])});
    }

    const auto tables = score.findMelodyPattern(patterns, 0.5f, 0.5f);
    ASSERT_EQ(tables.size(), patterns.size());
    for (size_t i = 0; i < patterns.size(); i++) {
        const auto single = score.findMelodyPattern(patterns[i], 0.5f, 0.5f);
        EXPECT_FALSE(single.empty()) << "pattern " << i;
        EXPECT_EQ(tables[i], single) << "pattern " << i << " of " << patterns.size();
    }
}

// C4-D4 (+2) and C4-D1b4 (+1.5) are two patterns a quarter tone apart, not duplicates. The line
// C4 D4 C4 D1b4 E4 has four distinct two-note windows: C4-D4, D4-C4, C4-D1b4 and, the last,
// D1b4-E4 (+2.5).
TEST(ScoreMelodyPatternSearch, PatternsAQuarterToneApartAreNotMergedAsDuplicates) {
    Score score("./test/xml_examples/unit_test/melody_patterns_quarter_tone_apart.xml");
    const auto found = score.findAnyMelodyPattern(2, 1.0f, 1.0f, 1);
    EXPECT_EQ(found.size(), 4u);
}

// Two windows are one pattern when their intervals and their durations are equal; the first
// window is kept. C4-D4 in quarter notes occurs twice; E4-F#4 has its interval in half notes and
// is another pattern. Each pattern is searched with the given thresholds, so C4-D4 also matches
// E4-F#4, whose rhythm has the same proportions. With minOccurrences 1 every pattern is kept.
TEST(ScoreMelodyPatternSearch, FindAnyMelodyPatternKeepsTheFirstOfEqualWindows) {
    Score score("./test/xml_examples/unit_test/melody_duplicate_patterns.musicxml");
    const auto found = score.findAnyMelodyPattern(2, 1.0f, 1.0f, 1);

    std::vector<std::vector<std::string>> patterns;
    std::vector<float> firstDurations;
    for (const Score::FoundMelodyPattern& entry : found) {
        std::vector<std::string> pitches;
        for (const Note& note : entry.pattern) {
            pitches.push_back(note.getWrittenPitch());
        }
        patterns.push_back(pitches);
        firstDurations.push_back(entry.pattern[0].getQuarterDuration());
    }
    EXPECT_EQ(patterns, (std::vector<std::vector<std::string>>{
                            {"C4", "D4"}, {"D4", "C4"}, {"D4", "E4"}, {"E4", "F#4"}}));
    EXPECT_EQ(firstDurations, (std::vector<float>{1.0f, 1.0f, 1.0f, 2.0f}));
    ASSERT_EQ(found.size(), 4u);
    EXPECT_EQ(writtenPitchesOf(found[0].matches),
              (std::vector<std::vector<std::string>>{{"C4", "D4"}, {"C4", "D4"}, {"E4", "F#4"}}));
}

// A pattern needs at least 2 notes: a smaller length is rejected by name, never a crash.
TEST(ScoreMelodyPatternSearch, FindAnyMelodyPatternRejectsFewerThanTwoNotes) {
    Score score("./test/xml_examples/unit_test/melody_duplicate_patterns.musicxml");
    for (const int length : {1, 0, -1}) {
        EXPECT_EQ(thrownFirstLine([&] { score.findAnyMelodyPattern(length); }),
                  "[maiacore] Score::findAnyMelodyPattern: patternNumNotes must be at least 2, "
                  "and it is " +
                      std::to_string(length))
            << length;
    }
}

// A pattern is kept when it has at least minOccurrences matches, its own window included. At the
// default thresholds of 1 and the default of 2, C4-D4 (matched by C4-D4 twice and E4-F#4) and
// E4-F#4 (the same three) are kept; D4-C4 and D4-E4, which occur once, are not.
TEST(ScoreMelodyPatternSearch, FindAnyMelodyPatternKeepsPatternsThatOccurAtLeastMinOccurrences) {
    Score score("./test/xml_examples/unit_test/melody_duplicate_patterns.musicxml");
    const auto firstPitches = [](const std::vector<Score::FoundMelodyPattern>& found) {
        std::vector<std::string> pitches;
        for (const Score::FoundMelodyPattern& entry : found) {
            pitches.push_back(entry.pattern[0].getWrittenPitch() + "-" +
                              entry.pattern[1].getWrittenPitch());
        }
        return pitches;
    };

    EXPECT_EQ(firstPitches(score.findAnyMelodyPattern(2)),
              (std::vector<std::string>{"C4-D4", "E4-F#4"}));
    EXPECT_EQ(firstPitches(score.findAnyMelodyPattern(2, 1.0f, 1.0f, 3)),
              (std::vector<std::string>{"C4-D4", "E4-F#4"}));
    EXPECT_TRUE(score.findAnyMelodyPattern(2, 1.0f, 1.0f, 4).empty());
    for (const int minOccurrences : {0, -1}) {
        EXPECT_EQ(
            thrownFirstLine([&] { score.findAnyMelodyPattern(2, 1.0f, 1.0f, minOccurrences); }),
            "[maiacore] Score::findAnyMelodyPattern: minOccurrences must be at least 1, "
            "and it is " +
                std::to_string(minOccurrences))
            << minOccurrences;
    }
}

// A pattern longer than every line finds nothing, however many notes the score has: here voice 1
// has one event and voice 2 two, in a score of three notes.
TEST(ScoreMelodyPatternSearch, APatternLongerThanEveryLineFindsNoMatch) {
    Score score({"Flute"}, 1);
    Measure& measure = score.getPart(0).getMeasure(0);
    measure.addNote(Note("C4"));
    for (const char* pitch : {"D4", "E4"}) {
        Note secondVoice(pitch);
        secondVoice.setVoice(2);
        measure.addNote(secondVoice);
    }
    const std::vector<Note> pattern = quarters({"C4", "D4", "E4"});
    ASSERT_EQ(score.getNumNotes(), 3);

    EXPECT_TRUE(score.findMelodyPattern(pattern).empty());
    const auto tables = score.findMelodyPattern(std::vector<std::vector<Note>>{pattern});
    ASSERT_EQ(tables.size(), 1u);
    EXPECT_TRUE(tables[0].empty());
    EXPECT_TRUE(score.findAnyMelodyPattern(3).empty());
}

// A part with more measures than the first part: the concert key, computed for the first part's
// measures, is empty beyond them, and the search goes on.
TEST(ScoreMelodyPatternSearch, AMeasureBeyondTheFirstPartHasNoConcertKey) {
    Score score({"Flute", "Oboe"}, 1);
    score.getPart(1).addMeasure(1);
    score.getPart(1).getMeasure(1).addNote(quarters({"C4", "D4"}));

    const auto table = score.findMelodyPattern(quarters({"C4", "D4"}), 1.0f, 1.0f);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(std::make_tuple(table[0].partName, table[0].measure),
              std::make_tuple(score.getPartName(1), 1));
    EXPECT_EQ(table[0].writtenKey, "C");
    EXPECT_EQ(table[0].concertKey, "");
}

// ===== Parts of transposing instruments ===== //

// A horn in F's written B4, C#5 and D#5 sound E4, F#4 and G#4, the pattern itself: the window is
// the pattern at a unison, named "P1" without a trailing space.
TEST(ScoreMelodyPatternSearch, ATransposingPartIsComparedByThePitchesItSounds) {
    Score score({"Horn in F"}, 1);
    Measure& measure = score.getPart(0).getMeasure(0);
    for (const char* written : {"B4", "C#5", "D#5", "B4"}) {
        measure.addNote(hornInF(written));
    }

    const auto table = score.findMelodyPattern(quarters({"E4", "F#4", "G#4"}), 1.0f, 1.0f);
    ASSERT_EQ(table.size(), 1u);
    EXPECT_EQ(table[0].transposeInterval, "P1");
    EXPECT_FLOAT_EQ(table[0].transposeSemitones, 0.0f);
    EXPECT_EQ(table[0].writtenPitches, (std::vector<std::string>{"B4", "C#5", "D#5"}));
    EXPECT_EQ(table[0].soundingPitches, (std::vector<std::string>{"E4", "F#4", "G#4"}));
    EXPECT_FLOAT_EQ(table[0].totalSimilarity, 1.0f);
}

// A clarinet in B-flat's part: its written key and pitches, the pitches it sounds and the
// score's concert key, which the violin's C major gives.
TEST(ScoreMelodyPatternSearch, AMatchReportsTheWrittenAndTheConcertKey) {
    Score score("./test/xml_examples/unit_test/melody_transposing_instrument.musicxml");
    const auto table = score.findMelodyPattern(quarters({"C4", "D4", "E4"}), 1.0f, 1.0f);

    ASSERT_EQ(table.size(), 1u);
    const Score::MelodyPatternRow& row = table[0];
    EXPECT_EQ(row.partName, "Clarinet in Bb");
    EXPECT_EQ(std::make_tuple(row.measure, row.staff, row.voice), std::make_tuple(0, 0, 1));
    EXPECT_EQ(row.writtenKey, "D");
    EXPECT_EQ(row.concertKey, "C");
    EXPECT_EQ(row.transposeInterval, "P1");
    EXPECT_EQ(row.writtenPitches, (std::vector<std::string>{"D4", "E4", "F#4"}));
    EXPECT_EQ(row.soundingPitches, (std::vector<std::string>{"C4", "D4", "E4"}));
    EXPECT_EQ(row.semitonesDiff, (std::vector<float>{0.0f, 0.0f}));
    EXPECT_FLOAT_EQ(row.intervalSimilarity, 1.0f);
    EXPECT_FLOAT_EQ(row.rhythmSimilarity, 1.0f);
}

// A score's chords are named by the pitches its parts sound: a horn in F's written B4 sounds E4,
// which with a violin's C4 and a viola's G4 is a C major chord, and which a second violin's E4
// doubles, so the duplicate is removed.
TEST(ScoreGetChords, ATransposingPartIsAnalysedByThePitchesItSounds) {
    Score score({"Violin", "Horn in F", "Viola", "Violin II"}, 1);
    score.getPart(0).getMeasure(0).addNote(Note("C4", RhythmFigure::WHOLE));
    score.getPart(1).getMeasure(0).addNote(hornInF("B4", RhythmFigure::WHOLE));
    score.getPart(2).getMeasure(0).addNote(Note("G4", RhythmFigure::WHOLE));
    score.getPart(3).getMeasure(0).addNote(Note("E4", RhythmFigure::WHOLE));

    const auto chords = score.getChords();
    ASSERT_EQ(chords.size(), 1u);
    Chord chord = std::get<3>(chords[0]);
    EXPECT_EQ(chord.size(), 3);
    EXPECT_EQ(chord.getName(), "C");
}

// ====================
// MusicXML <transpose>
// ====================

namespace {
const std::string kUnitTest = "./test/xml_examples/unit_test/";
const std::string kW3c = "./test/musicxml/w3c-test-suite/xmlFiles/";
const std::string kSamples = "./maialib/xml-scores-examples/";

std::string doublingName(const OctaveDoubling doubling) {
    switch (doubling) {
        case OctaveDoubling::BELOW:
            return "BELOW";
        case OctaveDoubling::ABOVE:
            return "ABOVE";
        default:
            return "NONE";
    }
}

// A note as "<written> (<diatonic>, <chromatic>, <doubling>) <sounding>".
std::string describeTransposition(const Note& note) {
    return note.getWrittenPitch() + " (" + std::to_string(note.getTransposeDiatonic()) + ", " +
           std::to_string(note.getTransposeChromatic()) + ", " +
           doublingName(note.getOctaveDoubling()) + ") " + note.getSoundingPitch();
}

// Every pitched note of a part, measure by measure and staff by staff, as describeTransposition()
// writes it.
std::vector<std::string> transposedNotes(Score& score, const int partId) {
    std::vector<std::string> notes;
    Part& part = score.getPart(partId);
    for (int m = 0; m < part.getNumMeasures(); m++) {
        const Measure& measure = part.getMeasure(m);
        for (int s = 0; s < measure.getNumStaves(); s++) {
            for (int n = 0; n < measure.getNumNotes(s); n++) {
                const Note& note = measure.getNote(n, s);
                if (note.isNoteOn() && note.isPitched()) {
                    notes.push_back(describeTransposition(note));
                }
            }
        }
    }
    return notes;
}

// How many warnings of the <transpose> reader 'printed' holds.
int transposeWarnings(const std::string& printed) {
    int count = 0;
    for (const char* code : {"[WARN] [transpose-", "[WARN] [for-part-"}) {
        for (size_t at = printed.find(code); at != std::string::npos;
             at = printed.find(code, at + 1)) {
            count++;
        }
    }
    return count;
}
}  // namespace

// A <transpose> applies from where it stands to the next one: a change in the middle of a part is
// followed and carried forward, and <diatonic>0</diatonic><chromatic>0</chromatic> returns to
// untransposed.
TEST(ScoreTransposeRead, aChangeInTheMiddleOfAPartIsFollowed) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_change_mid_part.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-2, -3, NONE) A3",
                                        "D4 (-2, -3, NONE) B3", "C4 (0, 0, NONE) C4"}));
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// A part whose first <transpose> comes in a later measure is untransposed up to it; rests and
// unpitched notes take no transposition.
TEST(ScoreTransposeRead, aTransposeInALaterMeasureAppliesFromThere) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_later_measure.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (0, 0, NONE) C4", "C4 (-4, -7, NONE) F3",
                                        "G4 (-4, -7, NONE) C4"}));
    const Note& rest = score.getPart(0).getMeasure(1).getNote(0, 0);
    ASSERT_TRUE(rest.isNoteOff());
    EXPECT_FALSE(rest.isTransposed());
    const Note& unpitched = score.getPart(0).getMeasure(2).getNote(1, 0);
    ASSERT_FALSE(unpitched.isPitched());
    EXPECT_FALSE(unpitched.isTransposed());
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// A <transpose number="s"> applies to staff s alone; one without number to every staff.
TEST(ScoreTransposeRead, aNumberedTransposeAppliesToItsStaffAlone) {
    Score score(kUnitTest + "transpose_per_staff.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C5 (-1, -2, NONE) Bb4", "C5 (-2, -3, NONE) A4",
                                        "D5 (-1, -2, NONE) C5", "D5 (-4, -7, NONE) G4",
                                        "E5 (-1, -2, NONE) D5", "E5 (-1, -2, NONE) D5"}));
}

// <octave-change> is folded into the interval: 7 letters and 12 semitones per octave.
TEST(ScoreTransposeRead, anOctaveChangeIsFoldedIntoTheInterval) {
    Score score(kUnitTest + "transpose_octave_change.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"G5 (7, 12, NONE) G6"}));
    EXPECT_EQ(transposedNotes(score, 1), (std::vector<std::string>{"D4 (-8, -14, NONE) C3"}));
    EXPECT_EQ(transposedNotes(score, 2), (std::vector<std::string>{"C3 (-7, -12, NONE) C2"}));
}

// <double/> doubles one octave below, <double above="yes"/> one octave above; the note itself
// sounds as written.
TEST(ScoreTransposeRead, doubleIsReadAsTheOctaveDoubling) {
    Score score(kUnitTest + "transpose_double.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"C3 (0, 0, BELOW) C3"}));
    EXPECT_EQ(transposedNotes(score, 1), (std::vector<std::string>{"G4 (0, 0, ABOVE) G4"}));
}

// Inside a measure, document order decides: a <transpose> after notes applies to the notes after
// it, and one after the measure's last note from the next measure on.
TEST(ScoreTransposeRead, aTransposeAfterNotesAppliesToTheNotesAfterIt) {
    Score score(kUnitTest + "transpose_after_notes.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C5 (0, 0, NONE) C5", "D5 (0, 0, NONE) D5",
                                        "E5 (-3, -5, NONE) B4", "F5 (-3, -5, NONE) C5",
                                        "G5 (-3, -5, NONE) D5", "C5 (0, 0, NONE) C5"}));
}

// A chord is read as a unit: a <transpose> written between the notes of a chord applies from the
// first note after the chord, and every note of the chord keeps the transposition in force at its
// first note.
TEST(ScoreTransposeRead, aTransposeBetweenTheNotesOfAChordAppliesAfterTheChord) {
    Score score(kUnitTest + "transpose_inside_chord.musicxml");
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C5 (-1, -2, NONE) Bb4", "E5 (-1, -2, NONE) D5",
                                        "G5 (-1, -2, NONE) F5", "D5 (-2, -3, NONE) B4"}));
}

// Without <diatonic>, the conventional diatonic interval of <chromatic> is stored, silently.
TEST(ScoreTransposeRead, aTransposeWithoutDiatonicStoresTheConventionalInterval) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_without_diatonic.musicxml");
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"F#4 (-1, -2, NONE) E4"}));
    EXPECT_EQ(capture.str().find("[WARN]"), std::string::npos) << capture.str();
}

// A <chromatic> that is not a whole number is ignored with one warning; the previous
// transposition stays in force.
TEST(ScoreTransposeRead, aChromaticThatIsNotAWholeNumberIsIgnored) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_chromatic_not_integer.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-1, -2, NONE) Bb3"}));
    EXPECT_NE(printed.find("[WARN] [transpose-chromatic-not-integer] part \"Clarinet\", measure 2: "
                           "<chromatic>-2.5</chromatic> is not a whole number of semitones; the "
                           "<transpose> is ignored and the previous transposition stays in "
                           "force.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// Likewise an <octave-change> that is not a whole number.
TEST(ScoreTransposeRead, anOctaveChangeThatIsNotAWholeNumberIsIgnored) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_octave_change_not_integer.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3", "C4 (-1, -2, NONE) Bb3"}));
    EXPECT_NE(printed.find("[WARN] [transpose-octave-change-not-integer] part \"Clarinet\", "
                           "measure 2: <octave-change>1.5</octave-change> is not a whole number "
                           "of octaves; the <transpose> is ignored and the previous "
                           "transposition stays in force.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// The W3C transposing-instrument files sound as they say: 72c's change from E-flat to B-flat
// clarinet in measure 2 is followed, and 72d's part transposed by an augmented fourth and three
// octaves sounds MIDI note 72 from a written F#1.
TEST(ScoreTransposeRead, theW3cTransposingInstrumentsSoundAsTheFilesSay) {
    Score a(kW3c + "72a-TransposingInstruments.musicxml");
    EXPECT_EQ(describeTransposition(a.getPart(0).getMeasure(0).getNote(0, 0)),
              "D4 (-1, -2, NONE) C4");
    EXPECT_EQ(describeTransposition(a.getPart(1).getMeasure(0).getNote(0, 0)),
              "A4 (-5, -9, NONE) C4");

    Score c(kW3c + "72c-TransposingInstruments-Change.musicxml");
    EXPECT_EQ(transposedNotes(c, 0),
              (std::vector<std::string>{"C4 (2, 3, NONE) Eb4", "C4 (-1, -2, NONE) Bb3",
                                        "C4 (-1, -2, NONE) Bb3"}));

    Score d(kW3c + "72d-TransposingInstruments-scorePitch.musicxml");
    const Note& displayed = d.getPart(9).getMeasure(0).getNote(0, 0);
    EXPECT_EQ(describeTransposition(displayed), "F#1 (24, 42, NONE) C5");
    EXPECT_EQ(displayed.getMidiNumber(), 72);
}

// W3C 41c's piccolo, bass clarinet, contrabassoon and contrabass sound an octave from what they
// read.
TEST(ScoreTransposeRead, theOctaveTransposingPartsOf41cSoundAnOctaveAway) {
    Score score(kW3c + "41c-StaffGroups.musicxml");
    EXPECT_EQ(describeTransposition(score.getPart(0).getMeasure(0).getNote(0, 0)),
              "B4 (7, 12, NONE) B5");
    EXPECT_EQ(describeTransposition(score.getPart(6).getMeasure(0).getNote(0, 0)),
              "B4 (-8, -14, NONE) A3");
    EXPECT_EQ(describeTransposition(score.getPart(8).getMeasure(0).getNote(0, 0)),
              "B2 (-7, -12, NONE) B1");
    EXPECT_EQ(describeTransposition(score.getPart(24).getMeasure(0).getNote(0, 0)),
              "C3 (-7, -12, NONE) C2");
}

// The samples' contrabasses sound an octave below what they read; their other transposing parts
// read as before.
TEST(ScoreTransposeRead, theSamplesContrabassesSoundAnOctaveLower) {
    Score beethoven(kSamples + "Beethoven_Symphony_5_mov_1.xml");
    EXPECT_EQ(describeTransposition(beethoven.getPart(11).getMeasure(0).getNote(1, 0)),
              "G3 (-7, -12, NONE) G2");
    EXPECT_EQ(describeTransposition(beethoven.getPart(2).getMeasure(0).getNote(1, 0)),
              "A4 (-1, -2, NONE) G4");
    EXPECT_EQ(describeTransposition(beethoven.getPart(4).getMeasure(17).getNote(1, 0)),
              "E5 (-5, -9, NONE) G4");

    Score dvorak(kSamples + "Dvorak_Symphony_9_mov_4.mxl");
    EXPECT_EQ(describeTransposition(dvorak.getPart(15).getMeasure(0).getNote(0, 0)),
              "B2 (-7, -12, NONE) B1");
    EXPECT_EQ(describeTransposition(dvorak.getPart(2).getMeasure(7).getNote(1, 0)),
              "Db5 (-2, -3, NONE) Bb4");
}

// Reading <octave-change> changes the chords: test_getchords_poly's contrabass sounds C2 under the
// first chord, and its G2 makes measure 3's first chord a G7 in root position, where its G3 had
// made a G7/F.
TEST(ScoreTransposeRead, anOctaveTranspositionChangesTheChords) {
    Score score(kUnitTest + "test_getchords_poly.musicxml");
    const auto chords = score.getChords();
    ASSERT_EQ(chords.size(), 9u);
    Chord first = std::get<3>(chords[0]);
    EXPECT_EQ(first.getBassNote().getPitch(), "C2");
    Chord dominant = std::get<3>(chords[5]);
    EXPECT_EQ(dominant.getName(), "G7");
}

// A <diatonic> that does not match <chromatic> is replaced by the conventional diatonic interval,
// with one warning per <transpose>; an explicit 0 with a non-zero chromatic interval does not
// match either, while a tritone matches the augmented fourth and the diminished fifth alike.
TEST(ScoreTransposeCorrection, aDiatonicIntervalThatDoesNotMatchIsReplaced) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_pair_inconsistent.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"F#4 (2, 4, NONE) A#4"}));
    EXPECT_EQ(transposedNotes(score, 1), (std::vector<std::string>{"C4 (-1, -2, NONE) Bb3"}));
    EXPECT_EQ(transposedNotes(score, 2), (std::vector<std::string>{"C4 (-4, -6, NONE) F#3"}));
    EXPECT_EQ(transposedNotes(score, 3), (std::vector<std::string>{"C4 (-3, -6, NONE) Gb3"}));
    EXPECT_NE(printed.find("[WARN] [transpose-pair-corrected] part \"Trumpet in E\", measure 1: "
                           "<diatonic>3</diatonic> does not match <chromatic>4</chromatic>; "
                           "using 2.\n"),
              std::string::npos)
        << printed;
    EXPECT_NE(printed.find("[WARN] [transpose-pair-corrected] part \"Clarinet in Bb\", measure 1: "
                           "<diatonic>0</diatonic> does not match <chromatic>-2</chromatic>; "
                           "using -1.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 2) << printed;
}

// The Dvorak sample's trumpets in E, written with the letters of a fourth and the semitones of a
// major third, are read as a major third: a written F#4 sounds A#4.
TEST(ScoreTransposeCorrection, theDvorakTrumpetsInESoundAMajorThirdUp) {
    StdoutCapture capture;
    Score score(kSamples + "Dvorak_Symphony_9_mov_4.mxl");
    const std::string printed = capture.str();
    EXPECT_EQ(describeTransposition(score.getPart(6).getMeasure(7).getNote(1, 0)),
              "F#4 (2, 4, NONE) A#4");
    EXPECT_NE(printed.find("[WARN] [transpose-pair-corrected] part \"Trombe I. II. E\", measure "
                           "1: <diatonic>3</diatonic> does not match <chromatic>4</chromatic>; "
                           "using 2.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// A <diatonic> that is not a whole number does not match <chromatic> either.
TEST(ScoreTransposeCorrection, aDiatonicThatIsNotAWholeNumberIsReplaced) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_diatonic_not_integer.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"G4 (-4, -7, NONE) C4"}));
    EXPECT_NE(printed.find("[WARN] [transpose-pair-corrected] part \"Horn in F\", measure 1: "
                           "<diatonic>-4.5</diatonic> does not match <chromatic>-7</chromatic>; "
                           "using -4.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// A <transpose> with which a note of its scope would have no sounding pitch is ignored for its
// whole scope -- none of its notes takes it, not even one that could sound with it -- and the
// previous transposition stays in force; a chord with a note that this one cannot sound either is
// read untransposed as a unit, and the warning counts its notes.
TEST(ScoreTransposeCorrection, aTransposeOutOfRangeIsIgnoredForItsWholeScope) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_out_of_range.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0),
              (std::vector<std::string>{"C9 (7, 12, NONE) C10", "C8 (7, 12, NONE) C9",
                                        "C9 (7, 12, NONE) C10"}));
    EXPECT_EQ(transposedNotes(score, 1),
              (std::vector<std::string>{"C1 (-7, -12, NONE) C0", "Cb0 (0, 0, NONE) B-1",
                                        "C1 (0, 0, NONE) C1", "D1 (-7, -12, NONE) D0"}));
    EXPECT_NE(printed.find("[WARN] [transpose-out-of-range] part \"Piccolo\", measure 2: the "
                           "written C9 of measure 3, staff 1 would sound outside the "
                           "representable range; the <transpose> is ignored and the previous "
                           "transposition stays in force.\n"),
              std::string::npos)
        << printed;
    EXPECT_NE(printed.find("[WARN] [transpose-out-of-range] part \"Contrabass\", measure 2: the "
                           "written Cb0 of measure 2, staff 1 would sound outside the "
                           "representable range; the <transpose> is ignored and the previous "
                           "transposition stays in force. Its notes that the previous "
                           "transposition cannot sound either are read untransposed, with the "
                           "other notes of their chords (2 in all).\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 2) << printed;
}

// <for-part> is dropped with a warning: a concert score's notes are already at concert pitch.
TEST(ScoreTransposeCorrection, aForPartIsDroppedWithAWarning) {
    StdoutCapture capture;
    Score score(kUnitTest + "transpose_for_part.musicxml");
    const std::string printed = capture.str();
    EXPECT_EQ(transposedNotes(score, 0), (std::vector<std::string>{"C4 (0, 0, NONE) C4"}));
    EXPECT_NE(printed.find("[WARN] [for-part-not-modelled] part \"Clarinet in Bb\", measure 1: "
                           "<for-part> is not modelled and is dropped; the notes of a concert "
                           "score are written at concert pitch.\n"),
              std::string::npos)
        << printed;
    EXPECT_EQ(transposeWarnings(printed), 1) << printed;
}

// ====================
// The concert key of getChords()
// ====================

namespace {
// The (fifths, major) of the key of every chord getChords() finds.
std::set<std::pair<int, bool>> chordKeys(Score& score,
                                         const nlohmann::json& config = nlohmann::json()) {
    std::set<std::pair<int, bool>> keys;
    for (const auto& chord : score.getChords(config)) {
        keys.insert({std::get<2>(chord).getFifthCircle(), std::get<2>(chord).isMajorMode() != 0});
    }
    return keys;
}

// A whole note written 'pitch' on an instrument transposing by (diatonic, chromatic).
Note wholeNote(const std::string& pitch, const int diatonic = 0, const int chromatic = 0) {
    return transposingNote(pitch, diatonic, chromatic, RhythmFigure::WHOLE);
}

// One part of a one-measure score: its name, its written key and the whole note it plays.
struct KeyedPart {
    std::string name;
    int fifths;
    bool major;
    Note note;
};

Score keyedScore(const std::vector<KeyedPart>& parts) {
    std::vector<std::string> names;
    for (const KeyedPart& part : parts) {
        names.push_back(part.name);
    }
    Score score(names, 1);
    for (size_t p = 0; p < parts.size(); p++) {
        Measure& measure = score.getPart(static_cast<int>(p)).getMeasure(0);
        measure.setKey(parts[p].fifths, parts[p].major);
        measure.addNote(parts[p].note);
    }
    return score;
}
}  // namespace

// The W3C transposing-instrument files report the concert key: 72a's piano gives C major against
// the trumpet's written D and the horn's written A; 72c, whose only part transposes, gives B-flat
// major in both its measures, as an E-flat clarinet in G and then a B-flat clarinet in C; 72d's
// untransposed parts give G major.
TEST(ScoreConcertKey, theW3cTransposingInstrumentsReportTheConcertKey) {
    Score a(kW3c + "72a-TransposingInstruments.musicxml");
    EXPECT_EQ(chordKeys(a), (std::set<std::pair<int, bool>>{{0, true}}));
    Score c(kW3c + "72c-TransposingInstruments-Change.musicxml");
    EXPECT_EQ(chordKeys(c), (std::set<std::pair<int, bool>>{{-2, true}}));
    Score d(kW3c + "72d-TransposingInstruments-scorePitch.musicxml");
    EXPECT_EQ(chordKeys(d), (std::set<std::pair<int, bool>>{{1, true}}));
}

// test_pattern's horn in F is written in G major; its trombone gives the concert key, C major.
TEST(ScoreConcertKey, aHornWrittenInGDoesNotGiveTheKey) {
    Score score(kUnitTest + "test_pattern.musicxml");
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{0, true}}));
}

// The Beethoven 5 sample's C trumpet and timpani are untransposed and written without a key
// signature; the eight other untransposed parts, contrabasses included, write C minor's three
// flats, which win.
TEST(ScoreConcertKey, theBeethovenSampleIsInCMinor) {
    Score score(kSamples + "Beethoven_Symphony_5_mov_1.xml");
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-3, false}}));
}

TEST(ScoreConcertKey, theMostFrequentKeyWins) {
    Score score = keyedScore({{"Timpani", 0, true, wholeNote("C3")},
                              {"Violin", -3, false, wholeNote("C4")},
                              {"Viola", -3, false, wholeNote("G3")}});
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-3, false}}));
}

// A part transposed by whole octaves only is key-neutral: the piccolo counts with the flute, and
// their G major outweighs the timpani's C major.
TEST(ScoreConcertKey, anOctaveTranspositionIsKeyNeutral) {
    Score score = keyedScore({{"Piccolo", 1, true, wholeNote("D5", 7, 12)},
                              {"Timpani", 0, true, wholeNote("G2")},
                              {"Flute", 1, true, wholeNote("D5")}});
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{1, true}}));
}

TEST(ScoreConcertKey, aTieGoesToTheFirstPart) {
    Score score =
        keyedScore({{"Flute", -1, true, wholeNote("C5")}, {"Oboe", 2, true, wholeNote("D5")}});
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-1, true}}));
}

TEST(ScoreConcertKey, percussionDoesNotCount) {
    Score score = keyedScore({{"Violin", -1, true, wholeNote("C4")},
                              {"Snare Drum", 0, true, wholeNote("C4")},
                              {"Bass Drum", 0, true, wholeNote("C4")}});
    score.getPart(1).setIsPitched(false);
    score.getPart(2).setIsPitched(false);
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-1, true}}));
}

// A part's interval in a measure without pitched notes is that of its last pitched note before
// the measure, or else of its first one after: the horn rests in measures 1 and 3 and still
// transposes there, so its written G major does not count, and the violin gives the key.
TEST(ScoreConcertKey, aMeasureOfRestsTakesTheIntervalOfTheNeighbouringNotes) {
    Score score({"Horn in F", "Violin"}, 3);
    for (int m = 0; m < 3; m++) {
        for (int p = 0; p < 2; p++) {
            Measure& measure = score.getPart(p).getMeasure(m);
            measure.setNumber(m);
            measure.setKey(p == 0 ? 1 : 0, true);
        }
        score.getPart(1).getMeasure(m).addNote(wholeNote("C4"));
    }
    score.getPart(0).getMeasure(0).addNote(Note("rest", RhythmFigure::WHOLE));
    score.getPart(0).getMeasure(1).addNote(wholeNote("D5", -4, -7));
    score.getPart(0).getMeasure(2).addNote(Note("rest", RhythmFigure::WHOLE));
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{0, true}}));
}

// A pitched part without any pitched note counts as untransposed, with its written key.
TEST(ScoreConcertKey, aPartWithoutNotesCountsAsUntransposed) {
    Score score({"Violin", "Flute 2", "Flute 3"}, 1);
    score.getPart(0).getMeasure(0).setKey(0, true);
    score.getPart(0).getMeasure(0).addNote(wholeNote("C4"));
    score.getPart(1).getMeasure(0).setKey(-1, true);
    score.getPart(2).getMeasure(0).setKey(-1, true);
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{-1, true}}));
}

// When every pitched part transposes, the first part's written key is moved by its interval and
// brought by twelves into -6..11 fifths: a trumpet in D written with 11 fifths sounds 13, held as
// 1; a B-flat clarinet written with -6 fifths, minor, sounds -8, held as 4, minor.
TEST(ScoreConcertKey, whenEveryPartTransposesTheFirstPartsKeyIsMovedAndWrapped) {
    Score trumpet = keyedScore({{"Trumpet in D", 11, true, wholeNote("C4", 1, 2)}});
    EXPECT_EQ(chordKeys(trumpet), (std::set<std::pair<int, bool>>{{1, true}}));
    Score clarinet = keyedScore({{"Clarinet in Bb", -6, false, wholeNote("C4", -1, -2)}});
    EXPECT_EQ(chordKeys(clarinet), (std::set<std::pair<int, bool>>{{4, false}}));
}

// A counted part's transposition is read in each measure: the clarinet transposes in measure 1, so
// the violin's C major is the key there; in measure 2 it is untransposed and its A major ties with
// the violin's C major, and the tie goes to the clarinet, the first part.
TEST(ScoreConcertKey, aCountedPartsTranspositionIsReadInEachMeasure) {
    Score score({"Clar", "Vln"}, 2);
    for (int m = 0; m < 2; m++) {
        for (int p = 0; p < 2; p++) {
            score.getPart(p).getMeasure(m).setNumber(m);
        }
        score.getPart(1).getMeasure(m).setKey(0, true);
        score.getPart(1).getMeasure(m).addNote(wholeNote("C4"));
    }
    score.getPart(0).getMeasure(0).setKey(2, true);
    score.getPart(0).getMeasure(0).addNote(wholeNote("D4", -1, -2));
    score.getPart(0).getMeasure(1).setKey(3, true);
    score.getPart(0).getMeasure(1).addNote(wholeNote("D4"));
    std::vector<std::pair<int, bool>> keys;
    for (const auto& chord : score.getChords()) {
        keys.push_back(
            {std::get<2>(chord).getFifthCircle(), std::get<2>(chord).isMajorMode() != 0});
    }
    EXPECT_EQ(keys, (std::vector<std::pair<int, bool>>{{0, true}, {3, true}}));
}

// When every pitched part transposes, the first pitched part gives the key, not an unpitched part
// before it: a B-flat clarinet written with 3 fifths sounds 1.
TEST(ScoreConcertKey, whenEveryPitchedPartTransposesTheFirstPitchedPartGivesTheKey) {
    Score score = keyedScore({{"Snare Drum", 0, true, wholeNote("C4")},
                              {"Clarinet in Bb", 3, true, wholeNote("D4", -1, -2)}});
    score.getPart(0).setIsPitched(false);
    EXPECT_EQ(chordKeys(score), (std::set<std::pair<int, bool>>{{1, true}}));
}

// Every part counts, also those partNames leaves out: the violin, in F major, gives the key of the
// clarinet's chords.
TEST(ScoreConcertKey, partsLeftOutOfTheAnalysisStillCount) {
    Score score = keyedScore({{"Clarinet in Bb", 2, true, wholeNote("D4", -1, -2)},
                              {"Violin", -1, true, wholeNote("F4")}});
    nlohmann::json config;
    config["partNames"] = std::vector<std::string>{"Clarinet in Bb"};
    EXPECT_EQ(chordKeys(score, config), (std::set<std::pair<int, bool>>{{-1, true}}));
}

// ====================
// The octave doubling in getChords()
// ====================

namespace {
// The pitches a chord sounds, lowest first.
std::vector<std::string> soundingPitches(const Chord& chord) {
    std::vector<std::string> pitches;
    for (const Note& note : chord.getNotes()) {
        pitches.push_back(note.getSoundingPitch());
    }
    return pitches;
}

Note doubledNote(Note note, const OctaveDoubling doubling) {
    note.setOctaveDoubling(doubling);
    return note;
}
}  // namespace

TEST(ScoreOctaveDoubling, aDoubledNoteAddsItsOctaveToTheChord) {
    Score score({"Violoncello and Contrabass", "Flute and Piccolo"}, 1);
    score.getPart(0).getMeasure(0).addNote(doubledNote(wholeNote("C3"), OctaveDoubling::BELOW));
    score.getPart(1).getMeasure(0).addNote(doubledNote(wholeNote("G4"), OctaveDoubling::ABOVE));
    const auto chords = score.getChords();
    ASSERT_EQ(chords.size(), 1u);
    EXPECT_EQ(soundingPitches(std::get<3>(chords[0])),
              (std::vector<std::string>{"C2", "C3", "G4", "G5"}));
}

// The doubled octave is an octave from what the note sounds: a bass clarinet's written D4 sounds
// C3, and its doubling below C2.
TEST(ScoreOctaveDoubling, theDoubledOctaveIsAnOctaveFromWhatTheNoteSounds) {
    Score score({"Bass Clarinet"}, 1);
    score.getPart(0).getMeasure(0).addNote(
        doubledNote(wholeNote("D4", -8, -14), OctaveDoubling::BELOW));
    const auto chords = score.getChords();
    ASSERT_EQ(chords.size(), 1u);
    EXPECT_EQ(soundingPitches(std::get<3>(chords[0])), (std::vector<std::string>{"C2", "C3"}));
}

// A doubled octave outside the representable range is left out of every chord the note sounds in,
// with one warning for the note.
TEST(ScoreOctaveDoubling, aDoubledOctaveOutOfRangeIsLeftOutWithOneWarning) {
    Score score({"Contrabass", "Violin"}, 1);
    score.getPart(0).getMeasure(0).addNote(doubledNote(wholeNote("C-1"), OctaveDoubling::BELOW));
    for (int i = 0; i < 4; i++) {
        score.getPart(1).getMeasure(0).addNote(Note("G4"));
    }
    StdoutCapture capture;
    const auto chords = score.getChords();
    const std::string printed = capture.str();
    ASSERT_EQ(chords.size(), 4u);
    for (const auto& chord : chords) {
        EXPECT_EQ(soundingPitches(std::get<3>(chord)), (std::vector<std::string>{"C-1", "G4"}));
    }
    const std::string warning =
        "[WARN] Score::getChords: the octave doubling of the written C-1, which sounds C-1, lies "
        "outside the representable range and is left out of the chords.\n";
    const size_t first = printed.find(warning);
    EXPECT_NE(first, std::string::npos) << printed;
    EXPECT_EQ(printed.find(warning, first + 1), std::string::npos) << printed;
}

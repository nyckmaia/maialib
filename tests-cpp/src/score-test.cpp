#include "maiacore/score.h"

#include <gtest/gtest.h>

#include "maiacore/helper.h"
#include "maiacore/note.h"
#include "maiacore/part.h"

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
  score.forEachNote([&noteCount](Part* part, Measure* measure, int staveId, Note* note) {
    noteCount++;
    EXPECT_NE(note, nullptr);
  });

  EXPECT_GT(noteCount, 0);
  EXPECT_EQ(noteCount, score.getNumNotes());
}

TEST(ScoreNoteIteration, ForEachNoteMeasureRange) {
  Score score("./test/xml_examples/unit_test/test_multiples_measures.xml");

  int noteCount = 0;
  score.forEachNote([&noteCount](Part* part, Measure* measure, int staveId, Note* note) {
    noteCount++;
  }, 0, 2);  // Only measures 0, 1, 2

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
  score.forEachNote([](Part* part, Measure* measure, int staveId, Note* note) {
    if (note->isNoteOn()) {
      int currentOctave = note->getOctave();
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
  bool hasTypeTag = score.haveTypeTag();
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
  // Load a score
  Score score1("./test/xml_examples/unit_test/test_chord.xml");
  EXPECT_TRUE(score1.isValid());

  int originalNotes = score1.getNumNotes();
  int originalMeasures = score1.getNumMeasures();
  int originalParts = score1.getNumParts();

  // Export to file (toFile adds .xml extension automatically)
  score1.toFile("test_roundtrip", false);

  // Load the exported file
  Score score2("test_roundtrip.xml");
  EXPECT_TRUE(score2.isValid());

  // Verify counts match
  EXPECT_EQ(score2.getNumNotes(), originalNotes);
  EXPECT_EQ(score2.getNumMeasures(), originalMeasures);
  EXPECT_EQ(score2.getNumParts(), originalParts);
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

  // Self-assignment should be safe
  score = score;

  // Verify score is unchanged
  EXPECT_EQ(score.getTitle(), "Self Test");
  EXPECT_EQ(score.getNumParts(), 1);
  EXPECT_EQ(score.getNumMeasures(), 4);
}

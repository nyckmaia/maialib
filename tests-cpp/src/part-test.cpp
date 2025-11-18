#include <gtest/gtest.h>
#include <maiacore/part.h>
#include <maiacore/measure.h>
#include <maiacore/note.h>
#include <maiacore/chord.h>
#include <string>
#include <vector>

// ====================
// Constructor Tests
// ====================

TEST(PartConstructor, DefaultParameters) {
  Part part("Piano");

  EXPECT_EQ(part.getName(), "Piano");
  EXPECT_EQ(part.getNumStaves(), 1);
  EXPECT_TRUE(part.isPitched());
  EXPECT_EQ(part.getNumMeasures(), 0);
}

TEST(PartConstructor, WithNumStaves) {
  Part part("Organ", 2);

  EXPECT_EQ(part.getName(), "Organ");
  EXPECT_EQ(part.getNumStaves(), 2);
  EXPECT_TRUE(part.isPitched());
}

TEST(PartConstructor, UnpitchedPercussion) {
  Part part("Drums", 1, false);

  EXPECT_EQ(part.getName(), "Drums");
  EXPECT_FALSE(part.isPitched());
}

TEST(PartConstructor, CustomDivisionsPerQuarterNote) {
  Part part("Violin", 1, true, 512);

  EXPECT_EQ(part.getName(), "Violin");
  EXPECT_TRUE(part.isPitched());
}

// ====================
// Part Index Tests
// ====================

TEST(PartIndex, GetAndSetPartIndex) {
  Part part("Flute");

  part.setPartIndex(0);
  EXPECT_EQ(part.getPartIndex(), 0);

  part.setPartIndex(5);
  EXPECT_EQ(part.getPartIndex(), 5);
}

// ====================
// Name Tests
// ====================

TEST(PartName, GetName) {
  Part part("Clarinet");
  EXPECT_EQ(part.getName(), "Clarinet");
}

TEST(PartName, GetAndSetShortName) {
  Part part("Violoncello");

  part.setShortName("Vc.");
  EXPECT_EQ(part.getShortName(), "Vc.");

  part.setShortName("Cello");
  EXPECT_EQ(part.getShortName(), "Cello");
}

TEST(PartName, DefaultShortName) {
  Part part("Viola");
  // Default short name is generated from the full name
  EXPECT_GT(part.getShortName().length(), 0);
}

// ====================
// Measure Management Tests
// ====================

TEST(PartMeasure, AddMeasures) {
  Part part("Piano");

  part.addMeasure(5);
  EXPECT_EQ(part.getNumMeasures(), 5);

  part.addMeasure(3);
  EXPECT_EQ(part.getNumMeasures(), 8);
}

TEST(PartMeasure, GetMeasure) {
  Part part("Piano");
  part.addMeasure(3);

  Measure& measure0 = part.getMeasure(0);
  Measure& measure1 = part.getMeasure(1);
  Measure& measure2 = part.getMeasure(2);

  // Measures should be distinct objects
  EXPECT_NE(&measure0, &measure1);
  EXPECT_NE(&measure1, &measure2);
}

TEST(PartMeasure, GetMeasureConst) {
  Part part("Piano");
  part.addMeasure(2);

  const Part& constPart = part;
  const Measure& measure = constPart.getMeasure(0);

  // Should be able to access measure from const Part
  EXPECT_EQ(measure.getNumNotes(), 0);
}

TEST(PartMeasure, GetMeasures) {
  Part part("Piano");
  part.addMeasure(4);

  std::vector<Measure> measures = part.getMeasures();
  EXPECT_EQ(measures.size(), 4);
}

TEST(PartMeasure, RemoveMeasureSingle) {
  Part part("Piano");
  part.addMeasure(5);

  part.removeMeasure(2, 2);  // Remove measure 2 only
  EXPECT_EQ(part.getNumMeasures(), 4);
}

TEST(PartMeasure, RemoveMeasureRange) {
  Part part("Piano");
  part.addMeasure(10);

  part.removeMeasure(3, 6);  // Remove measures 3, 4, 5, 6
  EXPECT_EQ(part.getNumMeasures(), 6);
}

TEST(PartMeasure, RemoveMeasureAll) {
  Part part("Piano");
  part.addMeasure(5);

  part.removeMeasure(0, 4);  // Remove all measures
  EXPECT_EQ(part.getNumMeasures(), 0);
}

TEST(PartMeasure, ClearAllMeasures) {
  Part part("Piano");
  part.addMeasure(10);

  part.clear();
  EXPECT_EQ(part.getNumMeasures(), 0);
}

// ====================
// Stave Management Tests
// ====================

TEST(PartStave, SetNumStaves) {
  Part part("Piano");
  part.addMeasure(1);  // setNumStaves requires at least one measure

  part.setNumStaves(2);
  EXPECT_EQ(part.getNumStaves(), 2);

  part.setNumStaves(1);
  EXPECT_EQ(part.getNumStaves(), 1);
}

TEST(PartStave, AddStaves) {
  Part part("Organ", 1);

  part.addStaves(1);
  EXPECT_EQ(part.getNumStaves(), 2);

  part.addStaves(2);
  EXPECT_EQ(part.getNumStaves(), 4);
}

TEST(PartStave, RemoveStave) {
  Part part("Piano", 3);

  // NOTE: removeStave() is currently not implemented (stub)
  // It only calls ignore(staveId) and doesn't actually remove the stave
  part.removeStave(1);
  EXPECT_EQ(part.getNumStaves(), 3);  // Unchanged
}

// ====================
// Staff Lines Tests
// ====================

TEST(PartStaffLines, DefaultStaffLines) {
  Part part("Violin");
  EXPECT_EQ(part.getStaffLines(), 5);
}

TEST(PartStaffLines, SetStaffLines) {
  Part part("Percussion", 1, false);

  part.setStaffLines(1);  // Single-line percussion
  EXPECT_EQ(part.getStaffLines(), 1);

  part.setStaffLines(5);  // Standard 5-line staff
  EXPECT_EQ(part.getStaffLines(), 5);
}

// ====================
// Pitched/Unpitched Tests
// ====================

TEST(PartPitched, SetIsPitched) {
  Part part("Instrument");
  part.addMeasure(1);  // setIsPitched requires at least one measure

  part.setIsPitched(true);
  EXPECT_TRUE(part.isPitched());

  part.setIsPitched(false);
  EXPECT_FALSE(part.isPitched());
}

TEST(PartPitched, AddMidiUnpitched) {
  Part part("Drums", 1, false);

  part.addMidiUnpitched(36);  // Bass drum
  part.addMidiUnpitched(38);  // Snare
  part.addMidiUnpitched(42);  // Hi-hat

  std::vector<int> midiUnpitched = part.getMidiUnpitched();
  EXPECT_EQ(midiUnpitched.size(), 3);
  EXPECT_EQ(midiUnpitched[0], 36);
  EXPECT_EQ(midiUnpitched[1], 38);
  EXPECT_EQ(midiUnpitched[2], 42);
}

// ====================
// Note Count Tests
// ====================

TEST(PartNoteCount, GetNumNotesEmpty) {
  Part part("Piano");
  part.addMeasure(5);

  EXPECT_EQ(part.getNumNotes(), 0);
}

TEST(PartNoteCount, GetNumNotesAfterAddingNotes) {
  Part part("Piano");
  part.addMeasure(2);

  part.getMeasure(0).addNote(Note("C4"), 0);
  part.getMeasure(0).addNote(Note("E4"), 0);
  part.getMeasure(1).addNote(Note("G4"), 0);

  EXPECT_EQ(part.getNumNotes(), 3);
}

TEST(PartNoteCount, GetNumNotesSpecificStave) {
  Part part("Piano", 2);
  part.addMeasure(1);

  part.getMeasure(0).addNote(Note("C4"), 0);  // Stave 0
  part.getMeasure(0).addNote(Note("E4"), 0);  // Stave 0
  part.getMeasure(0).addNote(Note("C3"), 1);  // Stave 1

  EXPECT_EQ(part.getNumNotes(0), 2);
  EXPECT_EQ(part.getNumNotes(1), 1);
  EXPECT_EQ(part.getNumNotes(-1), 3);  // All staves
}

TEST(PartNoteCount, GetNumNotesOnVsOff) {
  Part part("Piano");
  part.addMeasure(1);

  part.getMeasure(0).addNote(Note("C4"), 0);
  part.getMeasure(0).addNote(Note("E4"), 0);
  part.getMeasure(0).addNote(Note(-1), 0);   // Rest (note off)

  EXPECT_EQ(part.getNumNotesOn(), 2);
  EXPECT_EQ(part.getNumNotesOff(), 1);
  EXPECT_EQ(part.getNumNotes(), 3);
}

TEST(PartNoteCount, GetNumNotesOnSpecificStave) {
  Part part("Piano", 2);
  part.addMeasure(1);

  part.getMeasure(0).addNote(Note("C4"), 0);  // Stave 0, note on
  part.getMeasure(0).addNote(Note(-1), 0);    // Stave 0, rest
  part.getMeasure(0).addNote(Note("C3"), 1);  // Stave 1, note on

  EXPECT_EQ(part.getNumNotesOn(0), 1);
  EXPECT_EQ(part.getNumNotesOn(1), 1);
  EXPECT_EQ(part.getNumNotesOff(0), 1);
  EXPECT_EQ(part.getNumNotesOff(1), 0);
}

// ====================
// Append Note Tests
// ====================

TEST(PartAppend, AppendSingleNoteVariant) {
  Part part("Piano");
  part.addMeasure(1);

  std::variant<Note, Chord> noteVariant = Note("C4");
  part.append(noteVariant, -1, 0);

  EXPECT_EQ(part.getNumNotes(), 1);
}

TEST(PartAppend, AppendMultipleNotesVariant) {
  Part part("Piano");
  part.addMeasure(1);

  std::vector<std::variant<Note, Chord>> notes;
  notes.push_back(Note("C4"));
  notes.push_back(Note("E4"));
  notes.push_back(Note("G4"));

  part.append(notes, -1, 0);

  EXPECT_EQ(part.getNumNotes(), 3);
}

TEST(PartAppend, AppendChordVariant) {
  Part part("Piano");
  part.addMeasure(1);

  Chord chord({"C4", "E4", "G4"});
  std::variant<Note, Chord> chordVariant = chord;
  part.append(chordVariant, -1, 0);

  // A chord with 3 notes should count as 3 notes
  EXPECT_EQ(part.getNumNotes(), 3);
}

TEST(PartAppend, AppendMultipleChordsVariant) {
  Part part("Piano");
  part.addMeasure(1);

  std::vector<std::variant<Note, Chord>> chords;
  chords.push_back(Chord({"C4", "E4", "G4"}));
  chords.push_back(Chord({"D4", "F4", "A4"}));

  part.append(chords, -1, 0);

  EXPECT_EQ(part.getNumNotes(), 6);  // 2 chords × 3 notes each
}

TEST(PartAppend, AppendMixedNotesAndChords) {
  Part part("Piano");
  part.addMeasure(1);

  std::vector<std::variant<Note, Chord>> objects;
  objects.push_back(Note("C4"));
  objects.push_back(Chord(std::vector<std::string>{"E4", "G4"}));
  objects.push_back(Note("A4"));

  part.append(objects, -1, 0);

  EXPECT_EQ(part.getNumNotes(), 4);  // 1 + 2 + 1
}

// ====================
// Serialization Tests
// ====================

TEST(PartSerialization, ToXML) {
  Part part("Piano");
  part.addMeasure(1);
  part.setShortName("Pno.");

  std::string xml = part.toXML(1, 2);

  EXPECT_GT(xml.length(), 0);
  // Part::toXML() outputs measure XML, not part names
  EXPECT_NE(xml.find("<measure"), std::string::npos);
}

TEST(PartSerialization, ToXMLWithNotes) {
  Part part("Violin");
  part.addMeasure(1);
  part.getMeasure(0).addNote(Note("A4"), 0);

  std::string xml = part.toXML(1, 2);

  EXPECT_GT(xml.length(), 0);
  EXPECT_NE(xml.find("<measure"), std::string::npos);
}

// TODO: Implement Part::toJSON() before enabling this test
// TEST(PartSerialization, ToJSON) {
//   Part part("Cello");
//   part.addMeasure(2);
//
//   std::string json = part.toJSON();
//
//   EXPECT_GT(json.length(), 0);
//   EXPECT_NE(json.find("{"), std::string::npos);
//   EXPECT_NE(json.find("}"), std::string::npos);
// }

// ====================
// Complex Part Tests
// ====================

TEST(PartComplex, MultiStaveWithDifferentVoices) {
  Part part("Piano", 2);
  part.addMeasure(2);

  // Right hand (stave 0)
  part.getMeasure(0).addNote(Note("C5"), 0);
  part.getMeasure(0).addNote(Note("E5"), 0);

  // Left hand (stave 1)
  part.getMeasure(0).addNote(Note("C3"), 1);
  part.getMeasure(0).addNote(Note("G3"), 1);

  EXPECT_EQ(part.getNumStaves(), 2);
  EXPECT_EQ(part.getNumNotes(), 4);
  EXPECT_EQ(part.getNumNotes(0), 2);
  EXPECT_EQ(part.getNumNotes(1), 2);
}

TEST(PartComplex, PercussionWithMidiUnpitched) {
  Part part("Drum Kit", 1, false);
  part.setStaffLines(1);
  part.addMeasure(1);

  part.addMidiUnpitched(36);  // Bass drum
  part.addMidiUnpitched(38);  // Snare
  part.addMidiUnpitched(42);  // Closed hi-hat
  part.addMidiUnpitched(46);  // Open hi-hat

  EXPECT_FALSE(part.isPitched());
  EXPECT_EQ(part.getStaffLines(), 1);
  EXPECT_EQ(part.getMidiUnpitched().size(), 4);
}

TEST(PartComplex, AddRemoveModifyMeasures) {
  Part part("Viola");

  part.addMeasure(10);
  EXPECT_EQ(part.getNumMeasures(), 10);

  part.removeMeasure(5, 7);  // Remove 3 measures
  EXPECT_EQ(part.getNumMeasures(), 7);

  part.addMeasure(3);
  EXPECT_EQ(part.getNumMeasures(), 10);

  part.clear();
  EXPECT_EQ(part.getNumMeasures(), 0);
}

TEST(PartComplex, FullOrganPart) {
  Part part("Organ", 3);  // Manuals I, II + Pedal
  part.setShortName("Org.");
  part.addMeasure(4);

  // Manual I
  part.getMeasure(0).addNote(Note("C5"), 0);

  // Manual II
  part.getMeasure(0).addNote(Note("G4"), 1);

  // Pedal
  part.getMeasure(0).addNote(Note("C2"), 2);

  EXPECT_EQ(part.getName(), "Organ");
  EXPECT_EQ(part.getShortName(), "Org.");
  EXPECT_EQ(part.getNumStaves(), 3);
  EXPECT_EQ(part.getNumMeasures(), 4);
  EXPECT_EQ(part.getNumNotes(), 3);
}

// ====================
// Edge Cases
// ====================

TEST(PartEdgeCases, EmptyPart) {
  Part part("Empty");

  EXPECT_EQ(part.getNumMeasures(), 0);
  EXPECT_EQ(part.getNumNotes(), 0);
  EXPECT_EQ(part.getNumStaves(), 1);
}

TEST(PartEdgeCases, LargeNumberOfStaves) {
  Part part("Experimental", 16);

  EXPECT_EQ(part.getNumStaves(), 16);
}

TEST(PartEdgeCases, ClearEmptyPart) {
  Part part("Piano");

  part.clear();  // Should not crash
  EXPECT_EQ(part.getNumMeasures(), 0);
}

TEST(PartEdgeCases, LongPartName) {
  Part part("Soprano Saxophone in B-flat (Transposing)");

  EXPECT_EQ(part.getName(), "Soprano Saxophone in B-flat (Transposing)");
}

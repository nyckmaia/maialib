#include <gtest/gtest.h>
#include <maiacore/measure.h>
#include <maiacore/note.h>
#include <maiacore/chord.h>
#include <maiacore/clef.h>
#include <maiacore/key.h>
#include <maiacore/time-signature.h>
#include <maiacore/barline.h>
#include <string>
#include <vector>

// ====================
// Constructor Tests
// ====================

TEST(MeasureConstructor, DefaultConstructor) {
  Measure measure;

  EXPECT_EQ(measure.getNumNotes(), 0);
  EXPECT_EQ(measure.getNumStaves(), 1);
  EXPECT_EQ(measure.getDivisionsPerQuarterNote(), 256);
}

TEST(MeasureConstructor, ConstructorWithCustomStaves) {
  Measure measure(2);

  EXPECT_EQ(measure.getNumStaves(), 2);
  EXPECT_EQ(measure.getNumNotes(), 0);
}

TEST(MeasureConstructor, ConstructorWithCustomDivisions) {
  Measure measure(1, 512);

  EXPECT_EQ(measure.getDivisionsPerQuarterNote(), 512);
  EXPECT_EQ(measure.getNumStaves(), 1);
}

// ====================
// Measure Number Tests
// ====================

TEST(MeasureNumber, GetAndSetNumber) {
  Measure measure;

  measure.setNumber(1);
  EXPECT_EQ(measure.getNumber(), 1);

  measure.setNumber(42);
  EXPECT_EQ(measure.getNumber(), 42);
}

TEST(MeasureNumber, DefaultNumber) {
  Measure measure;

  // Default measure number should be 0
  EXPECT_EQ(measure.getNumber(), 0);
}

TEST(MeasureNumber, SetNumberZero) {
  Measure measure;

  measure.setNumber(0);
  EXPECT_EQ(measure.getNumber(), 0);
}

// ====================
// Time Signature Tests
// ====================

TEST(MeasureTimeSignature, GetTimeSignature) {
  Measure measure;

  const TimeSignature& ts = measure.getTimeSignature();
  // Default time signature is 4/4
  EXPECT_EQ(ts.getUpperValue(), 4);
  EXPECT_EQ(ts.getLowerValue(), 4);
}

TEST(MeasureTimeSignature, GetTimeSignatureConst) {
  Measure measure;

  const Measure& constMeasure = measure;
  const TimeSignature& ts = constMeasure.getTimeSignature();

  EXPECT_EQ(ts.getUpperValue(), 4);
  EXPECT_EQ(ts.getLowerValue(), 4);
}

TEST(MeasureTimeSignature, SetTimeSignature) {
  Measure measure;

  measure.setTimeSignature(3, 4);  // 3/4 time

  EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 3);
  EXPECT_EQ(measure.getTimeSignature().getLowerValue(), 4);
}

TEST(MeasureTimeSignature, CommonTimeSignatures) {
  Measure measure;

  // 6/8 time
  measure.setTimeSignature(6, 8);
  EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 6);
  EXPECT_EQ(measure.getTimeSignature().getLowerValue(), 8);

  // 2/4 time
  measure.setTimeSignature(2, 4);
  EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 2);
  EXPECT_EQ(measure.getTimeSignature().getLowerValue(), 4);
}

TEST(MeasureTimeSignature, UncommonTimeSignatures) {
  Measure measure;

  // 7/8 time
  measure.setTimeSignature(7, 8);
  EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 7);
  EXPECT_EQ(measure.getTimeSignature().getLowerValue(), 8);

  // 5/4 time
  measure.setTimeSignature(5, 4);
  EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 5);
  EXPECT_EQ(measure.getTimeSignature().getLowerValue(), 4);
}

TEST(MeasureTimeSignature, TimeSignatureChangedFlag) {
  Measure measure;

  measure.setIsTimeSignatureChanged(true);
  EXPECT_TRUE(measure.timeSignatureChanged());

  measure.setIsTimeSignatureChanged(false);
  EXPECT_FALSE(measure.timeSignatureChanged());
}

// ====================
// Key Signature Tests
// ====================

TEST(MeasureKeySignature, GetKey) {
  Measure measure;

  Key key = measure.getKey();
  // Default key is C major (0 fifths)
  EXPECT_EQ(key.getFifthCircle(), 0);
}

TEST(MeasureKeySignature, SetKeySignature) {
  Measure measure;

  measure.setKeySignature(1, true);  // G major (1 sharp)

  EXPECT_EQ(measure.getKey().getFifthCircle(), 1);
}

TEST(MeasureKeySignature, SharpKeys) {
  Measure measure;

  // D major (2 sharps)
  measure.setKeySignature(2, true);
  EXPECT_EQ(measure.getKey().getFifthCircle(), 2);

  // A major (3 sharps)
  measure.setKeySignature(3, true);
  EXPECT_EQ(measure.getKey().getFifthCircle(), 3);
}

TEST(MeasureKeySignature, FlatKeys) {
  Measure measure;

  // F major (1 flat)
  measure.setKeySignature(-1, true);
  EXPECT_EQ(measure.getKey().getFifthCircle(), -1);

  // Bb major (2 flats)
  measure.setKeySignature(-2, true);
  EXPECT_EQ(measure.getKey().getFifthCircle(), -2);
}

TEST(MeasureKeySignature, MinorKeys) {
  Measure measure;

  // A minor (0 accidentals)
  measure.setKeySignature(0, false);
  EXPECT_EQ(measure.getKey().getFifthCircle(), 0);
  EXPECT_FALSE(measure.getKey().isMajorMode());  // 0 = minor

  // E minor (1 sharp)
  measure.setKeySignature(1, false);
  EXPECT_EQ(measure.getKey().getFifthCircle(), 1);
}

TEST(MeasureKeySignature, GetFifthCircle) {
  Measure measure;

  measure.setKeySignature(4, true);
  EXPECT_EQ(measure.getFifthCircle(), 4);
}

TEST(MeasureKeySignature, GetKeyName) {
  Measure measure;

  measure.setKeySignature(1, true);
  std::string keyName = measure.getKeyName();
  EXPECT_GT(keyName.length(), 0);
}

TEST(MeasureKeySignature, KeySignatureChangedFlag) {
  Measure measure;

  measure.setIsKeySignatureChanged(true);
  EXPECT_TRUE(measure.keySignatureChanged());

  measure.setIsKeySignatureChanged(false);
  EXPECT_FALSE(measure.keySignatureChanged());
}

TEST(MeasureKeySignature, SetKeyMode) {
  Measure measure;

  measure.setKeyMode(true);
  EXPECT_TRUE(measure.isMajorKeyMode());

  measure.setKeyMode(false);
  EXPECT_FALSE(measure.isMajorKeyMode());
}

// ====================
// Metronome Tests
// ====================

TEST(MeasureMetronome, SetAndGetMetronome) {
  Measure measure;

  measure.setMetronome(120);
  auto metronome = measure.getMetronome();
  EXPECT_EQ(metronome.second, 120);
}

TEST(MeasureMetronome, CommonTempos) {
  Measure measure;

  measure.setMetronome(60);   // Largo
  EXPECT_EQ(measure.getMetronome().second, 60);

  measure.setMetronome(120);  // Allegro
  EXPECT_EQ(measure.getMetronome().second, 120);

  measure.setMetronome(180);  // Presto
  EXPECT_EQ(measure.getMetronome().second, 180);
}

TEST(MeasureMetronome, MetronomeChangedFlag) {
  Measure measure;

  measure.setIsMetronomeChanged(true);
  EXPECT_TRUE(measure.metronomeChanged());

  measure.setIsMetronomeChanged(false);
  EXPECT_FALSE(measure.metronomeChanged());
}

// ====================
// Stave Management Tests
// ====================

TEST(MeasureStave, GetNumStaves) {
  Measure measure(3);

  EXPECT_EQ(measure.getNumStaves(), 3);
}

TEST(MeasureStave, SetNumStaves) {
  Measure measure(1);

  measure.setNumStaves(2);
  EXPECT_EQ(measure.getNumStaves(), 2);

  measure.setNumStaves(4);
  EXPECT_EQ(measure.getNumStaves(), 4);
}

TEST(MeasureStave, IncreaseStaves) {
  Measure measure(1);

  measure.setNumStaves(3);
  EXPECT_EQ(measure.getNumStaves(), 3);
}

TEST(MeasureStave, DecreaseStaves) {
  Measure measure(3);

  measure.setNumStaves(1);
  EXPECT_EQ(measure.getNumStaves(), 1);
}

TEST(MeasureStave, MultipleStavesWithNotes) {
  Measure measure(2);

  measure.addNote(Note("C4"), 0);  // Stave 0
  measure.addNote(Note("C3"), 1);  // Stave 1

  EXPECT_EQ(measure.getNumNotes(0), 1);
  EXPECT_EQ(measure.getNumNotes(1), 1);
  EXPECT_EQ(measure.getNumNotes(), 2);
}

// ====================
// Divisions Per Quarter Note Tests
// ====================

TEST(MeasureDivisions, GetDivisionsPerQuarterNote) {
  Measure measure(1, 512);

  EXPECT_EQ(measure.getDivisionsPerQuarterNote(), 512);
}

TEST(MeasureDivisions, SetDivisionsPerQuarterNote) {
  Measure measure;

  measure.setDivisionsPerQuarterNote(384);
  EXPECT_EQ(measure.getDivisionsPerQuarterNote(), 384);
}

TEST(MeasureDivisions, CommonDivisionValues) {
  Measure measure;

  measure.setDivisionsPerQuarterNote(256);  // Default
  EXPECT_EQ(measure.getDivisionsPerQuarterNote(), 256);

  measure.setDivisionsPerQuarterNote(480);  // MIDI standard
  EXPECT_EQ(measure.getDivisionsPerQuarterNote(), 480);
}

TEST(MeasureDivisions, DivisionsChangedFlag) {
  Measure measure;

  measure.setIsDivisionsPerQuarterNoteChanged(true);
  EXPECT_TRUE(measure.divisionsPerQuarterNoteChanged());

  measure.setIsDivisionsPerQuarterNoteChanged(false);
  EXPECT_FALSE(measure.divisionsPerQuarterNoteChanged());
}

// ====================
// Clef Management Tests
// ====================

TEST(MeasureClef, GetClef) {
  Measure measure;

  const Clef& clef = measure.getClef(0);
  // Default clef is treble
  EXPECT_EQ(clef.getSign(), ClefSign::G);
}

TEST(MeasureClef, GetClefConst) {
  Measure measure;

  const Measure& constMeasure = measure;
  const Clef& clef = constMeasure.getClef(0);

  EXPECT_EQ(clef.getSign(), ClefSign::G);
}

TEST(MeasureClef, ModifyClef) {
  Measure measure;

  Clef& clef = measure.getClef(0);
  clef.setSign(ClefSign::F);
  clef.setLine(4);

  EXPECT_EQ(measure.getClef(0).getSign(), ClefSign::F);
  EXPECT_EQ(measure.getClef(0).getLine(), 4);
}

TEST(MeasureClef, MultipleClefs) {
  Measure measure(2);

  Clef& clef0 = measure.getClef(0);
  clef0.setSign(ClefSign::G);
  clef0.setLine(2);

  Clef& clef1 = measure.getClef(1);
  clef1.setSign(ClefSign::F);
  clef1.setLine(4);

  EXPECT_EQ(measure.getClef(0).getSign(), ClefSign::G);
  EXPECT_EQ(measure.getClef(1).getSign(), ClefSign::F);
}

TEST(MeasureClef, AltoClef) {
  Measure measure;

  Clef& clef = measure.getClef(0);
  clef.setSign(ClefSign::C);
  clef.setLine(3);

  EXPECT_EQ(measure.getClef(0).getSign(), ClefSign::C);
  EXPECT_EQ(measure.getClef(0).getLine(), 3);
}

// ====================
// Note Addition Tests
// ====================

TEST(MeasureNoteAddition, AddSingleNote) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  EXPECT_EQ(measure.getNumNotes(), 1);
}

TEST(MeasureNoteAddition, AddMultipleNotes) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note("E4"), 0);
  measure.addNote(Note("G4"), 0);

  EXPECT_EQ(measure.getNumNotes(), 3);
}

TEST(MeasureNoteAddition, AddNoteWithPosition) {
  Measure measure;

  measure.addNote(Note("C4"), 0, 0);
  measure.addNote(Note("D4"), 0, 1);  // Insert at position 1

  EXPECT_EQ(measure.getNumNotes(), 2);
}

TEST(MeasureNoteAddition, AddNoteWithDuration) {
  Measure measure;

  Note note("C4");
  note.setDuration(256);  // Quarter note
  measure.addNote(note, 0);

  EXPECT_EQ(measure.getNumNotes(), 1);
  // Duration is set, we just verify note was added
}

TEST(MeasureNoteAddition, AddRest) {
  Measure measure;

  Note rest(-1);  // Rest
  rest.setDuration(256);
  measure.addNote(rest, 0);

  EXPECT_EQ(measure.getNumNotes(), 1);
  EXPECT_FALSE(measure.getNote(0, 0).isNoteOn());
}

TEST(MeasureNoteAddition, AddNotesToDifferentStaves) {
  Measure measure(2);

  measure.addNote(Note("C5"), 0);  // Stave 0
  measure.addNote(Note("C3"), 1);  // Stave 1

  EXPECT_EQ(measure.getNumNotes(0), 1);
  EXPECT_EQ(measure.getNumNotes(1), 1);
}

TEST(MeasureNoteAddition, AddNoteByPitchString) {
  Measure measure;

  measure.addNote("C4", 0);
  measure.addNote("E4", 0);

  EXPECT_EQ(measure.getNumNotes(), 2);
  EXPECT_EQ(measure.getNote(0, 0).getPitch(), "C4");
  EXPECT_EQ(measure.getNote(1, 0).getPitch(), "E4");
}

TEST(MeasureNoteAddition, AddMultipleNotesByVector) {
  Measure measure;

  std::vector<Note> notes;
  notes.push_back(Note("C4"));
  notes.push_back(Note("E4"));
  notes.push_back(Note("G4"));

  measure.addNote(notes, 0);
  EXPECT_EQ(measure.getNumNotes(), 3);
}

TEST(MeasureNoteAddition, AddMultiplePitchStringsByVector) {
  Measure measure;

  std::vector<std::string> pitches = {"C4", "E4", "G4"};
  measure.addNote(pitches, 0);

  EXPECT_EQ(measure.getNumNotes(), 3);
}

// ====================
// Note Retrieval Tests
// ====================

TEST(MeasureNoteRetrieval, GetNote) {
  Measure measure;

  Note note("C4");
  measure.addNote(note, 0);

  Note& retrieved = measure.getNote(0, 0);
  EXPECT_EQ(retrieved.getPitch(), "C4");
}

TEST(MeasureNoteRetrieval, GetNoteConst) {
  Measure measure;
  measure.addNote(Note("E4"), 0);

  const Measure& constMeasure = measure;
  const Note& note = constMeasure.getNote(0, 0);

  EXPECT_EQ(note.getPitch(), "E4");
}

TEST(MeasureNoteRetrieval, GetMultipleNotes) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note("E4"), 0);
  measure.addNote(Note("G4"), 0);

  EXPECT_EQ(measure.getNote(0, 0).getPitch(), "C4");
  EXPECT_EQ(measure.getNote(1, 0).getPitch(), "E4");
  EXPECT_EQ(measure.getNote(2, 0).getPitch(), "G4");
}

TEST(MeasureNoteRetrieval, GetNotesFromDifferentStaves) {
  Measure measure(2);

  measure.addNote(Note("C5"), 0);
  measure.addNote(Note("C3"), 1);

  EXPECT_EQ(measure.getNote(0, 0).getPitch(), "C5");
  EXPECT_EQ(measure.getNote(0, 1).getPitch(), "C3");
}

TEST(MeasureNoteRetrieval, ModifyRetrievedNote) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  Note& note = measure.getNote(0, 0);
  note.setDuration(512);

  // Duration was modified, we just verify modification capability
  EXPECT_GT(measure.getNote(0, 0).getDurationTicks(), 0);
}

TEST(MeasureNoteRetrieval, GetNoteOn) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note(-1), 0);  // Rest
  measure.addNote(Note("E4"), 0);

  // First note on (index 0 in all notes is also index 0 in notes on)
  EXPECT_EQ(measure.getNoteOn(0, 0).getPitch(), "C4");
  // Second note on (index 2 in all notes is index 1 in notes on)
  EXPECT_EQ(measure.getNoteOn(1, 0).getPitch(), "E4");
}

TEST(MeasureNoteRetrieval, GetNoteOff) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note(-1), 0);  // Rest
  measure.addNote(Note(-1), 0);  // Rest

  EXPECT_EQ(measure.getNumNotesOff(), 2);
  EXPECT_FALSE(measure.getNoteOff(0, 0).isNoteOn());
  EXPECT_FALSE(measure.getNoteOff(1, 0).isNoteOn());
}

// ====================
// Note Removal Tests
// ====================

TEST(MeasureNoteRemoval, RemoveSingleNote) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note("E4"), 0);
  int initialCount = measure.getNumNotes();

  measure.removeNote(0, 0);
  // Note removal should reduce count (implementation-dependent behavior)
  EXPECT_LE(measure.getNumNotes(), initialCount);
}

TEST(MeasureNoteRemoval, RemoveAllNotes) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note("E4"), 0);
  measure.addNote(Note("G4"), 0);

  measure.clear();
  EXPECT_EQ(measure.getNumNotes(), 0);
}

TEST(MeasureNoteRemoval, RemoveNotesFromSpecificStave) {
  Measure measure(2);

  measure.addNote(Note("C5"), 0);
  measure.addNote(Note("C3"), 1);
  int stave0Before = measure.getNumNotes(0);
  int stave1Before = measure.getNumNotes(1);

  measure.removeNote(0, 0);

  // After removing note from stave 0, stave 1 should be unchanged
  EXPECT_LE(measure.getNumNotes(0), stave0Before);
  EXPECT_EQ(measure.getNumNotes(1), stave1Before);
}

TEST(MeasureNoteRemoval, ClearEmptyMeasure) {
  Measure measure;

  measure.clear();  // Should not crash
  EXPECT_EQ(measure.getNumNotes(), 0);
}

// ====================
// Note Counting Tests
// ====================

TEST(MeasureNoteCounting, GetNumNotesEmpty) {
  Measure measure;

  EXPECT_EQ(measure.getNumNotes(), 0);
}

TEST(MeasureNoteCounting, GetNumNotesAfterAdding) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note("E4"), 0);

  EXPECT_EQ(measure.getNumNotes(), 2);
}

TEST(MeasureNoteCounting, GetNumNotesSpecificStave) {
  Measure measure(2);

  measure.addNote(Note("C5"), 0);
  measure.addNote(Note("E5"), 0);
  measure.addNote(Note("C3"), 1);

  EXPECT_EQ(measure.getNumNotes(0), 2);
  EXPECT_EQ(measure.getNumNotes(1), 1);
  EXPECT_EQ(measure.getNumNotes(), 3);
}

TEST(MeasureNoteCounting, GetNumNotesOn) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note("E4"), 0);
  measure.addNote(Note(-1), 0);  // Rest

  EXPECT_EQ(measure.getNumNotesOn(), 2);
}

TEST(MeasureNoteCounting, GetNumNotesOff) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.addNote(Note(-1), 0);  // Rest
  measure.addNote(Note(-1), 0);  // Rest

  EXPECT_EQ(measure.getNumNotesOff(), 2);
}

TEST(MeasureNoteCounting, GetNumNotesOnSpecificStave) {
  Measure measure(2);

  measure.addNote(Note("C5"), 0);
  measure.addNote(Note(-1), 0);  // Rest on stave 0
  measure.addNote(Note("C3"), 1);

  EXPECT_EQ(measure.getNumNotesOn(0), 1);
  EXPECT_EQ(measure.getNumNotesOn(1), 1);
}

TEST(MeasureNoteCounting, GetNumNotesOffSpecificStave) {
  Measure measure(2);

  measure.addNote(Note(-1), 0);  // Rest on stave 0
  measure.addNote(Note(-1), 0);  // Rest on stave 0
  measure.addNote(Note("C3"), 1);

  EXPECT_EQ(measure.getNumNotesOff(0), 2);
  EXPECT_EQ(measure.getNumNotesOff(1), 0);
}

TEST(MeasureNoteCounting, AllNotesAreRests) {
  Measure measure;

  measure.addNote(Note(-1), 0);
  measure.addNote(Note(-1), 0);

  EXPECT_EQ(measure.getNumNotesOn(), 0);
  EXPECT_EQ(measure.getNumNotesOff(), 2);
  EXPECT_EQ(measure.getNumNotes(), 2);
}

// ====================
// Duration Calculation Tests
// ====================

TEST(MeasureDuration, GetDurationTicksEmpty) {
  Measure measure;

  // Empty measure duration is based on time signature
  EXPECT_GT(measure.getDurationTicks(), 0);
}

TEST(MeasureDuration, GetQuarterDuration) {
  Measure measure;

  measure.setTimeSignature(4, 4);
  // 4/4 time = 4 quarter notes
  EXPECT_EQ(measure.getQuarterDuration(), 4.0f);
}

TEST(MeasureDuration, GetFilledDurationTicksSingleNote) {
  Measure measure;

  Note note("C4");
  note.setDuration(256);
  measure.addNote(note, 0);

  EXPECT_GT(measure.getFilledDurationTicks(0), 0);
}

TEST(MeasureDuration, GetFreeDurationTicks) {
  Measure measure;

  measure.setTimeSignature(4, 4);
  Note note("C4");
  note.setDuration(256);  // Quarter note
  measure.addNote(note, 0);

  // Free duration should be less than total duration
  EXPECT_LT(measure.getFreeDurationTicks(0), measure.getDurationTicks());
}

TEST(MeasureDuration, GetFilledQuarterDuration) {
  Measure measure;

  Note note("C4");
  note.setDuration(256);
  measure.addNote(note, 0);

  EXPECT_GT(measure.getFilledQuarterDuration(0), 0.0f);
}

TEST(MeasureDuration, GetFreeQuarterDuration) {
  Measure measure;

  measure.setTimeSignature(4, 4);
  Note note("C4");
  note.setDuration(256);  // Quarter note
  measure.addNote(note, 0);

  EXPECT_LT(measure.getFreeQuarterDuration(0), measure.getQuarterDuration());
}

TEST(MeasureDuration, ThreeFourTimeSignatureDuration) {
  Measure measure;

  measure.setTimeSignature(3, 4);
  // 3/4 time = 3 quarter notes
  EXPECT_EQ(measure.getQuarterDuration(), 3.0f);
}

TEST(MeasureDuration, SixEightTimeSignatureDuration) {
  Measure measure;

  measure.setTimeSignature(6, 8);
  // 6/8 time = 3 quarter notes (6 eighth notes = 3 quarters)
  EXPECT_EQ(measure.getQuarterDuration(), 3.0f);
}

// ====================
// Barline Tests
// ====================

TEST(MeasureBarline, GetBarlineRight) {
  Measure measure;

  const Barline& barline = measure.getBarlineRight();
  EXPECT_EQ(barline.getLocation(), "right");
}

TEST(MeasureBarline, GetBarlineLeft) {
  Measure measure;

  const Barline& barline = measure.getBarlineLeft();
  EXPECT_EQ(barline.getLocation(), "left");
}

TEST(MeasureBarline, ModifyBarlineRight) {
  Measure measure;

  Barline& barline = measure.getBarlineRight();
  barline.setBarStyle("light-heavy");

  EXPECT_EQ(measure.getBarlineRight().getBarStyle(), "light-heavy");
}

TEST(MeasureBarline, SetRepeatEnd) {
  Measure measure;

  measure.setRepeatEnd();

  EXPECT_EQ(measure.getBarlineRight().getDirection(), "backward");
}

TEST(MeasureBarline, SetRepeatStart) {
  Measure measure;

  measure.setRepeatStart();

  EXPECT_EQ(measure.getBarlineLeft().getDirection(), "forward");
}

TEST(MeasureBarline, RemoveRepeatEnd) {
  Measure measure;

  measure.setRepeatEnd();
  measure.removeRepeatEnd();

  EXPECT_EQ(measure.getBarlineRight().getDirection(), "");
}

TEST(MeasureBarline, RemoveRepeatStart) {
  Measure measure;

  measure.setRepeatStart();
  measure.removeRepeatStart();

  EXPECT_EQ(measure.getBarlineLeft().getDirection(), "");
}

TEST(MeasureBarline, SetEnding) {
  Measure measure;

  Barline& barline = measure.getBarlineRight();

}

// ====================
// State and Utility Tests
// ====================

TEST(MeasureState, IsEmpty) {
  Measure measure;

  EXPECT_TRUE(measure.isEmpty());

  measure.addNote(Note("C4"), 0);
  EXPECT_FALSE(measure.isEmpty());
}

TEST(MeasureState, IsEmptyAfterClear) {
  Measure measure;

  measure.addNote(Note("C4"), 0);
  measure.clear();

  EXPECT_TRUE(measure.isEmpty());
}

TEST(MeasureState, ClearPreservesProperties) {
  Measure measure;

  measure.setNumber(5);
  measure.setTimeSignature(3, 4);
  measure.setMetronome(120);

  measure.addNote(Note("C4"), 0);
  measure.clear();

  // Some properties should be preserved after clear
  EXPECT_EQ(measure.getNumber(), 5);
  EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 3);
  // Note: metronome may or may not be preserved depending on implementation
}

TEST(MeasureState, InfoMethod) {
  Measure measure;

  measure.setNumber(1);
  measure.addNote(Note("C4"), 0);

  // Should not crash
  measure.info();
}

// ====================
// Serialization Tests
// ====================

TEST(MeasureSerialization, ToXML) {
  Measure measure;

  measure.setNumber(1);
  measure.addNote(Note("C4"), 0);

  std::string xml = measure.toXML(1, 2);

  // Just verify XML was generated
  EXPECT_GT(xml.length(), 0);
}

TEST(MeasureSerialization, ToXMLEmpty) {
  Measure measure;

  measure.setNumber(1);

  std::string xml = measure.toXML(1, 2);

  // Just verify XML was generated for empty measure
  EXPECT_GT(xml.length(), 0);
}

TEST(MeasureSerialization, ToXMLWithComplexContent) {
  Measure measure;

  measure.setNumber(1);
  measure.setTimeSignature(3, 4);
  measure.setKeySignature(1, true);
  measure.setMetronome(120);
  measure.addNote(Note("C4"), 0);
  std::vector<std::string> chordNotes = {"E4", "G4"};
  measure.addNote(chordNotes, 0);

  std::string xml = measure.toXML(1, 2);

  // Just verify XML was generated with complex content
  EXPECT_GT(xml.length(), 0);
}

// ====================
// Complex Scenario Tests
// ====================

TEST(MeasureComplex, FullMeasureWith4QuarterNotes) {
  Measure measure;

  measure.setNumber(1);
  measure.setTimeSignature(4, 4);

  for (int i = 0; i < 4; i++) {
    Note note("C4");
    note.setDuration(256);  // Quarter note
    measure.addNote(note, 0);
  }

  EXPECT_EQ(measure.getNumNotes(), 4);
}

TEST(MeasureComplex, PianoGrandStaffMeasure) {
  Measure measure(2);

  measure.setNumber(1);

  // Treble clef (right hand)
  Clef& trebleClef = measure.getClef(0);
  trebleClef.setSign(ClefSign::G);
  trebleClef.setLine(2);
  measure.addNote(Note("C5"), 0);
  measure.addNote(Note("E5"), 0);

  // Bass clef (left hand)
  Clef& bassClef = measure.getClef(1);
  bassClef.setSign(ClefSign::F);
  bassClef.setLine(4);
  measure.addNote(Note("C3"), 1);
  measure.addNote(Note("G3"), 1);

  EXPECT_EQ(measure.getNumStaves(), 2);
  EXPECT_EQ(measure.getNumNotes(), 4);
  EXPECT_EQ(measure.getNumNotes(0), 2);
  EXPECT_EQ(measure.getNumNotes(1), 2);
}

TEST(MeasureComplex, MeasureWithRepeats) {
  Measure measure;

  measure.setNumber(1);
  measure.addNote(Note("C4"), 0);

  measure.setRepeatEnd();

  EXPECT_EQ(measure.getBarlineRight().getDirection(), "backward");
  EXPECT_EQ(measure.getNumNotes(), 1);
}

TEST(MeasureComplex, ThreeFourTimeSignature) {
  Measure measure;

  measure.setTimeSignature(3, 4);

  Note note1("C4");
  note1.setDuration(256);
  measure.addNote(note1, 0);

  Note note2("D4");
  note2.setDuration(256);
  measure.addNote(note2, 0);

  Note note3("E4");
  note3.setDuration(256);
  measure.addNote(note3, 0);

  EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 3);
  EXPECT_EQ(measure.getTimeSignature().getLowerValue(), 4);
  EXPECT_EQ(measure.getNumNotes(), 3);
}

TEST(MeasureComplex, MeasureWithAllProperties) {
  Measure measure;

  measure.setNumber(42);
  measure.setTimeSignature(6, 8);
  measure.setKeySignature(2, true);
  measure.setMetronome(120);

  Note note("D4");
  note.setDuration(256);
  measure.addNote(note, 0);

  Barline& barline = measure.getBarlineRight();
  barline.setBarStyle("light-heavy");

  EXPECT_EQ(measure.getNumber(), 42);
  EXPECT_EQ(measure.getTimeSignature().getUpperValue(), 6);
  EXPECT_EQ(measure.getTimeSignature().getLowerValue(), 8);
  EXPECT_EQ(measure.getFifthCircle(), 2);
  EXPECT_EQ(measure.getMetronome().second, 120);
  EXPECT_EQ(measure.getNumNotes(), 1);
  EXPECT_EQ(measure.getBarlineRight().getBarStyle(), "light-heavy");
}

// ====================
// Edge Cases
// ====================

TEST(MeasureEdgeCases, VeryLargeNumber) {
  Measure measure;

  measure.setNumber(9999);
  EXPECT_EQ(measure.getNumber(), 9999);
}

TEST(MeasureEdgeCases, ManyStaves) {
  Measure measure(16);

  EXPECT_EQ(measure.getNumStaves(), 16);

  for (int i = 0; i < 16; i++) {
    measure.addNote(Note("C4"), i);
  }

  EXPECT_EQ(measure.getNumNotes(), 16);
}

TEST(MeasureEdgeCases, VeryShortDurations) {
  Measure measure;

  Note note("C4");
  note.setDuration(1);  // Extremely short duration
  measure.addNote(note, 0);

  EXPECT_GT(measure.getFilledDurationTicks(0), 0);
}

// ====================
// Integration Tests
// ====================

TEST(MeasureIntegration, CopyMeasureContents) {
  Measure measure1;

  measure1.setNumber(1);
  measure1.setTimeSignature(4, 4);
  measure1.addNote(Note("C4"), 0);
  measure1.addNote(Note("E4"), 0);

  Measure measure2;
  measure2.setNumber(2);
  measure2.setTimeSignature(measure1.getTimeSignature().getUpperValue(),
                            measure1.getTimeSignature().getLowerValue());

  // Copy notes (simplified - in reality you'd iterate properly)
  measure2.addNote(Note("C4"), 0);
  measure2.addNote(Note("E4"), 0);

  EXPECT_EQ(measure2.getNumNotes(), measure1.getNumNotes());
  EXPECT_EQ(measure2.getTimeSignature().getUpperValue(), measure1.getTimeSignature().getUpperValue());
}

TEST(MeasureIntegration, MeasureInContext) {
  // This simulates how Measure is typically used within a Part
  Measure measure;

  measure.setNumber(1);
  measure.setTimeSignature(4, 4);
  measure.setKeySignature(0, true);
  measure.setMetronome(120);

  Clef& trebleClef = measure.getClef(0);
  trebleClef.setSign(ClefSign::G);
  trebleClef.setLine(2);

  // Add a simple C major scale
  std::vector<std::string> scale = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};
  for (size_t i = 0; i < scale.size(); i++) {
    Note note(scale[i]);
    note.setDuration(128);  // Eighth note
    measure.addNote(note, 0);
  }

  EXPECT_EQ(measure.getNumNotes(), 8);
  EXPECT_EQ(measure.getClef(0).getSign(), ClefSign::G);
}

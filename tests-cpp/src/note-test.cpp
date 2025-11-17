#include "maiacore/note.h"

#include <gtest/gtest.h>

using namespace testing;

// ===================================================================================================
// CONSTRUCTORS
// ===================================================================================================

TEST(NoteConstructor, DefaultConstructor) {
  Note note;

  // Default should be A4 (MIDI 69)
  EXPECT_EQ(note.getPitch(), "A4");
  EXPECT_EQ(note.getMidiNumber(), 69);
  EXPECT_EQ(note.getPitchClass(), "A");
  EXPECT_EQ(note.getOctave(), 4);
  EXPECT_TRUE(note.isNoteOn());
  EXPECT_FALSE(note.inChord());
}

TEST(NoteConstructor, PitchStringConstructor) {
  Note c4("C4");
  Note gSharp3("G#3");
  Note bFlat5("Bb5");

  EXPECT_EQ(c4.getPitch(), "C4");
  EXPECT_EQ(gSharp3.getPitch(), "G#3");
  EXPECT_EQ(bFlat5.getPitch(), "Bb5");

  EXPECT_EQ(c4.getMidiNumber(), 60);
  EXPECT_EQ(gSharp3.getMidiNumber(), 56);
  EXPECT_EQ(bFlat5.getMidiNumber(), 82);
}

TEST(NoteConstructor, MidiNumberConstructor) {
  Note note60(60);

  EXPECT_EQ(note60.getMidiNumber(), 60);
  EXPECT_EQ(note60.getPitch(), "C4");
}

TEST(NoteConstructor, MidiNumberWithAccidentalConstructor) {
  Note noteWithSharp(61, "#");
  Note noteWithFlat(61, "b");

  EXPECT_EQ(noteWithSharp.getPitch(), "C#4");
  EXPECT_EQ(noteWithFlat.getPitch(), "Db4");
  EXPECT_EQ(noteWithSharp.getMidiNumber(), 61);
  EXPECT_EQ(noteWithFlat.getMidiNumber(), 61);
}

TEST(NoteConstructor, InvalidNoteNameLength) {
  EXPECT_THROW({ Note note("C#123"); }, std::runtime_error);
}

TEST(NoteConstructor, OnlyPitchClass_GetPitchTypesOctaveAndDuration) {
  Note noteDoubleFlat("Abb");
  Note noteFlat("Ab");
  Note noteNatural("A");
  Note noteSharp("A#");
  Note noteDoubleSharp("Ax");

  // ===== PITCHSTEP ===== //
  EXPECT_EQ(noteDoubleFlat.getPitchStep(), "A");
  EXPECT_EQ(noteFlat.getPitchStep(), "A");
  EXPECT_EQ(noteNatural.getPitchStep(), "A");
  EXPECT_EQ(noteSharp.getPitchStep(), "A");
  EXPECT_EQ(noteDoubleSharp.getPitchStep(), "A");

  // ===== PITCHCLASS ===== //
  EXPECT_EQ(noteDoubleFlat.getPitchClass(), "Abb");
  EXPECT_EQ(noteFlat.getPitchClass(), "Ab");
  EXPECT_EQ(noteNatural.getPitchClass(), "A");
  EXPECT_EQ(noteSharp.getPitchClass(), "A#");
  EXPECT_EQ(noteDoubleSharp.getPitchClass(), "Ax");

  // ===== PITCH ===== //
  EXPECT_EQ(noteDoubleFlat.getPitch(), "Abb4");
  EXPECT_EQ(noteFlat.getPitch(), "Ab4");
  EXPECT_EQ(noteNatural.getPitch(), "A4");
  EXPECT_EQ(noteSharp.getPitch(), "A#4");
  EXPECT_EQ(noteDoubleSharp.getPitch(), "Ax4");

  // ===== DEFAULT OCTAVE ===== //
  EXPECT_EQ(noteDoubleFlat.getOctave(), 4);
  EXPECT_EQ(noteFlat.getOctave(), 4);
  EXPECT_EQ(noteNatural.getOctave(), 4);
  EXPECT_EQ(noteSharp.getOctave(), 4);
  EXPECT_EQ(noteDoubleSharp.getOctave(), 4);

  // ===== DEFAULT DURATION TICKS ===== //
  EXPECT_EQ(noteDoubleFlat.getDurationTicks(), 256);
  EXPECT_EQ(noteFlat.getDurationTicks(), 256);
  EXPECT_EQ(noteNatural.getDurationTicks(), 256);
  EXPECT_EQ(noteSharp.getDurationTicks(), 256);
  EXPECT_EQ(noteDoubleSharp.getDurationTicks(), 256);
}

// ===================================================================================================
// PITCH AND OCTAVE SETTERS/GETTERS
// ===================================================================================================

TEST(NoteSetPitch, WrittenAndSoundingPitchTypesAndOctave) {
  Note note("A");

  note.setPitchClass("D");

  // ===== WRITTEN ATTRIBUTES ===== //
  EXPECT_EQ(note.getWrittenPitchClass(), "D");
  EXPECT_EQ(note.getWrittenPitch(), "D4");
  EXPECT_EQ(note.getWrittenOctave(), 4);

  // ===== SOUNDING ATTRIBUTES ===== //
  EXPECT_EQ(note.getSoundingPitchClass(), "D");
  EXPECT_EQ(note.getSoundingPitch(), "D4");
  EXPECT_EQ(note.getSoundingOctave(), 4);

  // ===== ALIAS WRITTEN ATTRIBUTES ===== //
  EXPECT_EQ(note.getPitchClass(), note.getWrittenPitchClass());
  EXPECT_EQ(note.getPitch(), note.getWrittenPitch());
  EXPECT_EQ(note.getOctave(), note.getWrittenOctave());
}

TEST(NoteSetPitch, WrittenAndSoundingPitchTypesAndOctave_TransposeInstrument) {
  Note note("A");

  note.setPitchClass("D");
  note.setTransposingInterval(-1, -2);

  // ===== WRITTEN ATTRIBUTES ===== //
  EXPECT_EQ(note.getWrittenPitchClass(), "D");
  EXPECT_EQ(note.getWrittenPitch(), "D4");
  EXPECT_EQ(note.getWrittenOctave(), 4);

  // ===== SOUNDING ATTRIBUTES ===== //
  EXPECT_EQ(note.getSoundingPitchClass(), "C");
  EXPECT_EQ(note.getSoundingPitch(), "C4");
  EXPECT_EQ(note.getSoundingOctave(), 4);
}

TEST(NoteSetPitch, WrittenAndSoundingPitchTypesAndOctave_TransposeInstrumentChangeOctave) {
  Note note("C4");

  note.setTransposingInterval(-1, -2);

  // ===== WRITTEN ATTRIBUTES ===== //
  EXPECT_EQ(note.getWrittenPitchClass(), "C");
  EXPECT_EQ(note.getWrittenPitch(), "C4");
  EXPECT_EQ(note.getWrittenOctave(), 4);

  // ===== SOUNDING ATTRIBUTES ===== //
  EXPECT_EQ(note.getSoundingPitchClass(), "Bb");
  EXPECT_EQ(note.getSoundingPitch(), "Bb3");
  EXPECT_EQ(note.getSoundingOctave(), 3);
}

TEST(NoteSetPitch, GetPitchTypesAndOctave) {
  Note noteDoubleFlat("Dbb");
  Note noteFlat("Db");
  Note noteNatural("D");
  Note noteSharp("D#");
  Note noteDoubleSharp("Dx");

  noteDoubleFlat.setPitch("A");
  noteFlat.setPitch("A");
  noteNatural.setPitch("A");
  noteSharp.setPitch("A");
  noteDoubleSharp.setPitch("A");

  // ===== PITCHSTEP ===== //
  EXPECT_EQ(noteDoubleFlat.getPitchStep(), "A");
  EXPECT_EQ(noteFlat.getPitchStep(), "A");
  EXPECT_EQ(noteNatural.getPitchStep(), "A");
  EXPECT_EQ(noteSharp.getPitchStep(), "A");
  EXPECT_EQ(noteDoubleSharp.getPitchStep(), "A");

  // ===== PITCHCLASS ===== //
  EXPECT_EQ(noteDoubleFlat.getPitchClass(), "A");
  EXPECT_EQ(noteFlat.getPitchClass(), "A");
  EXPECT_EQ(noteNatural.getPitchClass(), "A");
  EXPECT_EQ(noteSharp.getPitchClass(), "A");
  EXPECT_EQ(noteDoubleSharp.getPitchClass(), "A");

  // ===== PITCH ===== //
  EXPECT_EQ(noteDoubleFlat.getPitch(), "A4");
  EXPECT_EQ(noteFlat.getPitch(), "A4");
  EXPECT_EQ(noteNatural.getPitch(), "A4");
  EXPECT_EQ(noteSharp.getPitch(), "A4");
  EXPECT_EQ(noteDoubleSharp.getPitch(), "A4");

  // ===== OCTAVE ===== //
  EXPECT_EQ(noteDoubleFlat.getOctave(), 4);
  EXPECT_EQ(noteFlat.getOctave(), 4);
  EXPECT_EQ(noteNatural.getOctave(), 4);
  EXPECT_EQ(noteSharp.getOctave(), 4);
  EXPECT_EQ(noteDoubleSharp.getOctave(), 4);
}

TEST(NoteSetOctave, VariousOctaves) {
  Note note("C");

  note.setOctave(0);
  EXPECT_EQ(note.getOctave(), 0);
  EXPECT_EQ(note.getPitch(), "C0");
  EXPECT_EQ(note.getMidiNumber(), 12);

  note.setOctave(8);
  EXPECT_EQ(note.getOctave(), 8);
  EXPECT_EQ(note.getPitch(), "C8");
  EXPECT_EQ(note.getMidiNumber(), 108);

  // Test extreme octaves
  note.setOctave(-1);
  EXPECT_EQ(note.getOctave(), -1);

  note.setOctave(10);
  EXPECT_EQ(note.getOctave(), 10);
}

// ===================================================================================================
// EQUALITY OPERATORS
// ===================================================================================================

TEST(NoteEqualsOperator, NonEnharmonicNotes) {
  Note a("A");
  Note b("A");

  EXPECT_EQ(a == b, true);
  EXPECT_EQ(a.getMidiNumber() == b.getMidiNumber(), true);
}

TEST(NoteEqualsOperator, EnharmonicNotes) {
  Note a("A");
  Note b("Gx");

  EXPECT_EQ(a == b, false);  // Not equal because pitch names differ
  EXPECT_EQ(a.getMidiNumber() == b.getMidiNumber(), true);  // Same MIDI number
}

TEST(NoteEqualsOperator, DifferentNotes) {
  Note c("C");
  Note d("D");

  EXPECT_FALSE(c == d);
  EXPECT_TRUE(c != d);
}

TEST(NoteEqualsOperator, SameNoteDifferentOctaves) {
  Note c4("C4");
  Note c5("C5");

  EXPECT_FALSE(c4 == c5);
  EXPECT_NE(c4.getMidiNumber(), c5.getMidiNumber());
}

// ===================================================================================================
// COMPARISON OPERATORS
// ===================================================================================================

TEST(NoteComparisonOperators, LessThan) {
  Note c4("C4");
  Note d4("D4");
  Note c5("C5");

  EXPECT_TRUE(c4 < d4);
  EXPECT_TRUE(c4 < c5);
  EXPECT_FALSE(d4 < c4);
}

TEST(NoteComparisonOperators, GreaterThan) {
  Note c4("C4");
  Note d4("D4");
  Note c5("C5");

  EXPECT_TRUE(d4 > c4);
  EXPECT_TRUE(c5 > c4);
  EXPECT_FALSE(c4 > d4);
}

TEST(NoteComparisonOperators, LessOrEqual) {
  Note c4_1("C4");
  Note c4_2("C4");
  Note d4("D4");

  EXPECT_TRUE(c4_1 <= c4_2);
  EXPECT_TRUE(c4_1 <= d4);
  EXPECT_FALSE(d4 <= c4_1);
}

TEST(NoteComparisonOperators, GreaterOrEqual) {
  Note c4_1("C4");
  Note c4_2("C4");
  Note d4("D4");

  EXPECT_TRUE(c4_1 >= c4_2);
  EXPECT_TRUE(d4 >= c4_1);
  EXPECT_FALSE(c4_1 >= d4);
}

// ===================================================================================================
// ENHARMONIC METHODS
// ===================================================================================================

TEST(GetEnharmonicPitch, NonAlternativeEnharmonicPitch) {
  Note Cbb4("Cbb4");
  Note Cb4("Cb4");
  Note C4("C4");
  Note Csp4("C#4");
  Note Cdsp4("Cx4");
  Note Dbb4("Dbb4");
  Note Db4("Db4");
  Note D4("D4");
  Note Dsp4("D#4");
  Note Ddsp4("Dx4");

  // ===== Pitch Step: C ===== //
  EXPECT_EQ(Cbb4.getEnharmonicPitch(), "Bb3");
  EXPECT_EQ(Cb4.getEnharmonicPitch(), "B3");
  EXPECT_EQ(C4.getEnharmonicPitch(), "Dbb4");
  EXPECT_EQ(Csp4.getEnharmonicPitch(), "Db4");
  EXPECT_EQ(Cdsp4.getEnharmonicPitch(), "D4");

  // ===== Pitch Step: D ===== //
  EXPECT_EQ(Dbb4.getEnharmonicPitch(), "C4");
  EXPECT_EQ(Db4.getEnharmonicPitch(), "C#4");
  EXPECT_EQ(D4.getEnharmonicPitch(), "Ebb4");
  EXPECT_EQ(Dsp4.getEnharmonicPitch(), "Eb4");
  EXPECT_EQ(Ddsp4.getEnharmonicPitch(), "E4");
}

TEST(GetEnharmonicPitch, AlternativeEnharmonicPitch) {
  Note Cbb4("Cbb4");
  Note Cb4("Cb4");
  Note C4("C4");
  Note Csp4("C#4");
  Note Cdsp4("Cx4");
  Note Dbb4("Dbb4");
  Note Db4("Db4");
  Note D4("D4");
  Note Dsp4("D#4");
  Note Ddsp4("Dx4");

  // ===== Pitch Step: C ===== //
  EXPECT_EQ(Cbb4.getEnharmonicPitch(true), "A#3");
  EXPECT_EQ(Cb4.getEnharmonicPitch(true), "Ax3");
  EXPECT_EQ(C4.getEnharmonicPitch(true), "B#3");
  EXPECT_EQ(Csp4.getEnharmonicPitch(true), "Bx3");
  EXPECT_EQ(Cdsp4.getEnharmonicPitch(true), "Ebb4");

  // ===== Pitch Step: D ===== //
  EXPECT_EQ(Dbb4.getEnharmonicPitch(true), "B#3");
  EXPECT_EQ(Db4.getEnharmonicPitch(true), "Bx3");
  EXPECT_EQ(D4.getEnharmonicPitch(true), "Cx4");
  EXPECT_EQ(Dsp4.getEnharmonicPitch(true), "Fbb4");
  EXPECT_EQ(Ddsp4.getEnharmonicPitch(true), "Fb4");
}

TEST(GetEnharmonicPitches, ReturnAllEnharmonicPitches) {
  Note Cbb4("Cbb4");
  Note Cb4("Cb4");
  Note C4("C4");
  Note Csp4("C#4");
  Note Cdsp4("Cx4");
  Note Dbb4("Dbb4");
  Note Db4("Db4");
  Note D4("D4");
  Note Dsp4("D#4");
  Note Ddsp4("Dx4");

  // ===== Pitch Step: C ===== //
  EXPECT_EQ(Cbb4.getEnharmonicPitches(true),
            std::vector<std::string>({"Cbb4", "Bb3", "A#3"}));
  EXPECT_EQ(Cb4.getEnharmonicPitches(true),
            std::vector<std::string>({"Cb4", "B3", "Ax3"}));
  EXPECT_EQ(C4.getEnharmonicPitches(true),
            std::vector<std::string>({"C4", "Dbb4", "B#3"}));
  EXPECT_EQ(Csp4.getEnharmonicPitches(true),
            std::vector<std::string>({"C#4", "Db4", "Bx3"}));
  EXPECT_EQ(Cdsp4.getEnharmonicPitches(true),
            std::vector<std::string>({"Cx4", "D4", "Ebb4"}));

  // ===== Pitch Step: D ===== //
  EXPECT_EQ(Dbb4.getEnharmonicPitches(true),
            std::vector<std::string>({"Dbb4", "C4", "B#3"}));
  EXPECT_EQ(Db4.getEnharmonicPitches(true),
            std::vector<std::string>({"Db4", "C#4", "Bx3"}));
  EXPECT_EQ(D4.getEnharmonicPitches(true),
            std::vector<std::string>({"D4", "Ebb4", "Cx4"}));
  EXPECT_EQ(Dsp4.getEnharmonicPitches(true),
            std::vector<std::string>({"D#4", "Eb4", "Fbb4"}));
  EXPECT_EQ(Ddsp4.getEnharmonicPitches(true),
            std::vector<std::string>({"Dx4", "E4", "Fb4"}));
}

TEST(GetEnharmonicNotes, ReturnAllEnharmonicNotes) {
  Note Cbb4("Cbb4");
  Note Cb4("Cb4");
  Note C4("C4");
  Note Csp4("C#4");
  Note Cdsp4("Cx4");
  Note Dbb4("Dbb4");
  Note Db4("Db4");
  Note D4("D4");
  Note Dsp4("D#4");
  Note Ddsp4("Dx4");

  // ===== Pitch Step: C ===== //
  EXPECT_EQ(Cbb4.getEnharmonicNotes(true),
            std::vector<Note>({Note("Cbb4"), Note("Bb3"), Note("A#3")}));
  EXPECT_EQ(Cb4.getEnharmonicNotes(true),
            std::vector<Note>({Note("Cb4"), Note("B3"), Note("Ax3")}));
  EXPECT_EQ(C4.getEnharmonicNotes(true),
            std::vector<Note>({Note("C4"), Note("Dbb4"), Note("B#3")}));
  EXPECT_EQ(Csp4.getEnharmonicNotes(true),
            std::vector<Note>({Note("C#4"), Note("Db4"), Note("Bx3")}));
  EXPECT_EQ(Cdsp4.getEnharmonicNotes(true),
            std::vector<Note>({Note("Cx4"), Note("D4"), Note("Ebb4")}));

  // ===== Pitch Step: D ===== //
  EXPECT_EQ(Dbb4.getEnharmonicNotes(true),
            std::vector<Note>({Note("Dbb4"), Note("C4"), Note("B#3")}));
  EXPECT_EQ(Db4.getEnharmonicNotes(true),
            std::vector<Note>({Note("Db4"), Note("C#4"), Note("Bx3")}));
  EXPECT_EQ(D4.getEnharmonicNotes(true),
            std::vector<Note>({Note("D4"), Note("Ebb4"), Note("Cx4")}));
  EXPECT_EQ(Dsp4.getEnharmonicNotes(true),
            std::vector<Note>({Note("D#4"), Note("Eb4"), Note("Fbb4")}));
  EXPECT_EQ(Ddsp4.getEnharmonicNotes(true),
            std::vector<Note>({Note("Dx4"), Note("E4"), Note("Fb4")}));
}

// ===================================================================================================
// DURATION AND RHYTHM
// ===================================================================================================

TEST(NoteDuration, SetAndGetDurationTicks) {
  Note note("C4");

  note.setDuration(512, 256);  // Half note in 256 divisions
  EXPECT_EQ(note.getDurationTicks(), 512);
  EXPECT_EQ(note.getQuarterDuration(), 2.0f);
}

TEST(NoteDuration, SetAndGetQuarterDuration) {
  Note note("C4");

  note.setDuration(1.5f, 256);  // Dotted quarter
  EXPECT_EQ(note.getQuarterDuration(), 1.5f);
  EXPECT_TRUE(note.isDotted());
  EXPECT_FALSE(note.isDoubleDotted());
}

TEST(NoteDuration, DottedNotes) {
  Note dottedQuarter("C4");
  dottedQuarter.setDuration(1.5f);  // Quarter dot

  EXPECT_TRUE(dottedQuarter.isDotted());
  EXPECT_EQ(dottedQuarter.getNumDots(), 1);
}

TEST(NoteDuration, DoubleDottedNotes) {
  Note doubleDotted("C4");
  doubleDotted.setDuration(1.75f);  // Quarter dot-dot

  EXPECT_TRUE(doubleDotted.isDoubleDotted());
  EXPECT_EQ(doubleDotted.getNumDots(), 2);
}

// ===================================================================================================
// NOTE STATE (ON/OFF, GRACE, CHORD)
// ===================================================================================================

TEST(NoteState, NoteOnOff) {
  Note note("C4");

  EXPECT_TRUE(note.isNoteOn());
  EXPECT_FALSE(note.isNoteOff());

  note.setIsNoteOn(false);
  EXPECT_FALSE(note.isNoteOn());
  EXPECT_TRUE(note.isNoteOff());
}

TEST(NoteState, GraceNote) {
  Note note("C4");

  EXPECT_FALSE(note.isGraceNote());

  note.setIsGraceNote(true);
  EXPECT_TRUE(note.isGraceNote());
}

TEST(NoteState, InChord) {
  Note note("C4");

  EXPECT_FALSE(note.inChord());

  note.setIsInChord(true);
  EXPECT_TRUE(note.inChord());
}

// ===================================================================================================
// VOICE AND STAFF
// ===================================================================================================

TEST(NoteVoiceStaff, SetAndGetVoice) {
  Note note("C4");

  note.setVoice(1);
  EXPECT_EQ(note.getVoice(), 1);

  note.setVoice(4);
  EXPECT_EQ(note.getVoice(), 4);
}

TEST(NoteVoiceStaff, SetAndGetStaff) {
  Note note("C4");

  note.setStaff(1);
  EXPECT_EQ(note.getStaff(), 1);

  note.setStaff(2);
  EXPECT_EQ(note.getStaff(), 2);
}

// ===================================================================================================
// TIES, SLURS, ARTICULATIONS, BEAMS
// ===================================================================================================

TEST(NoteTies, AddAndGetTies) {
  Note note("C4");

  EXPECT_TRUE(note.getTie().empty());

  note.addTie("start");
  EXPECT_EQ(note.getTie().size(), 1);
  EXPECT_EQ(note.getTie()[0], "start");

  note.addTie("stop");
  EXPECT_EQ(note.getTie().size(), 2);
}

TEST(NoteTies, SetTieStart) {
  Note note("C4");

  note.setTieStart();
  auto ties = note.getTie();
  EXPECT_FALSE(ties.empty());
  EXPECT_EQ(ties[0], "start");
}

TEST(NoteTies, SetTieStop) {
  Note note("C4");

  note.setTieStop();
  auto ties = note.getTie();
  EXPECT_FALSE(ties.empty());
  EXPECT_EQ(ties[0], "stop");
}

TEST(NoteTies, SetTieStopStart) {
  Note note("C4");

  note.setTieStopStart();
  auto ties = note.getTie();
  EXPECT_EQ(ties.size(), 2);
}

TEST(NoteTies, RemoveTies) {
  Note note("C4");

  note.addTie("start");
  note.addTie("stop");
  EXPECT_EQ(note.getTie().size(), 2);

  note.removeTies();
  EXPECT_TRUE(note.getTie().empty());
}

TEST(NoteSlurs, AddAndGetSlur) {
  Note note("C4");

  note.addSlur("start", "above");
  auto slur = note.getSlur();
  EXPECT_EQ(slur.first, "start");
  EXPECT_EQ(slur.second, "above");
}

TEST(NoteArticulations, AddAndGetArticulations) {
  Note note("C4");

  EXPECT_TRUE(note.getArticulation().empty());

  note.addArticulation("staccato");
  EXPECT_EQ(note.getArticulation().size(), 1);
  EXPECT_EQ(note.getArticulation()[0], "staccato");

  note.addArticulation("accent");
  EXPECT_EQ(note.getArticulation().size(), 2);
}

TEST(NoteBeams, AddAndGetBeams) {
  Note note("C4");

  EXPECT_TRUE(note.getBeam().empty());

  note.addBeam("begin");
  EXPECT_EQ(note.getBeam().size(), 1);
  EXPECT_EQ(note.getBeam()[0], "begin");

  note.addBeam("continue");
  EXPECT_EQ(note.getBeam().size(), 2);
}

// ===================================================================================================
// STEM DIRECTION
// ===================================================================================================

TEST(NoteStem, SetAndGetStem) {
  Note note("C4");

  note.setStem("up");
  EXPECT_EQ(note.getStem(), "up");

  note.setStem("down");
  EXPECT_EQ(note.getStem(), "down");
}

// ===================================================================================================
// TUPLETS
// ===================================================================================================

TEST(NoteTuplet, SetAndCheckTuplet) {
  Note note("C4");

  EXPECT_FALSE(note.isTuplet());

  note.setIsTuplet(true);
  EXPECT_TRUE(note.isTuplet());
}

TEST(NoteTuplet, SetTupleValues) {
  Note note("C4");

  note.setIsTuplet(true);
  note.setTupleValues(3, 2, "eighth");

  EXPECT_TRUE(note.isTuplet());
  // Note: Duration class should handle the tuplet values
}

// ===================================================================================================
// PITCHED/UNPITCHED
// ===================================================================================================

TEST(NotePitched, SetAndCheckPitched) {
  Note note("C4");

  EXPECT_TRUE(note.isPitched());

  note.setIsPitched(false);
  EXPECT_FALSE(note.isPitched());
}

TEST(NoteUnpitched, SetAndGetUnpitchedIndex) {
  Note note("C4");
  note.setIsPitched(false);

  note.setUnpitchedIndex(42);
  EXPECT_EQ(note.getUnpitchedIndex(), 42);
}

// ===================================================================================================
// TRANSPOSITION
// ===================================================================================================

TEST(NoteTransposition, SetAndGetTransposingInterval) {
  Note note("C4");

  EXPECT_FALSE(note.isTransposed());
  EXPECT_EQ(note.getTransposeDiatonic(), 0);
  EXPECT_EQ(note.getTransposeChromatic(), 0);

  note.setTransposingInterval(-1, -2);

  EXPECT_TRUE(note.isTransposed());
  EXPECT_EQ(note.getTransposeDiatonic(), -1);
  EXPECT_EQ(note.getTransposeChromatic(), -2);
}

TEST(NoteTransposition, TransposeMethod) {
  Note note("C4");

  note.transpose(2);  // Transpose up 2 semitones
  EXPECT_EQ(note.getPitch(), "D4");
  EXPECT_EQ(note.getMidiNumber(), 62);

  note.transpose(-2);  // Transpose back down
  EXPECT_EQ(note.getPitch(), "C4");
  EXPECT_EQ(note.getMidiNumber(), 60);
}

TEST(NoteTransposition, TransposeAcrossOctave) {
  Note note("B3");

  note.transpose(2);  // Should become C#4
  EXPECT_EQ(note.getOctave(), 4);
  EXPECT_EQ(note.getMidiNumber(), 61);
}

TEST(NoteTransposition, TransposeDownAcrossOctave) {
  Note note("C4");

  note.transpose(-2);  // Should become Bb3
  EXPECT_EQ(note.getOctave(), 3);
  EXPECT_EQ(note.getMidiNumber(), 58);
}

// ===================================================================================================
// MIDI AND FREQUENCY
// ===================================================================================================

TEST(NoteMidi, GetMidiNumber) {
  Note c0("C0");
  Note a4("A4");
  Note c8("C8");

  EXPECT_EQ(c0.getMidiNumber(), 12);
  EXPECT_EQ(a4.getMidiNumber(), 69);
  EXPECT_EQ(c8.getMidiNumber(), 108);
}

TEST(NoteFrequency, GetFrequency) {
  Note a4("A4");

  // A4 should be 440 Hz
  float freq = a4.getFrequency();
  EXPECT_NEAR(freq, 440.0f, 0.01f);
}

TEST(NoteFrequency, GetFrequencyWithCustomA4) {
  Note a4("A4");

  // Test with A4 = 442 Hz (some orchestras tune higher)
  float freq = a4.getFrequency(442.0f);
  EXPECT_NEAR(freq, 442.0f, 0.01f);
}

// ===================================================================================================
// ALTER SYMBOL
// ===================================================================================================

TEST(NoteAlterSymbol, GetAlterSymbol) {
  Note noteSharp("C#4");
  Note noteFlat("Db4");
  Note noteNatural("C4");

  EXPECT_EQ(noteSharp.getAlterSymbol(), "#");
  EXPECT_EQ(noteFlat.getAlterSymbol(), "b");
  EXPECT_EQ(noteNatural.getAlterSymbol(), "");
}

// ===================================================================================================
// EDGE CASES
// ===================================================================================================

TEST(NoteEdgeCases, RestNote) {
  // MIDI -1 indicates a rest
  Note rest(-1);

  EXPECT_TRUE(rest.isNoteOff());
  EXPECT_FALSE(rest.isNoteOn());
}

TEST(NoteEdgeCases, ExtremeOctaves) {
  Note lowC("C-1");
  Note highC("C10");

  EXPECT_EQ(lowC.getOctave(), -1);
  EXPECT_EQ(highC.getOctave(), 10);
}

TEST(NoteEdgeCases, AllPitchClasses) {
  // Test all 12 chromatic pitches
  std::vector<std::string> pitches = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  std::vector<int> expectedMidi = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71};

  for (size_t i = 0; i < pitches.size(); i++) {
    Note note(pitches[i] + "4");
    EXPECT_EQ(note.getMidiNumber(), expectedMidi[i]);
  }
}

TEST(NoteEdgeCases, DoubleAccidentals) {
  Note doubleSharp("Cx4");
  Note doubleFlat("Dbb4");

  EXPECT_EQ(doubleSharp.getPitchClass(), "Cx");
  EXPECT_EQ(doubleFlat.getPitchClass(), "Dbb");

  // Cx4 = D4 = MIDI 62
  EXPECT_EQ(doubleSharp.getMidiNumber(), 62);
  // Dbb4 = C4 = MIDI 60
  EXPECT_EQ(doubleFlat.getMidiNumber(), 60);
}

// ===================================================================================================
// DIATONIC PITCH CLASS
// ===================================================================================================

TEST(NoteDiatonic, GetDiatonicPitchClass) {
  Note cSharp("C#4");
  Note dFlat("Db4");

  EXPECT_EQ(cSharp.getDiatonicWrittenPitchClass(), "C");
  EXPECT_EQ(dFlat.getDiatonicWrittenPitchClass(), "D");

  EXPECT_EQ(cSharp.getDiatonicSoundingPitchClass(), "C");
  EXPECT_EQ(dFlat.getDiatonicSoundingPitchClass(), "D");
}

// ===================================================================================================
// NOTE TYPE (RHYTHM FIGURE STRING)
// ===================================================================================================

TEST(NoteType, GetTypeStrings) {
  Note quarter("C4", RhythmFigure::QUARTER);

  // These methods return string representations of the note type
  std::string type = quarter.getType();
  std::string longType = quarter.getLongType();
  std::string shortType = quarter.getShortType();

  EXPECT_FALSE(type.empty());
  EXPECT_FALSE(longType.empty());
  EXPECT_FALSE(shortType.empty());
}

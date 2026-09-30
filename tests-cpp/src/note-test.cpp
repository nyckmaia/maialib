#include "maiacore/note.h"

#include <gtest/gtest.h>

#include <functional>
#include <limits>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "maiacore/helper.h"
#include "maiacore/interval.h"
#include "pitch-spelling-legacy-data.h"
#include "test-capture.h"
#include "transposing-instruments.h"

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
    note.setOctave(0);
    EXPECT_EQ(note.getOctave(), 0);

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
    EXPECT_EQ(Cbb4.getEnharmonicPitches(true), std::vector<std::string>({"Cbb4", "Bb3", "A#3"}));
    EXPECT_EQ(Cb4.getEnharmonicPitches(true), std::vector<std::string>({"Cb4", "B3", "Ax3"}));
    EXPECT_EQ(C4.getEnharmonicPitches(true), std::vector<std::string>({"C4", "Dbb4", "B#3"}));
    EXPECT_EQ(Csp4.getEnharmonicPitches(true), std::vector<std::string>({"C#4", "Db4", "Bx3"}));
    EXPECT_EQ(Cdsp4.getEnharmonicPitches(true), std::vector<std::string>({"Cx4", "D4", "Ebb4"}));

    // ===== Pitch Step: D ===== //
    EXPECT_EQ(Dbb4.getEnharmonicPitches(true), std::vector<std::string>({"Dbb4", "C4", "B#3"}));
    EXPECT_EQ(Db4.getEnharmonicPitches(true), std::vector<std::string>({"Db4", "C#4", "Bx3"}));
    EXPECT_EQ(D4.getEnharmonicPitches(true), std::vector<std::string>({"D4", "Ebb4", "Cx4"}));
    EXPECT_EQ(Dsp4.getEnharmonicPitches(true), std::vector<std::string>({"D#4", "Eb4", "Fbb4"}));
    EXPECT_EQ(Ddsp4.getEnharmonicPitches(true), std::vector<std::string>({"Dx4", "E4", "Fb4"}));
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

// Note::transpose() takes a float, so half a semitone is an interval it accepts, and it computes
// on exact pitch positions: rounding the pitch to a MIDI integer BEFORE applying the interval
// would destroy any quarter tone it was given.
TEST(NoteTransposition, TransposeByQuarterTone) {
    Note note("C4");

    note.transpose(0.5f);
    EXPECT_EQ(note.getPitch(), "C1x4");
    EXPECT_TRUE(note.isQuarterTone());
    EXPECT_EQ(note.getMidiNumber(), 61);  // ties upward, as getMidiNumber() always has

    note.transpose(-0.5f);  // and back to where it started
    EXPECT_EQ(note.getPitch(), "C4");
    EXPECT_FALSE(note.isQuarterTone());
    EXPECT_EQ(note.getMidiNumber(), 60);
}

TEST(NoteTransposition, TransposePreservesAQuarterToneAcrossAWholeToneInterval) {
    Note note("C1x4");

    note.transpose(2.0f);
    EXPECT_EQ(note.getPitch(), "D1x4");  // not rounded to "D4"
    EXPECT_TRUE(note.isQuarterTone());
}

TEST(NoteTransposition, TransposeRejectsAnIntervalOffTheQuarterToneGrid) {
    Note note("C4");

    try {
        note.transpose(0.3f);
        FAIL() << "Expected std::runtime_error for a transposition off the quarter-tone grid";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("multiple of 0.5"), std::string::npos) << "message: " << what;
        EXPECT_NE(what.find("0.3"), std::string::npos) << "message: " << what;
    }

    EXPECT_EQ(note.getPitch(), "C4");  // the refused call changed nothing
}

// A note transposed below the representable range raises, and is left exactly as it was: it
// must never be silently turned into a rest, which storing Helper::steps2pitch()'s rest sentinel
// for a position below MIDI 0 would do.
TEST(NoteTransposition, TransposeOutOfRangeRaisesAndLeavesTheNoteUnchanged) {
    Note note("C4");

    try {
        note.transpose(-61.0f);
        FAIL() << "Expected std::runtime_error; the note became '" << note.getPitch() << "'";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("outside the representable range"), std::string::npos)
            << "message: " << what;
        EXPECT_NE(what.find("'C4'"), std::string::npos) << "message: " << what;
    }

    EXPECT_EQ(note.getPitch(), "C4");
    EXPECT_TRUE(note.isNoteOn());
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
    Note lowC("C0");
    Note highC("C10");

    EXPECT_EQ(lowC.getOctave(), 0);
    EXPECT_EQ(highC.getOctave(), 10);
}

TEST(NoteEdgeCases, AllPitchClasses) {
    // Test all 12 chromatic pitches
    std::vector<std::string> pitches = {"C",  "C#", "D",  "D#", "E",  "F",
                                        "F#", "G",  "G#", "A",  "A#", "B"};
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

// ===================================================================================================
// PITCH SPELLING
// ===================================================================================================

TEST(PitchSpellingLegacy, EnharmonicTableMatches) {
    for (const auto& entry : kLegacyEnharmonicTable) {
        const Note note(entry.pitch);
        EXPECT_EQ(note.getEnharmonicPitch(false), entry.defaultPitch) << "pitch: " << entry.pitch;
        EXPECT_EQ(note.getEnharmonicPitch(true), entry.alternativePitch)
            << "pitch: " << entry.pitch;
    }
}

TEST(PitchSpelling, NoteAcceptsFullRange) {
    for (const auto& entry : kFullRangeMidiTable) {
        const Note note(entry.pitch);
        EXPECT_EQ(note.getMidiNumber(), entry.midiNumber) << "pitch: " << entry.pitch;
        EXPECT_EQ(note.getPitch(), entry.pitch) << "pitch: " << entry.pitch;
    }
}

TEST(PitchSpelling, NoteEdgeCases) {
    EXPECT_EQ(Note("C-1").getMidiNumber(), 0);
    EXPECT_EQ(Note("C-1").getOctave(), -1);
    EXPECT_EQ(Note("Cbb10").getMidiNumber(), 130);
    EXPECT_EQ(Note(5).getPitch(), "F-1");
    EXPECT_EQ(Note(157, "x").getPitch(), "Bx11");
    EXPECT_THROW({ Note note("Cb-1"); }, std::runtime_error);
    EXPECT_THROW({ Note note("C12"); }, std::runtime_error);
    EXPECT_THROW({ Note note("C#123"); }, std::runtime_error);
}

TEST(PitchSpelling, SetPitchResetsAccidentalAndParsesOctaves) {
    Note note("C#4");
    note.setPitch("D4");
    EXPECT_EQ(note.getAlterSymbol(), "");
    EXPECT_EQ(note.getMidiNumber(), 62);

    note.setPitch("C10");
    EXPECT_EQ(note.getPitch(), "C10");
    EXPECT_EQ(note.getMidiNumber(), 132);

    note.setPitch("Bb-1");
    EXPECT_EQ(note.getOctave(), -1);
    EXPECT_EQ(note.getMidiNumber(), 10);
    EXPECT_EQ(note.getAlterSymbol(), "b");
}

TEST(PitchSpelling, FullRangeEnharmonicTable) {
    for (const auto& entry : kFullRangeEnharmonicTable) {
        const Note note(entry.pitch);
        EXPECT_EQ(note.getEnharmonicPitch(false), entry.defaultPitch) << "pitch: " << entry.pitch;
        EXPECT_EQ(note.getEnharmonicPitch(true), entry.alternativePitch)
            << "pitch: " << entry.pitch;
    }
}

TEST(PitchSpelling, EnharmonicOutputsAreValidNotes) {
    for (const auto& entry : kFullRangeMidiTable) {
        const Note note(entry.pitch);
        for (const bool alternative : {false, true}) {
            const std::string enharmonic = note.getEnharmonicPitch(alternative);
            EXPECT_EQ(Note(enharmonic).getMidiNumber(), entry.midiNumber)
                << "pitch: " << entry.pitch << " enharmonic: " << enharmonic;
        }
    }
}

TEST(PitchSpelling, EnharmonicRangeFallback) {
    EXPECT_EQ(Note("Bx11").getEnharmonicPitch(false), "Bx11");
    EXPECT_EQ(Note("Bx11").getEnharmonicPitch(true), "Bx11");
    EXPECT_EQ(Note("B11").getEnharmonicPitch(false), "B11");
    EXPECT_EQ(Note("B11").getEnharmonicPitch(true), "Ax11");
    EXPECT_EQ(Note("C-1").getEnharmonicPitch(false), "Dbb-1");
    EXPECT_EQ(Note("C-1").getEnharmonicPitch(true), "Dbb-1");
    EXPECT_EQ(Note("G#4").getEnharmonicPitches(true),
              std::vector<std::string>({"G#4", "Ab4", "Ab4"}));
}

// ===== Quarter-tone enharmonic spellings ===== //
//
// A quarter tone's partners are the white-key steps within 1.5 semitones of it, spelled with the
// quarter-tone accidental that separates them in that step's own octave. The default is the
// partner with the smaller alter, a tie going to the side opposite the note's own accidental; the
// alternative is the other partner, or the default when there is only one.

namespace {
using SpellingPair = std::pair<std::string, std::string>;

// The default and the alternative enharmonic spelling of 'note'.
SpellingPair enharmonicsOf(const Note& note) {
    return {note.getEnharmonicPitch(false), note.getEnharmonicPitch(true)};
}
}  // namespace

TEST(QuarterToneEnharmonic, aQuarterToneHasOneOrTwoPartners) {
    EXPECT_EQ(enharmonicsOf(Note("C1x4")), SpellingPair("D3b4", "B3x3"));  // 60.5: two
    EXPECT_EQ(enharmonicsOf(Note("C3x4")), SpellingPair("D1b4", "D1b4"));  // 61.5: one
    EXPECT_EQ(enharmonicsOf(Note("E1b4")), SpellingPair("D3x4", "F3b4"));  // 63.5: two
    EXPECT_EQ(enharmonicsOf(Note("E1x4")), SpellingPair("F1b4", "F1b4"));  // 64.5: one
}

TEST(QuarterToneEnharmonic, theDefaultIsTheSmallerAlterThenTheOppositeSide) {
    // One partner is nearer: it is the default, whichever side it is on.
    EXPECT_EQ(enharmonicsOf(Note("D3b4")), SpellingPair("C1x4", "B3x3"));
    EXPECT_EQ(enharmonicsOf(Note("B3x3")), SpellingPair("C1x4", "D3b4"));
    EXPECT_EQ(enharmonicsOf(Note("F3b4")), SpellingPair("E1b4", "D3x4"));
    // Both partners equally far: the one opposite the note's own accidental, as C#4 -> Db4.
    EXPECT_EQ(enharmonicsOf(Note("C1x4")).first, "D3b4");  // sharp side -> flat side
    EXPECT_EQ(enharmonicsOf(Note("E1b4")).first, "D3x4");  // flat side -> sharp side
    EXPECT_EQ(enharmonicsOf(Note("B1b3")), SpellingPair("A3x3", "C3b4"));
}

// Octaves -1..11 bound the partners exactly as they bound the semitone spellings: a partner
// outside them does not exist, and the range fallback applies.
TEST(QuarterToneEnharmonic, aPartnerOutsideTheOctaveRangeDoesNotExist) {
    // Top: C3b12 (154.5) would be A3x11's second partner, C1b12 (155.5) B1x11's only one.
    EXPECT_EQ(enharmonicsOf(Note("A3x11")), SpellingPair("B1b11", "B1b11"));
    EXPECT_EQ(enharmonicsOf(Note("B1x11")), SpellingPair("B1x11", "B1x11"));
    // Bottom: B3x-2 (0.5) would be C1x-1's second partner, B1x-2 (-0.5) C1b-1's only one.
    EXPECT_EQ(enharmonicsOf(Note("C1x-1")), SpellingPair("D3b-1", "D3b-1"));
    EXPECT_EQ(enharmonicsOf(Note("C1b-1")), SpellingPair("C1b-1", "C1b-1"));
}

// The rest of the family builds on getEnharmonicPitch() exactly as it does for a semitone,
// repeats included.
TEST(QuarterToneEnharmonic, theWholeFamilyRespellsAQuarterTone) {
    EXPECT_EQ(Note("C1x4").getEnharmonicPitches(true),
              std::vector<std::string>({"C1x4", "D3b4", "B3x3"}));
    EXPECT_EQ(Note("C1x4").getEnharmonicPitches(false), std::vector<std::string>({"D3b4", "B3x3"}));
    EXPECT_EQ(Note("C3x4").getEnharmonicPitches(true),
              std::vector<std::string>({"C3x4", "D1b4", "D1b4"}));

    EXPECT_EQ(Note("E1b4").getEnharmonicNote(false).getPitch(), "D3x4");
    EXPECT_EQ(Note("E1b4").getEnharmonicNote(true).getPitch(), "F3b4");

    std::vector<std::string> notePitches;
    for (const Note& note : Note("E1b4").getEnharmonicNotes(true)) {
        notePitches.push_back(note.getPitch());
    }
    EXPECT_EQ(notePitches, std::vector<std::string>({"E1b4", "D3x4", "F3b4"}));

    Note respelled("C1x4");
    respelled.toEnharmonicPitch();
    EXPECT_EQ(respelled.getPitch(), "D3b4");
    respelled.toEnharmonicPitch(true);
    EXPECT_EQ(respelled.getPitch(), "B3x3");
}

// The family respells the sounding pitch, which is what getPitch() reports: a B-flat clarinet's
// written C1x4 sounds B1b3 (58.5), respelled A3x3 or C3b4 -- not D3b4 or B3x3, the partners of
// the written pitch.
TEST(QuarterToneEnharmonic, theSoundingPitchIsRespelled) {
    const Note clarinet("C1x4", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                        /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    ASSERT_EQ(clarinet.getPitch(), "B1b3");
    EXPECT_EQ(enharmonicsOf(clarinet), SpellingPair("A3x3", "C3b4"));
}

// Every quarter-tone spelling in the representable range is respelled into spellings of the same
// exact position, each a quarter tone, and differing from the note's own unless it has no
// partner.
TEST(QuarterToneEnharmonic, everyRespellingDescribesTheSamePitch) {
    int numSpellings = 0;
    int numWithoutPartner = 0;
    for (const std::string& step : c_C_diatonicScale) {
        for (const std::string symbol : {"3b", "1b", "1x", "3x"}) {
            for (int octave = c_minPitchOctave; octave <= c_maxPitchOctave; octave++) {
                if (Helper::spelling2midiNote(step, Helper::alterSymbol2Value(symbol), octave) <
                    0) {
                    continue;  // below the lowest representable pitch, C1b-1
                }
                const std::string pitch = step + symbol + std::to_string(octave);
                const Note note(pitch);
                numSpellings++;
                const auto [defaultPitch, alternativePitch] = enharmonicsOf(note);
                for (const std::string& respelling : {defaultPitch, alternativePitch}) {
                    const Note respelled(respelling);
                    EXPECT_EQ(respelled.getQuarterToneSteps(), note.getQuarterToneSteps())
                        << pitch << " -> " << respelling;
                    EXPECT_TRUE(respelled.isQuarterTone()) << pitch << " -> " << respelling;
                }
                if (defaultPitch == pitch) {
                    numWithoutPartner++;
                    EXPECT_EQ(alternativePitch, pitch) << pitch;
                } else {
                    EXPECT_NE(alternativePitch, pitch) << pitch;
                }
            }
        }
    }
    EXPECT_EQ(numSpellings, 7 * 4 * 13 - 1);  // every spelling but C3b-1 (-1.5)
    // B1x11 (155.5), B3x11 (156.5) and C1b-1 (-0.5) are the only positions with one spelling.
    EXPECT_EQ(numWithoutPartner, 3);
}

// =====================================================================================
// NOTE COMPOSES PITCH
// =====================================================================================

// setPitchClass() changes the accidental and the MIDI number together: Note holds a single Pitch
// and derives getMidiNumber() from it on demand, so no stored MIDI number can keep reporting the
// note's previous pitch.
TEST(Note, setPitchClassUpdatesAccidentalAndMidi) {
    Note n("C4");
    n.setPitchClass("Eb");
    EXPECT_EQ(n.getAlterSymbol(), "b");
    EXPECT_EQ(n.getMidiNumber(), 63);
}

// A rest has no octave. Note's three octave getters return std::optional<int>, empty for a rest:
// 0 and -1 are both legitimate octaves, so no int value is safe to use as a stand-in.
TEST(Note, restHasNoOctaveAnywhere) {
    const Note rest("");
    EXPECT_FALSE(rest.getOctave().has_value());
    EXPECT_FALSE(rest.getWrittenOctave().has_value());
    EXPECT_FALSE(rest.getSoundingOctave().has_value());

    const Note note("C#4");
    EXPECT_EQ(note.getOctave().value(), 4);
    EXPECT_EQ(note.getWrittenOctave().value(), 4);
}

// Note(pitch, isNoteOn=false) is a fully consistent rest -- every getter agrees, not just
// isNoteOn(). The "C4" is deliberately discarded (see the constructor's rest guard).
TEST(NoteComposesPitch, ConstructorIsNoteOnFalseIsAFullyConsistentRest) {
    const Note n("C4", RhythmFigure::QUARTER, /*isNoteOn=*/false);
    EXPECT_FALSE(n.isNoteOn());
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_EQ(n.getPitchClass(), "rest");
    EXPECT_EQ(n.getPitch(), "rest");
    EXPECT_EQ(n.getMidiNumber(), -1);
    EXPECT_EQ(n.getPitchStep(), "rest");
    EXPECT_FALSE(n.getOctave().has_value());  // a rest has no octave: empty, not a sentinel
}

// setIsNoteOn(false) makes the note a rest everywhere at once: isNoteOn() derives from
// _writtenPitch.isRest(), and setIsNoteOn(false) goes through _writtenPitch.setPitch("rest"), so no
// getter can keep reporting the pitch the note had.
TEST(NoteComposesPitch, SetIsNoteOnFalseReportsRestEverywhere) {
    Note n("C#4");
    n.setIsNoteOn(false);
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_FALSE(n.isNoteOn());
    EXPECT_EQ(n.getPitchClass(), "rest");
    EXPECT_EQ(n.getPitch(), "rest");
    EXPECT_EQ(n.getMidiNumber(), -1);
    EXPECT_EQ(n.getPitchStep(), "rest");
    EXPECT_FALSE(n.getOctave().has_value());
}

// setIsNoteOn(true) on a rest carries no pitch to resurrect one with, so it must refuse (LOG_WARN,
// no throw) and leave the rest a rest.
TEST(NoteComposesPitch, SetIsNoteOnTrueOnRestRefusesAndWarns) {
    Note n("");
    ASSERT_TRUE(n.isNoteOff());
    n.setIsNoteOn(true);  // must not throw
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_FALSE(n.isNoteOn());
    EXPECT_EQ(n.getWrittenPitchStep(), "rest");
    EXPECT_EQ(n.getPitchClass(), "rest");
}

// All three octave getters derive from Pitch's std::optional<int> octave, so they agree that a
// rest has no octave: an EMPTY optional, never a numeric sentinel (0 and -1 are both legitimate
// octaves).
TEST(NoteComposesPitch, AllOctaveGettersAgreeRestHasNoOctave) {
    const Note n("rest");
    EXPECT_FALSE(n.getOctave().has_value());
    EXPECT_FALSE(n.getWrittenOctave().has_value());
    EXPECT_FALSE(n.getSoundingOctave().has_value());
}

// setOctave() delegates to Pitch::setOctave() and inherits its refuse-on-rest policy: a bare
// octave carries no pitch to resurrect a rest with.
TEST(NoteComposesPitch, SetOctaveOnRestRefusesAndWarns) {
    Note n("rest");
    n.setOctave(5);  // must not throw
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_FALSE(n.getOctave().has_value());
    EXPECT_EQ(n.getPitchClass(), "rest");
}

// setAlter() delegates to Pitch::setAlter() and inherits the same refuse-on-rest policy as
// setOctave(): neither carries enough information to resurrect a rest.
TEST(NoteComposesPitch, SetAlterOnRestRefusesAndWarns) {
    Note n("rest");
    n.setAlter(0.5f);  // must not throw
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_FALSE(n.getOctave().has_value());
    EXPECT_EQ(n.getPitchClass(), "rest");
}

// setStep() delegates to Pitch::setStep() and is permissive on a rest, unlike
// setOctave()/setAlter(): a diatonic step is enough to resurrect one, defaulting the octave to 4.
TEST(NoteComposesPitch, SetStepResurrectsRestToOctave4) {
    Note n("rest");
    n.setStep("C");
    EXPECT_TRUE(n.isNoteOn());
    EXPECT_FALSE(n.isNoteOff());
    EXPECT_EQ(n.getPitch(), "C4");
    EXPECT_EQ(n.getOctave(), 4);
}

// setIsNoteOn(false) deliberately keeps a note's transposing interval: silencing a transposing
// instrument's note is temporary, and the instrument it belongs to does not stop transposing
// while it is quiet. setStep() is permissive on a rest (SetStepResurrectsRestToOctave4 above) and
// resurrects it at octave 4 -- but it only touches _writtenPitch; it has no reason to touch
// _transposeDiatonic/_transposeChromatic; and it should not guess whether the caller wants the
// note to keep belonging to the same transposing instrument. So the interval survives
// resurrection too. That rests on the instrument-ownership argument, not on numeric coincidence.
//
// The note starts from C5, not C4: setStep() revives ANY rest at a HARDCODED octave 4, so from C4
// "the octave survived silencing" and "the octave was just reset to the resurrection default"
// would look identical. What is pinned is SELF-CONSISTENCY -- the revived note's sounding pitch is
// freshly derived from its (defaulted) written pitch and the surviving interval, not a stale
// leftover of the pre-silence sounding pitch -- not that a revived note equals a fresh note
// carrying the interval, which would assume the interval SHOULD survive.
TEST(NoteComposesPitch, SetStepAfterSetIsNoteOnFalseKeepsTransposingInterval) {
    Note n("C5", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
           /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    ASSERT_EQ(n.getWrittenPitch(), "C5");
    ASSERT_EQ(n.getSoundingPitch(), "Bb4");

    n.setIsNoteOn(false);
    ASSERT_TRUE(n.isNoteOff());
    // the interval survives silencing (documented on setIsNoteOn())
    ASSERT_TRUE(n.isTransposed());

    n.setStep("C");
    EXPECT_TRUE(n.isNoteOn());
    EXPECT_TRUE(n.isTransposed());
    EXPECT_EQ(n.getTransposeDiatonic(), -1);
    EXPECT_EQ(n.getTransposeChromatic(), -2);
    // Discriminates octave handling: NOT preserved from before silencing (that would be "C5");
    // this is setStep()'s ordinary hardcoded-4 default (SetStepResurrectsRestToOctave4),
    // unaffected by the interval.
    EXPECT_EQ(n.getWrittenPitch(), "C4");
    // Discriminates staleness: freshly derived from the CURRENT written pitch ("C4") and the
    // surviving interval, giving "Bb3" -- never the pre-silence sounding pitch "Bb4", which
    // would indicate a leftover/cached value rather than a live recomputation.
    EXPECT_EQ(n.getSoundingPitch(), "Bb3");
    EXPECT_NE(n.getSoundingPitch(), "Bb4");
}

// getAlterSymbol() reads the sounding pitch, like every unqualified Note getter, not the written
// one. A transposing instrument shows the two diverging. B-flat clarinet: written C sounds a
// major second lower (concert Bb). The same (pitch="C4", transposeDiatonic=-1,
// transposeChromatic=-2) construction sounds "Bb3" in
// NoteSetPitch.WrittenAndSoundingPitchTypesAndOctave_TransposeInstrumentChangeOctave too:
//   written "C4"  -> alter symbol ""  (natural)
//   sounding "Bb3" -> alter symbol "b" (flat), MIDI 58
TEST(NoteComposesPitch, GetAlterSymbolForwardsToSoundingPitchOnTransposedNote) {
    // Untransposed stand-in for "the written pitch's own accidental symbol": Note has no
    // getWrittenAlterSymbol() getter, and for an untransposed note sounding == written.
    const Note written("C4");
    ASSERT_EQ(written.getAlterSymbol(), "");

    const Note transposed("C4", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                          /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    ASSERT_EQ(transposed.getSoundingPitch(), "Bb3");
    ASSERT_EQ(transposed.getMidiNumber(), 58);
    EXPECT_EQ(transposed.getAlterSymbol(), "b");
    EXPECT_NE(transposed.getAlterSymbol(), written.getAlterSymbol());
}

// A transposing instrument's sounding pitch is the written pitch's exact position moved by the
// chromatic interval, spelled from there (see computeSoundingPitch() in note.cpp). getMidiNumber()
// is that arithmetic, and getPitch() and getOctave() spell the same position:
//   case                        getPitch()   getOctave()   getMidiNumber()
//   B-flat clarinet C#4 -1/-2   "B3"         3             59
//   horn in F       F#4 -4/-7   "B3"         3             59
//   piccolo         C4  +7/+12  "C5"         5             72
// The clarinet and the horn both sound MIDI 59, spelled B3. The piccolo sounds an octave above the
// written pitch: an interval of exactly +12 semitones moves the octave too.
TEST(NoteComposesPitch, GetMidiNumberIsArithmeticForBFlatClarinet) {
    // B-flat clarinet: written C#4 sounds a major second lower.
    const Note n("C#4", RhythmFigure::QUARTER, true, false, -1, -2);
    EXPECT_EQ(n.getMidiNumber(), 59);
    EXPECT_EQ(n.getOctave(), 3);
    EXPECT_EQ(n.getPitch(), "B3");  // MIDI 59 is B3
}

TEST(NoteComposesPitch, GetMidiNumberIsArithmeticForHornInF) {
    // Horn in F: written F#4 sounds a perfect fifth lower.
    const Note n("F#4", RhythmFigure::QUARTER, true, false, -4, -7);
    EXPECT_EQ(n.getMidiNumber(), 59);
    EXPECT_EQ(n.getOctave(), 3);
    EXPECT_EQ(n.getPitch(), "B3");  // MIDI 59 is B3
}

TEST(NoteComposesPitch, GetMidiNumberIsArithmeticForPiccolo) {
    // Piccolo: written C4 sounds an octave higher.
    const Note n("C4", RhythmFigure::QUARTER, true, false, 7, 12);
    EXPECT_EQ(n.getMidiNumber(), 72);
    // An exact +12 transpose moves the octave, so the sounding octave is one above the written
    // one, and getOctave() agrees with getSoundingOctave() below.
    EXPECT_EQ(n.getOctave(), 5);
    EXPECT_EQ(n.getSoundingOctave(), 5);
    EXPECT_EQ(n.getPitch(), "C5");
}

// getEnharmonicPitch() respells the sounding pitch getPitch() reports, so the two agree on the
// octave: for the piccolo, getPitch() is "C5" and the respellings are "Dbb5" and "B#4" -- the same
// pitch, never one spelled a full octave apart ("Dbb4").
TEST(NoteComposesPitch, GetEnharmonicPitchAgreesWithGetPitchOnTransposedNote) {
    // Piccolo: written C4 sounds an octave higher.
    const Note n("C4", RhythmFigure::QUARTER, true, false, 7, 12);
    ASSERT_EQ(n.getPitch(), "C5");
    EXPECT_EQ(n.getEnharmonicPitch(false), "Dbb5");
    EXPECT_EQ(n.getEnharmonicPitch(true), "B#4");

    // Scope of this check: it pins THIS case's respellings against THIS note's own MIDI number,
    // comparing two live calls rather than a hard-coded literal, so a future legitimate change
    // of spelling moves both sides together and a genuine regression is still caught.
    //
    // The test below checks the same for every transposing instrument.
    EXPECT_EQ(Note(n.getEnharmonicPitch(false)).getMidiNumber(), n.getMidiNumber());
    EXPECT_EQ(Note(n.getEnharmonicPitch(true)).getMidiNumber(), n.getMidiNumber());
}

// An enharmonic respelling must describe the same pitch as the note it was spelled from, for
// EVERY transposing instrument: getEnharmonicPitch() re-derives from getPitch(), so a wrongly
// spelled sounding pitch class would move the respelling to another pitch.
TEST(NoteComposesPitch, GetEnharmonicPitchDescribesTheSamePitchForEveryTransposingInstrument) {
    const std::vector<std::pair<std::string, std::pair<int, int>>> instruments = {
        {"C#4", {-1, -2}},  // B-flat clarinet
        {"F#4", {-4, -7}},  // horn in F
        {"C4", {7, 12}}};   // piccolo

    for (const auto& instrument : instruments) {
        const Note n("" + instrument.first, RhythmFigure::QUARTER, /*isNoteOn=*/true,
                     /*inChord=*/false, instrument.second.first, instrument.second.second);
        EXPECT_EQ(Note(n.getEnharmonicPitch(false)).getMidiNumber(), n.getMidiNumber())
            << "written pitch: " << instrument.first;
        EXPECT_EQ(Note(n.getEnharmonicPitch(true)).getMidiNumber(), n.getMidiNumber())
            << "written pitch: " << instrument.first;
    }
}

// getOctave() is the sounding octave: one BELOW the written octave for the clarinet and the horn,
// one ABOVE for the piccolo, and in each case it agrees with getSoundingOctave(), which is
// arithmetic:
//
//   case                         getOctave()
//   B-flat clarinet  C#4 -1/-2       3
//   horn in F        F#4 -4/-7       3
//   piccolo          C4  +7/+12      5
//
// What this test guards is CONSISTENCY ACROSS THE MUTATION PATH: setOctave() called with the
// note's OWN current written octave (4) is a no-op value, so getOctave() must answer the same thing
// before and after it. A transposed note must never report one octave when constructed and
// another once that same octave is set again.
TEST(NoteComposesPitch, GetOctaveIsUnchangedByASetOctaveNoOpOnATransposedNote) {
    // B-flat clarinet: written C#4 sounds a major second lower.
    {
        Note n("C#4", RhythmFigure::QUARTER, true, false, -1, -2);
        ASSERT_EQ(n.getOctave(), 3);  // construction path
        n.setOctave(4);               // no-op value: the note's own current WRITTEN octave
        EXPECT_EQ(n.getOctave(), 3);
        EXPECT_EQ(n.getSoundingOctave(), 3);  // arithmetic; agrees with getOctave()
        EXPECT_EQ(n.getMidiNumber(), 59);
        EXPECT_EQ(n.getPitch(), "B3");
    }

    // Horn in F: written F#4 sounds a perfect fifth lower.
    {
        Note n("F#4", RhythmFigure::QUARTER, true, false, -4, -7);
        ASSERT_EQ(n.getOctave(), 3);
        n.setOctave(4);
        EXPECT_EQ(n.getOctave(), 3);
        EXPECT_EQ(n.getSoundingOctave(), 3);
        EXPECT_EQ(n.getMidiNumber(), 59);
        EXPECT_EQ(n.getPitch(), "B3");
    }

    // Piccolo: written C4 sounds an octave higher.
    {
        Note n("C4", RhythmFigure::QUARTER, true, false, 7, 12);
        ASSERT_EQ(n.getOctave(), 5);
        n.setOctave(4);
        EXPECT_EQ(n.getOctave(), 5);
        EXPECT_EQ(n.getSoundingOctave(), 5);
        EXPECT_EQ(n.getMidiNumber(), 72);
        EXPECT_EQ(n.getPitch(), "C5");
    }
}

// A quarter-tone written pitch on a transposing instrument sounds like any other: the sounding
// position is the written position plus the chromatic interval, quarter tone and all.
TEST(NoteComposesPitch, QuarterTonePitchOnATransposingInstrumentSoundsInsteadOfThrowing) {
    // B-flat clarinet: written C1x4 (exactly 60.5) sounds a major second lower, 58.5.
    const Note n("C1x4", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                 /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    EXPECT_TRUE(n.isQuarterTone());
    EXPECT_EQ(n.getSoundingPitch(), "B1b3");
    EXPECT_EQ(n.getSoundingPitchClass(), "B1b");
    EXPECT_EQ(n.getAlterSymbol(), "1b");
    EXPECT_EQ(n.getOctave(), 3);
    // getMidiNumber() still rounds ties upward: 58.5 -> 59. The spelling above keeps the exact
    // value the MIDI number rounds away.
    EXPECT_EQ(n.getMidiNumber(), 59);
}

// setIsNoteOn(false) turns a note into a rest but deliberately does not clear its transposing
// intervals, so isTransposed() stays true. getSoundingPitch() answers "rest" for it through a rest
// guard at its top, ahead of the transposition branch, which would otherwise compose the pitch
// CLASS ("rest") with an octave into a malformed string such as "rest-2", not a valid pitch and
// rejected by every Note constructor. The sibling getters are covered too:
// computeSoundingPitch() returns early for a rest, toXML() emits its pitch block only under
// isNoteOn(), and getWrittenPitch() delegates to Pitch::getPitch(), which guards rest itself.
//
// "rest" is the correct answer, and what the untransposed case and setPitch("rest") return. It is
// not the note's former sounding pitch ("Bb3" for the clarinet, "C5" for the piccolo): a rest has
// none.
TEST(NoteComposesPitch, GetPitchIsWellFormedRestForTransposedNoteTurnedOff) {
    // B-flat clarinet, horn in F and piccolo.
    const std::vector<std::pair<std::string, std::pair<int, int>>> transposed = {
        {"C#4", {-1, -2}}, {"F#4", {-4, -7}}, {"C4", {7, 12}}};

    for (const auto& c : transposed) {
        Note n(c.first, RhythmFigure::QUARTER, true, false, c.second.first, c.second.second);
        ASSERT_TRUE(n.isNoteOn());
        n.setIsNoteOn(false);

        // The transposing intervals survive, which is precisely why the guard is needed.
        EXPECT_TRUE(n.isTransposed()) << "pitch: " << c.first;
        EXPECT_TRUE(n.isNoteOff()) << "pitch: " << c.first;
        EXPECT_EQ(n.getPitch(), "rest") << "pitch: " << c.first;
        EXPECT_EQ(n.getSoundingPitch(), "rest") << "pitch: " << c.first;
        EXPECT_EQ(n.getWrittenPitch(), "rest") << "pitch: " << c.first;
        EXPECT_EQ(n.getMidiNumber(), -1) << "pitch: " << c.first;
    }

    // Untransposed control.
    Note control("C4");
    control.setIsNoteOn(false);
    EXPECT_FALSE(control.isTransposed());
    EXPECT_EQ(control.getPitch(), "rest");
}

// A written "C#-1" on a B-flat clarinet (transposeDiatonic=-1, transposeChromatic=-2) sounds at
// -1, below the lowest representable pitch C1b-1 (-0.5). The note is constructible and is a note,
// not a rest; every sounding getter fails loudly with the same diagnosable error, instead of a
// rest's values or an unexplained std::bad_optional_access.
TEST(NoteComposesPitch, GetPitchBelowMidiZeroFailsDiagnosablyNotWithBadOptionalAccess) {
    const Note n("C#-1", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                 /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    ASSERT_TRUE(n.isNoteOn());

    const std::string message = thrownFirstLine([&] { n.getPitch(); });
    EXPECT_NE(message.find("below the lowest representable pitch C1b-1"), std::string::npos)
        << "message: " << message;
    EXPECT_NE(message.find("'C#-1'"), std::string::npos) << "message: " << message;
    EXPECT_NE(message.find("transposeChromatic=-2"), std::string::npos) << "message: " << message;
    EXPECT_EQ(message.find("ptional access"), std::string::npos) << "message: " << message;
}

// isNoteOff() stays false: the note is real, its sounding octave simply does not exist, and asking
// for it fails instead of answering a rest's empty optional.
TEST(NoteComposesPitch, GetSoundingOctaveOfANoteBelowTheFloorThrowsInsteadOfAnsweringARest) {
    const Note n("C#-1", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                 /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    EXPECT_TRUE(n.isNoteOn());
    EXPECT_FALSE(n.isNoteOff());
    const std::string message = thrownFirstLine([&] { n.getSoundingOctave(); });
    EXPECT_NE(message.find("below the lowest representable pitch C1b-1"), std::string::npos)
        << message;
}

// A written "C1x-1" (0.5) on an instrument sounding a semitone lower sounds exactly -0.5, which
// rounds, ties upward, to MIDI 0 and is "C1b-1": a real pitch, not one below MIDI 0. Its sounding
// pitch class, octave and MIDI number must all describe it; a sounding pitch class of "rest"
// beside MIDI 0 would compose the malformed string "rest-1".
TEST(NoteComposesPitch, SoundingPitchOnTheLowestQuarterToneIsWellFormedNotRestMinus1) {
    const Note n("C1x-1", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                 /*transposeDiatonic=*/-1, /*transposeChromatic=*/-1);
    EXPECT_EQ(n.getMidiNumber(), 0);
    EXPECT_EQ(n.getSoundingPitch(), "C1b-1");  // not "rest-1"
    EXPECT_EQ(n.getSoundingPitchClass(), "C1b");
    EXPECT_EQ(n.getOctave().value_or(-99), -1);  // agrees with the line below
    EXPECT_EQ(n.getSoundingOctave().value_or(-99), -1);
}

// A rest given an out-of-range octave warns and does not mutate, while a genuine out-of-range
// octave on a real (non-rest) pitch still throws. Two properties make that so:
//   1. getOctave() on a rest answers an empty std::optional<int>, not a numeric sentinel, so
//      there is no out-of-range value to write straight back into setOctave(int).
//   2. Pitch::setOctave() runs its refuse-on-rest check BEFORE its throwing range check: an
//      operation that is inapplicable to a rest is inapplicable whatever the argument, so a rest
//      handed ANY out-of-range octave warns.
// The in-range case on a rest is SetOctaveOnRestRefusesAndWarns; an out-of-range octave on a real
// pitch is also Pitch.setOctaveRejectsOutOfRange.
TEST(NoteComposesPitch, SetOctaveOutOfRangeOnRestWarnsAndDoesNotThrow) {
    Note n("rest");
    ASSERT_FALSE(n.getOctave().has_value());  // no sentinel to round-trip

    // -2, the nearest out-of-range octave below, handed directly to setOctave(): must warn, not
    // throw.
    EXPECT_NO_THROW(n.setOctave(-2));
    EXPECT_TRUE(n.isNoteOff());  // nothing was mutated by the refused call
    EXPECT_FALSE(n.getOctave().has_value());

    // A different out-of-range value confirms this is the rest check firing first, not a
    // value-specific carve-out for -2.
    EXPECT_NO_THROW(n.setOctave(12));
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_FALSE(n.getOctave().has_value());

    // Control 1: an in-range octave on the same rest also refuses with a warning
    // (SetOctaveOnRestRefusesAndWarns's policy) -- rest-ness alone is sufficient to refuse,
    // regardless of range.
    EXPECT_NO_THROW(n.setOctave(5));
    EXPECT_FALSE(n.getOctave().has_value());

    // Control 2: the range check is still alive and still throws for a genuine out-of-range
    // octave on a real (non-rest) pitch: the rest check runs first, but neither check is removed
    // or weakened.
    Note sounding("C4");
    EXPECT_THROW(sounding.setOctave(-2), std::runtime_error);
    EXPECT_EQ(sounding.getOctave(), 4);
    EXPECT_THROW(sounding.setOctave(12), std::runtime_error);
    EXPECT_EQ(sounding.getOctave(), 4);
}

// =====================================================================================
// MUSICXML WRITE -- ALTER AND ACCIDENTAL
// =====================================================================================

// Note::toXML() writes a quarter tone's alter with its decimal part and adds the matching Tartini
// <accidental> name, so the quarter tone survives the write: an alter truncated to an int would
// write <alter>0</alter>, a natural.
TEST(NoteToXML, QuarterSharpWritesFractionalAlterAndTartiniAccidental) {
    const std::string xml = Note("C1x4").toXML();
    EXPECT_NE(xml.find("<alter>0.5</alter>"), std::string::npos) << xml;
    EXPECT_NE(xml.find("<accidental>quarter-sharp</accidental>"), std::string::npos) << xml;
}

// All four quarter-tone symbols, both directions (flat/sharp) and both quarter-tone depths
// (1x/3x), so the Tartini mapping is exercised through the live write path for each one
// individually, not just for "C1x4".
TEST(NoteToXML, AllFourQuarterTonesWriteMatchingAlterAndAccidental) {
    struct Case {
        std::string pitch;
        std::string alterStr;
        std::string accidentalName;
    };
    const std::vector<Case> cases = {
        {"C1x4", "0.5", "quarter-sharp"},
        {"C3x4", "1.5", "three-quarters-sharp"},
        {"D1b4", "-0.5", "quarter-flat"},
        {"D3b4", "-1.5", "three-quarters-flat"},
    };

    for (const auto& c : cases) {
        const std::string xml = Note(c.pitch).toXML();
        EXPECT_NE(xml.find("<alter>" + c.alterStr + "</alter>"), std::string::npos)
            << "pitch: " << c.pitch << " xml: " << xml;
        EXPECT_NE(xml.find("<accidental>" + c.accidentalName + "</accidental>"), std::string::npos)
            << "pitch: " << c.pitch << " xml: " << xml;
    }
}

// Integer alters must NOT gain the decimal formatting a naive fractional formatter would apply
// universally (e.g. "1.0"/"-2.0"), nor std::to_string(float)'s six decimals ("1.000000"): the
// first re-export of any existing score would then produce a false diff on every integer alter in
// the file. Checked with genuine accidentals (sharp, double-flat) so <alter> is actually emitted
// -- a natural has no accidental symbol and emits no <alter> at all (see
// NoAccidentalPitchWritesNeitherAlterNorAccidental below).
//
// <accidental> is ABSENT for these two notes: a semitone accidental can be implied entirely by the
// key signature (e.g. an F# in D major needs only <alter>1</alter>, no glyph), so <accidental> is
// reserved for alters a key signature cannot express -- quarter tones only (see the note.cpp
// comment at the <accidental> guard).
TEST(NoteToXML, IntegerAlterWritesWithoutDecimalPart) {
    const std::string sharp = Note("C#4").toXML();
    EXPECT_NE(sharp.find("<alter>1</alter>"), std::string::npos) << sharp;
    EXPECT_EQ(sharp.find("<alter>1.0"), std::string::npos) << sharp;
    EXPECT_EQ(sharp.find("<accidental>"), std::string::npos) << sharp;

    const std::string doubleFlat = Note("Cbb4").toXML();
    EXPECT_NE(doubleFlat.find("<alter>-2</alter>"), std::string::npos) << doubleFlat;
    EXPECT_EQ(doubleFlat.find("<alter>-2.0"), std::string::npos) << doubleFlat;
    EXPECT_EQ(doubleFlat.find("<accidental>"), std::string::npos) << doubleFlat;
}

// A natural pitch carries no accidental symbol (Pitch::getAlterSymbol() == ""), and its alter
// (0.0) is not fractional, so it emits neither <alter> nor <accidental>. Pinned so that a change
// to either guard condition cannot silently start writing a spurious natural.
TEST(NoteToXML, NoAccidentalPitchWritesNeitherAlterNorAccidental) {
    const std::string xml = Note("C4").toXML();
    EXPECT_EQ(xml.find("<alter>"), std::string::npos) << xml;
    EXPECT_EQ(xml.find("<accidental>"), std::string::npos) << xml;
}

// The unpitched (percussion) branch is a separate code path in Note::toXML() from the pitched
// one, so its <alter>/<accidental> output is checked on its own: the same decimal alter and
// Tartini accidental.
TEST(NoteToXML, UnpitchedBranchWritesFractionalAlterAndAccidental) {
    Note note("C1x4");
    note.setIsPitched(false);
    const std::string xml = note.toXML();

    EXPECT_NE(xml.find("<display-step>C</display-step>"), std::string::npos) << xml;
    EXPECT_NE(xml.find("<alter>0.5</alter>"), std::string::npos) << xml;
    EXPECT_NE(xml.find("<accidental>quarter-sharp</accidental>"), std::string::npos) << xml;
    EXPECT_NE(xml.find("<display-octave>4</display-octave>"), std::string::npos) << xml;
}

// <accidental> lands immediately after <type> and before <time-modification>. The library emits
// <time-modification> before <dot>, although the MusicXML schema wants
// `type?, dot*, accidental?, time-modification?, stem?`: a known inversion of those two elements.
// This test pins where <accidental> sits relative to them, not that their relative order is
// correct.
TEST(NoteToXML, AccidentalPositionedAfterTypeAndBeforeTimeModification) {
    Note note("C1x4");
    note.setDuration(1.5f);  // adds one augmentation dot
    note.setIsTuplet(true);
    note.setTupleValues(3, 2, "eighth");

    const std::string xml = note.toXML();

    const auto typePos = xml.find("<type>");
    const auto accidentalPos = xml.find("<accidental>");
    const auto timeModPos = xml.find("<time-modification>");
    const auto dotPos = xml.find("<dot");

    ASSERT_NE(typePos, std::string::npos) << xml;
    ASSERT_NE(accidentalPos, std::string::npos) << xml;
    ASSERT_NE(timeModPos, std::string::npos) << xml;
    ASSERT_NE(dotPos, std::string::npos) << xml;

    EXPECT_LT(typePos, accidentalPos) << xml;
    EXPECT_LT(accidentalPos, timeModPos) << xml;
    // The known inversion: <time-modification> precedes <dot> here, though the schema orders
    // dot* before time-modification?. Recorded, not corrected.
    EXPECT_LT(timeModPos, dotPos) << xml;
}

// ===================================================================================================
// QUARTER-TONE PREDICATE AND ROUNDING ON Note
//
// Thin delegations to the Pitch behaviour, so that the Chord and Interval analysis guards, and
// Chord::roundQuarterTones(), can ask a Note about its accidental without re-implementing the
// rounding rule (or re-parsing the pitch string) at each site.
// ===================================================================================================

TEST(NoteIsQuarterTone, TrueOnlyForFractionalAlters) {
    EXPECT_TRUE(Note("C1x4").isQuarterTone());
    EXPECT_TRUE(Note("C3x4").isQuarterTone());
    EXPECT_TRUE(Note("D1b4").isQuarterTone());
    EXPECT_TRUE(Note("D3b4").isQuarterTone());

    EXPECT_FALSE(Note("C4").isQuarterTone());
    EXPECT_FALSE(Note("C#4").isQuarterTone());
    EXPECT_FALSE(Note("Cb4").isQuarterTone());
    EXPECT_FALSE(Note("Cx4").isQuarterTone());
    EXPECT_FALSE(Note("Cbb4").isQuarterTone());

    // A rest has no accidental: its alter is 0, so it is not a quarter tone.
    EXPECT_FALSE(Note("rest").isQuarterTone());
}

TEST(NoteRoundToSemitone, RoundsTiesUpwardAndLeavesWholeTonesAlone) {
    Note sharpSide("C1x4");
    sharpSide.roundToSemitone();
    EXPECT_EQ(sharpSide.getPitch(), "C#4");
    EXPECT_FALSE(sharpSide.isQuarterTone());

    // The flat side is what discriminates ties-upward from ties-away-from-zero:
    // floor(-0.5 + 0.5) == 0, so D1b4 rounds UP to D4; std::round(-0.5) would give Db4.
    Note flatSide("D1b4");
    flatSide.roundToSemitone();
    EXPECT_EQ(flatSide.getPitch(), "D4");

    Note lowerFlatSide("D3b4");
    lowerFlatSide.roundToSemitone();
    EXPECT_EQ(lowerFlatSide.getPitch(), "Db4");

    // Safe to call unconditionally: a whole-tone accidental is left exactly as it was.
    Note wholeTone("F#4");
    wholeTone.roundToSemitone();
    EXPECT_EQ(wholeTone.getPitch(), "F#4");
}

// ===== getQuarterToneSteps(): the exact, unrounded sounding position ===== //

TEST(NoteQuarterToneSteps, isTheExactPositionOfTheSoundingPitch) {
    EXPECT_EQ(Note("C1x4").getQuarterToneSteps(), 60.5f);
    EXPECT_EQ(Note("E1b4").getQuarterToneSteps(), 63.5f);
    EXPECT_EQ(Note("C4").getQuarterToneSteps(), 60.0f);
    EXPECT_EQ(Note("rest").getQuarterToneSteps(), -1.0f);

    // Sounding, not written: a B-flat clarinet's written C1x4 sounds a whole tone lower.
    const Note clarinet("C1x4", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                        /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    EXPECT_EQ(clarinet.getQuarterToneSteps(), 58.5f);
    EXPECT_EQ(clarinet.getMidiNumber(), 59);
}

// The ordering operators compare these positions: E1b4 (63.5) sorts below E4 (64), which its
// rounded MIDI number (64) cannot tell apart.
TEST(NoteQuarterToneSteps, orderingComparesExactPositions) {
    EXPECT_TRUE(Note("E1b4") < Note("E4"));
    EXPECT_FALSE(Note("E4") < Note("E1b4"));
    EXPECT_TRUE(Note("E4") > Note("E1b4"));
    EXPECT_FALSE(Note("E1b4") >= Note("E4"));
    EXPECT_TRUE(Note("E1b4") <= Note("D3x4"));  // the same position, 63.5
    EXPECT_TRUE(Note("E1b4") >= Note("D3x4"));
}

TEST(NoteQuarterToneSteps, setAlterRejectsAValueNearTheGrid) {
    Note note("C4");
    const std::string message = thrownFirstLine([&] { note.setAlter(0.99996f); });
    EXPECT_NE(message.find("multiple of 0.5"), std::string::npos) << message;
    EXPECT_EQ(note.getPitch(), "C4");
    EXPECT_FALSE(note.isQuarterTone());
}

// Negating a natural's alter gives -0.0; the note must still spell, and write, as a natural.
TEST(NoteQuarterToneSteps, setAlterOfNegativeZeroLeavesAPlainNatural) {
    Note note("C#4");
    note.setAlter(-0.0f);
    EXPECT_EQ(note.getPitch(), "C4");
    EXPECT_EQ(note.toXML().find("<alter>"), std::string::npos);
}

// ===== A sounding pitch below the lowest representable pitch ===== //

namespace {
// Each sounding getter of a note, by name, as a call that discards its result.
std::vector<std::pair<std::string, std::function<void(const Note&)>>> soundingGetters() {
    return {
        {"getPitch", [](const Note& n) { n.getPitch(); }},
        {"getSoundingPitch", [](const Note& n) { n.getSoundingPitch(); }},
        {"getPitchClass", [](const Note& n) { n.getPitchClass(); }},
        {"getSoundingPitchClass", [](const Note& n) { n.getSoundingPitchClass(); }},
        {"getPitchStep", [](const Note& n) { n.getPitchStep(); }},
        {"getSoundingPitchStep", [](const Note& n) { n.getSoundingPitchStep(); }},
        {"getDiatonicSoundingPitchClass", [](const Note& n) { n.getDiatonicSoundingPitchClass(); }},
        {"getAlterSymbol", [](const Note& n) { n.getAlterSymbol(); }},
        {"getOctave", [](const Note& n) { n.getOctave(); }},
        {"getSoundingOctave", [](const Note& n) { n.getSoundingOctave(); }},
        {"getMidiNumber", [](const Note& n) { n.getMidiNumber(); }},
        {"getQuarterToneSteps", [](const Note& n) { n.getQuarterToneSteps(); }},
        {"getFrequency", [](const Note& n) { n.getFrequency(); }},
        {"getEnharmonicPitch", [](const Note& n) { n.getEnharmonicPitch(); }},
        {"getEnharmonicPitches", [](const Note& n) { n.getEnharmonicPitches(); }},
    };
}
}  // namespace

// Semitone and quarter-tone written pitches whose sounding position falls below C1b-1 (-0.5).
TEST(NoteSoundingPitchBelowFloor, everySoundingGetterFailsTheSameWay) {
    for (const auto& [written, chromatic] :
         std::vector<std::pair<std::string, int>>{{"C#-1", -2}, {"C1b-1", -1}, {"C1x-1", -2}}) {
        const Note n(written, RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                     /*transposeDiatonic=*/-1, chromatic);
        for (const auto& [name, getter] : soundingGetters()) {
            // A C++17 lambda cannot capture a structured binding, hence the init-capture.
            const std::string message = thrownFirstLine([&n, &getter = getter] { getter(n); });
            EXPECT_NE(message.find("below the lowest representable pitch C1b-1"), std::string::npos)
                << written << " " << name << ": " << message;
            EXPECT_NE(message.find("'" + written + "'"), std::string::npos)
                << written << " " << name << ": " << message;
        }
    }
}

// It is a note, not a rest, and everything about its written pitch still answers.
TEST(NoteSoundingPitchBelowFloor, theWrittenPitchStillAnswers) {
    const Note n("C#-1", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                 /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    EXPECT_TRUE(n.isNoteOn());
    EXPECT_FALSE(n.isNoteOff());
    EXPECT_EQ(n.getWrittenPitch(), "C#-1");
    EXPECT_EQ(n.getWrittenPitchClass(), "C#");
    EXPECT_EQ(n.getWrittenOctave().value_or(-99), -1);
    EXPECT_FALSE(n.isQuarterTone());
    EXPECT_NE(n.toXML().find("<step>C</step>"), std::string::npos);
}

// Setting such an interval on an existing note is accepted too: the eager evaluation in
// setTransposingInterval() is skipped for it, and the condition is reported when asked.
TEST(NoteSoundingPitchBelowFloor, setTransposingIntervalKeepsTheNoteConstructible) {
    Note n("C#-1");
    n.setTransposingInterval(-1, -2);
    EXPECT_EQ(n.getTransposeChromatic(), -2);
    const std::string message = thrownFirstLine([&] { n.getMidiNumber(); });
    EXPECT_NE(message.find("below the lowest representable pitch C1b-1"), std::string::npos)
        << message;
}

// An Interval with such a note is refused, with the same error, when it is built -- before its
// octave arithmetic, where an empty optional would raise a bare std::bad_optional_access.
TEST(NoteSoundingPitchBelowFloor, anIntervalWithSuchANoteFailsDiagnosably) {
    const Note n("C#-1", RhythmFigure::QUARTER, /*isNoteOn=*/true, /*inChord=*/false,
                 /*transposeDiatonic=*/-1, /*transposeChromatic=*/-2);
    const std::string message = thrownFirstLine([&] { Interval(n, Note("C4")).getNumOctaves(); });
    EXPECT_NE(message.find("below the lowest representable pitch C1b-1"), std::string::npos)
        << message;
}

// ===== A mutator that throws leaves the note exactly as it was ===== //

namespace {
// Everything a Note mutator can change, as one comparable snapshot: the written pitch, the
// transposing interval, the sounding pitch (or the first line of the error a sounding getter
// raises), the duration, and the MusicXML the note writes, which also carries its tuplet values.
struct NoteState {
    std::string writtenPitch;
    int transposeDiatonic;
    int transposeChromatic;
    std::string soundingPitch;
    int durationTicks;
    std::string type;
    std::string xml;

    bool operator==(const NoteState& other) const {
        return std::tie(writtenPitch, transposeDiatonic, transposeChromatic, soundingPitch,
                        durationTicks, type, xml) ==
               std::tie(other.writtenPitch, other.transposeDiatonic, other.transposeChromatic,
                        other.soundingPitch, other.durationTicks, other.type, other.xml);
    }
};

std::ostream& operator<<(std::ostream& stream, const NoteState& state) {
    return stream << "{written " << state.writtenPitch << ", interval (" << state.transposeDiatonic
                  << ", " << state.transposeChromatic << "), sounding " << state.soundingPitch
                  << ", ticks " << state.durationTicks << ", type " << state.type << "}";
}

NoteState stateOf(const Note& note) {
    std::string soundingPitch;
    try {
        soundingPitch = note.getPitch();
    } catch (const std::runtime_error& error) {
        const std::string what = error.what();
        soundingPitch = "raises: " + what.substr(0, what.find('\n'));
    }
    return {note.getWrittenPitch(),
            note.getTransposeDiatonic(),
            note.getTransposeChromatic(),
            soundingPitch,
            note.getDurationTicks(),
            note.getLongType(),
            note.toXML()};
}

// The error for 'writtenPitch' on a (diatonic, chromatic) instrument, whose sounding pitch lies at
// 'position' (as std::to_string() writes it), above B11, the highest sounding pitch that can be
// spelled within octaves -1..11.
std::string aboveTheCeiling(const std::string& writtenPitch, const int diatonic,
                            const int chromatic, const std::string& position) {
    return "[maiacore] The sounding pitch of the written pitch '" + writtenPitch +
           "' with transposeDiatonic=" + std::to_string(diatonic) +
           " and transposeChromatic=" + std::to_string(chromatic) + " is at position " + position +
           ", above B11 (MIDI note 155), the highest sounding pitch that can be spelled within "
           "octaves -1..11, so it has no sounding spelling. A lower written pitch or a smaller "
           "transposing interval keeps the sounding pitch at or below B11.";
}
}  // namespace

// The interval is checked before it is stored: one whose sounding pitch cannot be spelled throws
// without leaving the note holding it, and every sounding getter still answers.
TEST(NoteMutatorThatThrows, setTransposingIntervalLeavesTheNoteUnchanged) {
    Note note("B11");
    const NoteState before = stateOf(note);
    EXPECT_EQ(thrownFirstLine([&] { note.setTransposingInterval(1, 3); }),
              aboveTheCeiling("B11", 1, 3, "158.000000"));
    EXPECT_EQ(stateOf(note), before);
    EXPECT_EQ(note.getPitch(), "B11");

    Note clarinet = transposingNote("C4", -1, -2);
    const NoteState clarinetBefore = stateOf(clarinet);
    EXPECT_EQ(thrownFirstLine([&] { clarinet.setTransposingInterval(0, 200); }),
              aboveTheCeiling("C4", 0, 200, "260.000000"));
    EXPECT_EQ(stateOf(clarinet), clarinetBefore);
    EXPECT_EQ(clarinet.getTransposeChromatic(), -2);
    EXPECT_EQ(clarinet.getPitch(), "Bb3");
}

// An interval at the limits of int cannot wrap the sounding pitch around to the other end of the
// representable range: the largest upward interval is rejected as above B11 before it is stored,
// and the largest downward one leaves a constructible note whose sounding pitch lies below C1b-1.
TEST(NoteMutatorThatThrows, anIntervalAtTheLimitsOfIntIsCheckedOnItsOwnSide) {
    const int up = std::numeric_limits<int>::max();
    EXPECT_EQ(thrownFirstLine([&] { transposingNote("C4", 1, up); }),
              aboveTheCeiling("C4", 1, up, "2147483648.000000"));

    Note note("C4");
    EXPECT_EQ(thrownFirstLine([&] { note.setTransposingInterval(1, up); }),
              aboveTheCeiling("C4", 1, up, "2147483648.000000"));
    EXPECT_FALSE(note.isTransposed());

    const Note low = transposingNote("C4", -1, std::numeric_limits<int>::min());
    const std::string message = thrownFirstLine([&] { low.getMidiNumber(); });
    EXPECT_NE(message.find("below the lowest representable pitch C1b-1"), std::string::npos)
        << message;
}

// A rest has no pitch to transpose, so it ignores the call and keeps no interval.
TEST(NoteMutatorThatThrows, setTransposingIntervalOnARestIsIgnored) {
    Note rest("rest");
    rest.setTransposingInterval(1, 3);
    EXPECT_FALSE(rest.isTransposed());
    EXPECT_EQ(rest.getTransposeChromatic(), 0);
}

// The new written pitch is checked with the current interval before it is stored. A sounding
// pitch below the lowest representable pitch is still accepted, as setTransposingInterval()
// accepts it: the note stays constructible.
TEST(NoteMutatorThatThrows, setPitchLeavesTheNoteUnchanged) {
    Note note = transposingNote("C4", 1, 3);
    const NoteState before = stateOf(note);
    EXPECT_EQ(thrownFirstLine([&] { note.setPitch("B11"); }),
              aboveTheCeiling("B11", 1, 3, "158.000000"));
    EXPECT_EQ(stateOf(note), before);
    EXPECT_EQ(note.getPitch(), "D#4");

    Note clarinet = transposingNote("C4", -1, -2);
    clarinet.setPitch("C#-1");
    EXPECT_EQ(clarinet.getWrittenPitch(), "C#-1");
    const std::string message = thrownFirstLine([&] { clarinet.getPitch(); });
    EXPECT_NE(message.find("below the lowest representable pitch C1b-1"), std::string::npos)
        << message;
}

// transpose() stores its result through setPitch(), which on a transposing instrument moves it by
// the interval again: A11 plus 3 semitones sounds at 156, above B11.
TEST(NoteMutatorThatThrows, transposeLeavesTheNoteUnchanged) {
    Note note = transposingNote("C4", 1, 3);
    const NoteState before = stateOf(note);
    EXPECT_EQ(thrownFirstLine([&] { note.transpose(90); }),
              aboveTheCeiling("A11", 1, 3, "156.000000"));
    EXPECT_EQ(stateOf(note), before);
    EXPECT_EQ(note.getPitch(), "D#4");
}

// toEnharmonicPitch() stores the respelling of the sounding B11 through setPitch(), where it is
// moved by the interval again: B11 (the default, which has no flat partner within octave 11) or
// Ax11 (the alternative) plus 2 semitones sounds at 157, above B11.
TEST(NoteMutatorThatThrows, toEnharmonicPitchLeavesTheNoteUnchanged) {
    for (const bool alternative : {false, true}) {
        Note note = transposingNote("A11", 1, 2);
        const NoteState before = stateOf(note);
        EXPECT_EQ(thrownFirstLine([&] { note.toEnharmonicPitch(alternative); }),
                  aboveTheCeiling(alternative ? "Ax11" : "B11", 1, 2, "157.000000"))
            << "alternative " << alternative;
        EXPECT_EQ(stateOf(note), before) << "alternative " << alternative;
        EXPECT_EQ(note.getPitch(), "B11");
    }
}

// A duration that cannot be converted to a rhythm figure throws without storing its tick count,
// through either numeric overload.
TEST(NoteMutatorThatThrows, setDurationLeavesTheNoteUnchanged) {
    for (const float quarterDuration : {0.0f, -1.0f}) {
        Note note = transposingNote("C4", -1, -2);
        const NoteState before = stateOf(note);
        EXPECT_EQ(thrownFirstLine([&] { note.setDuration(quarterDuration); }),
                  "[maiacore] Unable to convert durationTick to RhythmFigure")
            << quarterDuration;
        EXPECT_EQ(stateOf(note), before) << quarterDuration;
        EXPECT_EQ(note.getDurationTicks(), 256);
    }

    Note note("C4");
    const NoteState before = stateOf(note);
    EXPECT_EQ(thrownFirstLine([&] { note.setDuration(0, 256); }),
              "[maiacore] Unable to convert durationTick to RhythmFigure");
    EXPECT_EQ(stateOf(note), before);
}

// An unknown note type throws without storing the new note counts.
TEST(NoteMutatorThatThrows, setTupleValuesLeavesTheNoteUnchanged) {
    Note note("C4");
    note.setIsTuplet(true);
    note.setTupleValues(3, 2, "eighth");
    const NoteState before = stateOf(note);
    EXPECT_EQ(thrownFirstLine([&] { note.setTupleValues(5, 4, "garbage"); }),
              "[maiacore] Unknown note type: garbage");
    EXPECT_EQ(stateOf(note), before);
    EXPECT_EQ(note.getDuration().getTimeModificationActualNotes(), 3);
    EXPECT_EQ(note.getDuration().getTimeModificationNormalNotes(), 2);
}

// ===== The partial pitch setters check the sounding pitch as setPitch() does ===== //

namespace {
// The first line of what setPitch(pitch) throws on a copy of 'note': the error a partial setter
// giving the same written pitch must raise.
std::string setPitchRejection(const Note& note, const std::string& pitch) {
    Note copy = note;
    return thrownFirstLine([&] { copy.setPitch(pitch); });
}

// A note whose sounding pitch is below the lowest representable pitch: every sounding getter
// raises, naming the condition.
void expectSoundingBelowTheFloor(const Note& note) {
    const std::string message = thrownFirstLine([&] { note.getPitch(); });
    EXPECT_NE(message.find("below the lowest representable pitch C1b-1"), std::string::npos)
        << message;
}
}  // namespace

// C11 on a (1, 3) instrument sounds D#11; as A11 it would sound at 156, above B11.
TEST(NoteMutatorThatThrows, setStepChecksTheSoundingPitchLikeSetPitch) {
    Note note = transposingNote("C11", 1, 3);
    const NoteState before = stateOf(note);
    const std::string message = thrownFirstLine([&] { note.setStep("A"); });
    EXPECT_EQ(message, aboveTheCeiling("A11", 1, 3, "156.000000"));
    EXPECT_EQ(message, setPitchRejection(note, "A11"));
    EXPECT_EQ(stateOf(note), before);
    EXPECT_EQ(note.getPitch(), "D#11");
}

TEST(NoteMutatorThatThrows, setPitchClassChecksTheSoundingPitchLikeSetPitch) {
    Note note = transposingNote("C11", 1, 3);
    const NoteState before = stateOf(note);
    const std::string message = thrownFirstLine([&] { note.setPitchClass("A"); });
    EXPECT_EQ(message, aboveTheCeiling("A11", 1, 3, "156.000000"));
    EXPECT_EQ(message, setPitchRejection(note, "A11"));
    EXPECT_EQ(stateOf(note), before);
    EXPECT_EQ(note.getPitch(), "D#11");
}

// B4 on a (1, 3) instrument sounds D5; as B11 it would sound at 158, above B11.
TEST(NoteMutatorThatThrows, setOctaveChecksTheSoundingPitchLikeSetPitch) {
    Note note = transposingNote("B4", 1, 3);
    const NoteState before = stateOf(note);
    const std::string message = thrownFirstLine([&] { note.setOctave(11); });
    EXPECT_EQ(message, aboveTheCeiling("B11", 1, 3, "158.000000"));
    EXPECT_EQ(message, setPitchRejection(note, "B11"));
    EXPECT_EQ(stateOf(note), before);
    EXPECT_EQ(note.getPitch(), "D5");
}

// A11 on a (1, 2) instrument sounds B11; as A#11 it would sound at 156, above B11.
TEST(NoteMutatorThatThrows, setAlterChecksTheSoundingPitchLikeSetPitch) {
    Note note = transposingNote("A11", 1, 2);
    const NoteState before = stateOf(note);
    const std::string message = thrownFirstLine([&] { note.setAlter(1.0f); });
    EXPECT_EQ(message, aboveTheCeiling("A#11", 1, 2, "156.000000"));
    EXPECT_EQ(message, setPitchRejection(note, "A#11"));
    EXPECT_EQ(stateOf(note), before);
    EXPECT_EQ(note.getPitch(), "B11");
}

// A sounding pitch below the lowest representable pitch is still accepted, as setPitch() accepts
// it: each setter stores the pitch, the note stays constructible, and its sounding getters report
// the condition.
TEST(NoteMutatorThatThrows, thePartialSettersStillAcceptASoundingPitchBelowTheFloor) {
    Note byStep = transposingNote("D#-1", -1, -3);  // sounds C-1
    byStep.setStep("C");                            // C#-1 sounds at -2
    EXPECT_EQ(byStep.getWrittenPitch(), "C#-1");
    expectSoundingBelowTheFloor(byStep);

    Note byPitchClass = transposingNote("D#-1", -1, -3);
    byPitchClass.setPitchClass("C#");
    EXPECT_EQ(byPitchClass.getWrittenPitch(), "C#-1");
    expectSoundingBelowTheFloor(byPitchClass);

    Note byOctave = transposingNote("C#0", -7, -12);  // sounds Db-1
    byOctave.setOctave(-1);                           // C#-1 sounds at -11
    EXPECT_EQ(byOctave.getWrittenPitch(), "C#-1");
    expectSoundingBelowTheFloor(byOctave);

    Note byAlter = transposingNote("D-1", -1, -2);  // sounds C-1
    byAlter.setAlter(-1.0f);                        // Db-1 sounds at -1
    EXPECT_EQ(byAlter.getWrittenPitch(), "Db-1");
    expectSoundingBelowTheFloor(byAlter);
}

// A rest keeps its transposing interval (setIsNoteOn(false) does not clear it), so the pitch that
// setStep() or setPitchClass() gives it is checked with that interval too: B4 on a (0, 90)
// instrument would sound at 161, above B11, so the rest stays a rest.
TEST(NoteMutatorThatThrows, aRestIsNotGivenAPitchThatCannotSound) {
    for (const bool byStep : {true, false}) {
        Note rest = transposingNote("C4", 0, 90);  // sounds F#11
        rest.setIsNoteOn(false);
        const NoteState before = stateOf(rest);
        const std::string message = thrownFirstLine([&] {
            if (byStep) {
                rest.setStep("B");
            } else {
                rest.setPitchClass("B");
            }
        });
        EXPECT_EQ(message, aboveTheCeiling("B4", 0, 90, "161.000000")) << "byStep " << byStep;
        EXPECT_EQ(stateOf(rest), before) << "byStep " << byStep;
        EXPECT_TRUE(rest.isNoteOff());
        EXPECT_EQ(rest.getTransposeChromatic(), 90);
    }
}

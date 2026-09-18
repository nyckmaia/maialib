#include "maiacore/note.h"

#include <gtest/gtest.h>

#include "pitch-spelling-legacy-data.h"

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
// PITCH SPELLING (SP1)
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

// Quarter-tone alters are reachable here now that Helper::splitPitch accepts them, but
// Note::getEnharmonicPitch() still indexes a 5-slot semitone-only spellings array. Real
// quarter-tone enharmonic spelling remains out of scope (spellMidiNumber() only enumerates
// the five integer-semitone accidentals), so this must refuse rather than silently truncate
// to a wrong semitone spelling.
TEST(PitchSpelling, EnharmonicRejectsQuarterTones) {
    for (const std::string pitch : {"C1x4", "C3x4", "D1b4", "D3b4"}) {
        EXPECT_THROW(Note(pitch).getEnharmonicPitch(false), std::runtime_error)
            << "pitch: " << pitch;
        EXPECT_THROW(Note(pitch).getEnharmonicPitch(true), std::runtime_error)
            << "pitch: " << pitch;
    }
}

// =====================================================================================
// NOTE COMPOSES PITCH (Task 6)
// =====================================================================================

// Step 1: the defect this transplant closes. Before this task, setPitchClass() updated the
// written pitch-class string and accidental symbol but never recomputed the MIDI number, so
// getMidiNumber() kept reporting the note's *previous* pitch. Now that Note holds a single
// Pitch and getMidiNumber() derives from it on demand, this is correct with no special-casing.
TEST(Note, setPitchClassUpdatesAccidentalAndMidi) {
    Note n("C4");
    n.setPitchClass("Eb");
    EXPECT_EQ(n.getAlterSymbol(), "b");
    EXPECT_EQ(n.getMidiNumber(), 63);
}

// T1: Note(pitch, isNoteOn=false) is a fully consistent rest -- every getter agrees, not just
// isNoteOn(). The "C4" is deliberately discarded (see the constructor's rest guard).
TEST(NoteComposesPitch, ConstructorIsNoteOnFalseIsAFullyConsistentRest) {
    const Note n("C4", RhythmFigure::QUARTER, /*isNoteOn=*/false);
    EXPECT_FALSE(n.isNoteOn());
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_EQ(n.getPitchClass(), "rest");
    EXPECT_EQ(n.getPitch(), "rest");
    EXPECT_EQ(n.getMidiNumber(), -1);
    EXPECT_EQ(n.getPitchStep(), "rest");
    EXPECT_EQ(n.getOctave(), -2);
}

// T2: setIsNoteOn(false) used to only flip a bool, leaving the pitch string/MIDI fields stale
// (getPitchClass() kept reporting "C#" and getMidiNumber() kept reporting 60). Deriving
// isNoteOn() from _writtenPitch.isRest() and driving setIsNoteOn(false) through
// _writtenPitch.setPitch("rest") closes that: every getter is consistent immediately.
TEST(NoteComposesPitch, SetIsNoteOnFalseReportsRestEverywhere) {
    Note n("C#4");
    n.setIsNoteOn(false);
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_FALSE(n.isNoteOn());
    EXPECT_EQ(n.getPitchClass(), "rest");
    EXPECT_EQ(n.getPitch(), "rest");
    EXPECT_EQ(n.getMidiNumber(), -1);
    EXPECT_EQ(n.getPitchStep(), "rest");
    EXPECT_EQ(n.getOctave(), -2);
}

// T3: setIsNoteOn(true) on a rest carries no pitch to resurrect one with, so it must refuse
// (LOG_WARN, no throw) rather than flip the flag under a still-empty pitch. Before this guard,
// Note(""); n.setIsNoteOn(true); made getWrittenPitchStep()'s unconditional substr(0, 1) return
// the literal string "r" (the first character of "rest").
TEST(NoteComposesPitch, SetIsNoteOnTrueOnRestRefusesAndWarns) {
    Note n("");
    ASSERT_TRUE(n.isNoteOff());
    n.setIsNoteOn(true);  // must not throw
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_FALSE(n.isNoteOn());
    EXPECT_EQ(n.getWrittenPitchStep(), "rest");
    EXPECT_EQ(n.getPitchClass(), "rest");
}

// T4: getOctave()/getWrittenOctave() used to return 0 for a rest while getSoundingOctave()
// returned -2 -- three different answers to "what octave is a rest". All three now derive from
// Pitch's std::optional<int> octave and agree on the sentinel -2 (0 is a legitimate octave).
TEST(NoteComposesPitch, AllOctaveGettersAgreeOnRestSentinel) {
    const Note n("rest");
    EXPECT_EQ(n.getOctave(), -2);
    EXPECT_EQ(n.getWrittenOctave(), -2);
    EXPECT_EQ(n.getSoundingOctave(), -2);
}

// T7: setOctave() used to write the caller's octave straight into a rest's fields
// (Note("rest").setOctave(5) left getOctave() == 0). Delegating to Pitch::setOctave() inherits
// its refuse-on-rest policy: a bare octave carries no pitch to resurrect one with.
TEST(NoteComposesPitch, SetOctaveOnRestRefusesAndWarns) {
    Note n("rest");
    n.setOctave(5);  // must not throw
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_EQ(n.getOctave(), -2);
    EXPECT_EQ(n.getPitchClass(), "rest");
}

// T8: setAlter() is new on Note (delegating to Pitch::setAlter()) and inherits the same
// refuse-on-rest policy as setOctave(): neither carries enough information to resurrect one.
TEST(NoteComposesPitch, SetAlterOnRestRefusesAndWarns) {
    Note n("rest");
    n.setAlter(0.5f);  // must not throw
    EXPECT_TRUE(n.isNoteOff());
    EXPECT_EQ(n.getOctave(), -2);
    EXPECT_EQ(n.getPitchClass(), "rest");
}

// T9: setStep() is new on Note (delegating to Pitch::setStep()) and is permissive on a rest,
// unlike setOctave()/setAlter(): a diatonic step is enough to resurrect one, defaulting the
// octave to 4.
TEST(NoteComposesPitch, SetStepResurrectsRestToOctave4) {
    Note n("rest");
    n.setStep("C");
    EXPECT_TRUE(n.isNoteOn());
    EXPECT_FALSE(n.isNoteOff());
    EXPECT_EQ(n.getPitch(), "C4");
    EXPECT_EQ(n.getOctave(), 4);
}

// Fix round 1 (controller ruling on Task 6 concern 2): getAlterSymbol() forwarding to the
// sounding pitch instead of the written one is a genuine behaviour change the brief introduced
// (its "preserves today's semantics" claim was wrong: the pre-Task-6 body was
// `return _alterSymbol;`, populated from the WRITTEN pitch only). Pin it with a transposing
// instrument case where the two genuinely diverge.
//
// B-flat clarinet: written C sounds a major second lower (concert Bb). Measured via this exact
// construction, not assumed -- and cross-checked against the pre-existing, already-passing
// NoteSetPitch.WrittenAndSoundingPitchTypesAndOctave_TransposeInstrumentChangeOctave test, which
// pins the same (pitch="C4", transposeDiatonic=-1, transposeChromatic=-2) construction producing
// getSoundingPitch() == "Bb3":
//   written "C4"  -> alter symbol ""  (natural)
//   sounding "Bb3" -> alter symbol "b" (flat), MIDI 58
//
// The controller's suggested case -- written "C#4" under the same transpose -- was tried first
// and discarded: it lands in a different branch of the pre-existing (untouched by Task 6)
// sharp/flat scale-lookup in computeSoundingPitch(), where a written sharp-side pitch class
// transposed downward is not found in either lookup scale (only in the sharp one), and produces
// a musically wrong "Bb4" / MIDI 70 (a fourth higher, not a major second lower). That is a
// latent, pre-existing bug unrelated to this getAlterSymbol() question, not something to build a
// pinning test on top of.
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

// Fix round 2 (controller ruling on the reviewer's Critical finding): getMidiNumber() used to
// route through computeSoundingPitch()'s spelling lookup for a transposed note, letting that
// lookup's pre-existing spelling defect corrupt the numeric MIDI answer too. All three values
// below were measured against a d26aa67 worktree, not assumed -- and cross-checked with a
// 1197-combination sweep ((pitch x transposeDiatonic x transposeChromatic), spanning naturals,
// sharps, flats and double accidentals) that found zero divergence between this fixed HEAD and
// d26aa67 on getMidiNumber(), getOctave() and getPitch(), everywhere.
//
// getPitch()'s output below is pinned as CURRENT, DEFECTIVE behaviour, not the intended one.
// Two separate, genuinely pre-existing defects reproduce byte-for-byte at d26aa67 and belong to
// Task 10's scale-lookup rewrite, not this branch:
//   1. Pitch CLASS (letter+accidental): e.g. "Bb" where "B" is the correct spelling for this
//      MIDI number.
//   2. Octave-increment boundary: computeSoundingPitch()'s octave-wrap check only fires for a
//      transpose index strictly greater than one octave, so an exact +/-12 semitone transpose
//      (the Piccolo case below) fails to increment/decrement the octave at all. The CORRECT
//      octave for that case is called out explicitly in that test.
TEST(NoteComposesPitch, GetMidiNumberIsArithmeticForBFlatClarinet) {
    // B-flat clarinet: written C#4 sounds a major second lower.
    const Note n("C#4", RhythmFigure::QUARTER, true, false, -1, -2);
    EXPECT_EQ(n.getMidiNumber(), 59);
    EXPECT_EQ(n.getOctave(), 4);
    EXPECT_EQ(n.getPitch(), "Bb3");  // wrong spelling, pre-existing, not this round's to fix
}

TEST(NoteComposesPitch, GetMidiNumberIsArithmeticForHornInF) {
    // Horn in F: written F#4 sounds a perfect fifth lower.
    const Note n("F#4", RhythmFigure::QUARTER, true, false, -4, -7);
    EXPECT_EQ(n.getMidiNumber(), 59);
    EXPECT_EQ(n.getOctave(), 4);
    EXPECT_EQ(n.getPitch(), "F3");  // wrong spelling, pre-existing, not this round's to fix
}

TEST(NoteComposesPitch, GetMidiNumberIsArithmeticForPiccolo) {
    // Piccolo: written C4 sounds an octave higher.
    const Note n("C4", RhythmFigure::QUARTER, true, false, 7, 12);
    EXPECT_EQ(n.getMidiNumber(), 72);
    // Pinning CURRENT, DEFECTIVE behaviour (Task 10's pre-existing octave-increment boundary
    // bug, see the block comment above): the correct octave for this construction is 5, one
    // higher than written, matching a full-octave transpose. 4 (== the written octave,
    // unchanged) is what computeSoundingPitch() actually returns today, because its octave-wrap
    // check does not fire for a transpose index of exactly one octave (+/-12 semitones).
    EXPECT_EQ(n.getOctave(), 4);
    EXPECT_EQ(n.getPitch(), "C5");
}

// N1 (controller ruling on the re-review's Important finding): getEnharmonicPitch() used to
// derive its own MIDI number from computeSoundingPitch() directly -- the same buggy
// scale-lookup-tracked octave getOctave() intentionally still reproduces -- while getPitch()
// (fixed in round 2) used the arithmetic octave. The two disagreed: for this exact
// construction, getPitch() returned "C5" while getEnharmonicPitch(false) returned "Dbb4", the
// same note spelled a full octave apart -- HEAD contradicting itself, no baseline needed to see
// it. Fixed by re-deriving getEnharmonicPitch()'s MIDI number from getPitch() (exactly as
// d26aa67 did, via Helper::pitch2midiNote()), which is correct again since round 2. Measured
// against a d26aa67 worktree for four cases (untransposed control plus these three transposing
// instruments) and matched exactly in every one; only the Piccolo case is pinned here since the
// other two are already covered by the getMidiNumber() tests above.
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
    // It is NOT a general invariant of the library today, despite reading like one. Measured, it
    // holds here only because the Piccolo's sounding pitch CLASS happens to be spelled
    // correctly; it FAILS right now for the B-flat clarinet (59 vs 58) and the horn in F (59 vs
    // 53), where the pre-existing transpose scale-lookup picks the wrong spelling. Task 10's
    // rewrite of that lookup is what would make "an enharmonic respelling describes the same
    // pitch as the note it was spelled from" true everywhere -- do not promote this check to
    // those two cases before then.
    EXPECT_EQ(Note(n.getEnharmonicPitch(false)).getMidiNumber(), n.getMidiNumber());
    EXPECT_EQ(Note(n.getEnharmonicPitch(true)).getMidiNumber(), n.getMidiNumber());
}

// Fix round 4 (controller ruling on re-review 2's finding P1). getOctave() is the last method
// that still takes a NUMBER out of computeSoundingPitch()'s pre-existing, defective transpose
// scale lookup. On the CONSTRUCTION path that faithfully reproduces d26aa67 (the round-2 sweep
// confirmed it in 1197 of 1197 combinations). On the setOctave() MUTATION path it does not:
// d26aa67's setOctave() recomputed the sounding octave arithmetically (written MIDI + chromatic
// transpose -> midiNote2pitch -> octave) and bypassed the lookup entirely, so after any
// setOctave() call the two answers differ by a full octave for every transposing instrument
// whose pitch class that lookup mis-spells.
//
// THE OCTAVES PINNED BELOW ARE THE KNOWN-DEFECTIVE VALUES, NOT THE CORRECT ONES. Measured,
// HEAD against a d26aa67 worktree, calling setOctave() with the note's OWN current octave (4),
// so nothing but the derivation mechanism can explain the difference:
//
//   case                         getOctave() HEAD   getOctave() d26aa67   CORRECT
//   B-flat clarinet  C#4 -1/-2           4                   3              3
//   horn in F        F#4 -4/-7           4                   3              3
//   piccolo          C4  +7/+12          4                   5              5
//
// HEAD answers "the written octave, unchanged" in all three, because that lookup's octave-wrap
// check never fires for these transpositions. The correct answer is the sounding octave: one
// BELOW written for the clarinet and horn, one ABOVE for the piccolo -- which is exactly what
// getSoundingOctave() (arithmetic since round 2) already returns, asserted alongside each case.
//
// This is pinned, NOT restored, deliberately. d26aa67 was history-dependent here: constructing
// a transposed note reported one octave, then calling setOctave() with that very same value
// reported another, without anything about the note having changed. HEAD is consistently wrong
// where d26aa67 was inconsistently wrong, and consistent wrongness is what lets TASK 10 fix the
// scale lookup once and correct every dependant at the same time. TASK 10 OWNS THIS: when that
// lookup is rewritten, the three getOctave() expectations below must become 3, 3 and 5.
TEST(NoteComposesPitch, GetOctaveAfterSetOctaveOnTransposedNoteIsPinnedDefective) {
    // B-flat clarinet: written C#4 sounds a major second lower.
    {
        Note n("C#4", RhythmFigure::QUARTER, true, false, -1, -2);
        ASSERT_EQ(n.getOctave(), 4);  // construction path: matches d26aa67
        n.setOctave(4);               // no-op value: the note's own current octave
        EXPECT_EQ(n.getOctave(), 4);  // DEFECTIVE (Task 10); d26aa67 gave 3; correct is 3
        EXPECT_EQ(n.getSoundingOctave(), 3);  // arithmetic, correct, untouched by the defect
        EXPECT_EQ(n.getMidiNumber(), 59);
        EXPECT_EQ(n.getPitch(), "Bb3");  // wrong pitch class, pre-existing (Task 10)
    }

    // Horn in F: written F#4 sounds a perfect fifth lower.
    {
        Note n("F#4", RhythmFigure::QUARTER, true, false, -4, -7);
        ASSERT_EQ(n.getOctave(), 4);
        n.setOctave(4);
        EXPECT_EQ(n.getOctave(), 4);  // DEFECTIVE (Task 10); d26aa67 gave 3; correct is 3
        EXPECT_EQ(n.getSoundingOctave(), 3);
        EXPECT_EQ(n.getMidiNumber(), 59);
        EXPECT_EQ(n.getPitch(), "F3");
    }

    // Piccolo: written C4 sounds an octave higher.
    {
        Note n("C4", RhythmFigure::QUARTER, true, false, 7, 12);
        ASSERT_EQ(n.getOctave(), 4);
        n.setOctave(4);
        EXPECT_EQ(n.getOctave(), 4);  // DEFECTIVE (Task 10); d26aa67 gave 5; correct is 5
        EXPECT_EQ(n.getSoundingOctave(), 5);
        EXPECT_EQ(n.getMidiNumber(), 72);
        EXPECT_EQ(n.getPitch(), "C5");
    }
}

#include <gtest/gtest.h>
#include <note.h>

using namespace testing;

TEST(NoteConstructor, midiNumber)
{
    Note note(60);

    EXPECT_EQ(note.getPitch(), "C4");
}

TEST(NoteConstructor, onlyPitchClass_getPitchTypesOctaveAndDuration)
{
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

TEST(NoteEqualsOperator, nonEnharmonicNotes)
{
    Note a("A");
    Note b("A");

    EXPECT_EQ(a == b, true);
    EXPECT_EQ(a.getMIDINumber() == b.getMIDINumber(), true);
}

TEST(NoteEqualsOperator, enharmonicNotes)
{
    Note a("A");
    Note b("Gx");

    EXPECT_EQ(a == b, false);
    EXPECT_EQ(a.getMIDINumber() == b.getMIDINumber(), true);
}

TEST(NoteSetPitch, writtenAndSoundingPitchTypesAndOctave)
{
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

TEST(NoteSetPitch, writtenAndSoundingPitchTypesAndOctave_transposeInstrument)
{
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

TEST(NoteSetPitch, writtenAndSoundingPitchTypesAndOctave_transposeInstrumentChangeOctave)
{
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

TEST(NoteSetPitch, getPitchTypesAndOctave)
{
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

TEST(getEnharmonicPitch, nonAlternativeEnhamonicPitch)
{
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

    // TODO: Add another pitches to fill the full musical scale

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

TEST(getEnharmonicPitch, alternativeEnhamonicPitch)
{
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

    // TODO: Add another pitches to fill the full musical scale
}

TEST(getEnharmonicPitches, returnEnharmonicOnly)
{
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
    EXPECT_EQ(Cbb4.getEnharmonicPitches(), std::vector<std::string>({"Bb3", "A#3"}));
    EXPECT_EQ(Cb4.getEnharmonicPitches(), std::vector<std::string>({"B3", "Ax3"}));
    EXPECT_EQ(C4.getEnharmonicPitches(), std::vector<std::string>({"Dbb4", "B#3"}));
    EXPECT_EQ(Csp4.getEnharmonicPitches(), std::vector<std::string>({"Db4", "Bx3"}));
    EXPECT_EQ(Cdsp4.getEnharmonicPitches(), std::vector<std::string>({"D4", "Ebb4"}));

    // ===== Pitch Step: D ===== //
    EXPECT_EQ(Dbb4.getEnharmonicPitches(), std::vector<std::string>({"C4", "B#3"}));
    EXPECT_EQ(Db4.getEnharmonicPitches(), std::vector<std::string>({"C#4", "Bx3"}));
    EXPECT_EQ(D4.getEnharmonicPitches(), std::vector<std::string>({"Ebb4", "Cx4"}));
    EXPECT_EQ(Dsp4.getEnharmonicPitches(), std::vector<std::string>({"Eb4", "Fbb4"}));
    EXPECT_EQ(Ddsp4.getEnharmonicPitches(), std::vector<std::string>({"E4", "Fb4"}));

    // TODO: Add another pitches to fill the full musical scale
}

TEST(getEnharmonicPitches, returnAllEnharmonicPitches)
{
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

    // TODO: Add another pitches to fill the full musical scale
}

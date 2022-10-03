#include <gtest/gtest.h>
#include "note.h"
#include "chord.h"

using namespace testing;

TEST(getName, majorTriad01)
{
    Chord chord({"C4", "E4", "G4"});
    EXPECT_EQ(chord.getName(), "C");
}

TEST(getName, majorTriad02)
{
    Chord chord({"C4", "D7", "G6", "E4"});
    EXPECT_EQ(chord.getName(), "C9");
}

TEST(getName, majorWithBass)
{
    Chord chord({"C4", "E4", "G3", "B4"});
    EXPECT_EQ(chord.getName(), "C7M/G");
}

TEST(getName, majorMinorSeventh)
{
    Chord chord({"E5", "C5", "G3", "Bb3"});
    EXPECT_EQ(chord.getName(), "C7/G");
}

TEST(getName, majorMajorSeventh)
{
    Chord chord({"C", "E", "G", "B"});
    EXPECT_EQ(chord.getName(), "C7M");
}

TEST(getName, minorTriad01)
{
    Chord chord({"A4", "C5", "E7"});
    EXPECT_EQ(chord.getName(), "Am");
}

TEST(getName, minorTriad02)
{
    Chord chord({"C5", "Eb5", "G3"});
    EXPECT_EQ(chord.getName(), "Cm/G");
}

TEST(getName, minorSeventhWithBass)
{
    Chord chord({"A5", "C5", "E7", "G3"});
    EXPECT_EQ(chord.getName(), "Am7/G");
}

TEST(isTonal, tonalMajorChord01)
{
    Chord chord({"C", "E", "G"});
    EXPECT_EQ(chord.isTonal(), true);
}

TEST(isTonal, tonalMinorChord01)
{
    Chord chord({"D", "F", "A"});
    EXPECT_EQ(chord.isTonal(), true);
}

TEST(isTonal, notTonalMinorChord01)
{
    Chord chord({"D", "F", "A#"});
    EXPECT_EQ(chord.isTonal(), false);
}

TEST(getCloseStackChord, majorAndMinorChords)
{
    // Exact the same chords
    Chord chord01_test({"C4", "E4", "G4", "B4"});
    Chord chord01_answ({"C4", "E4", "G4", "B4"});
    EXPECT_EQ(chord01_test.getCloseStackChord().getNotes(), chord01_answ.getNotes());

    // Different note sequency
    Chord chord02_test({"Eb4", "C4", "B4", "G4"});
    Chord chord02_answ({"C4", "Eb4", "G4", "B4"});
    EXPECT_EQ(chord02_test.getCloseStackChord().getNotes(), chord02_answ.getNotes());

    // Both different note sequency and octave
    Chord chord03_test({"Eb3", "C5", "B2", "G9"});
    Chord chord03_answ({"C4", "Eb4", "G4", "B4"});
    EXPECT_EQ(chord03_test.getCloseStackChord().getNotes(), chord03_answ.getNotes());
}

TEST(getCloseStackChord, complexChords)
{
    // // Inverted major-seven-nine (5th ommited) chord
    // Chord chord01_test({"D2", "C4", "E5", "B4"});
    // Chord chord01_answ({"C4", "E4", "B4", "D5"});
    
    // std::string test01;
    // std::string answer01;

    // for (int i = 0; i < chord01_answ.size(); i++) {
    //     test01 += chord01_test.getCloseStackChord().getNotes()[i].getPitch() + " ";
    //     answer01 += chord01_answ.getNotes()[i].getPitch() + " ";
    // }

    // EXPECT_EQ(test01, answer01);

    // // Inverted minor-seven-nine (5th ommited) chord
    // Chord chord02_test({"F2", "E5", "D3", "C4"});
    // Chord chord02_answ({"D4", "F4", "C5", "E5"});
    // std::string test02;
    // std::string answer02;

    // for (int i = 0; i < chord02_answ.size(); i++) {
    //     test02 += chord02_test.getCloseStackChord().getNotes()[i].getPitch() + " ";
    //     answer02 += chord02_answ.getNotes()[i].getPitch() + " ";
    // }

    // EXPECT_EQ(test02, answer02);

    // // Inverted minor-seven-eleven (5th ommited) chord
    // Chord chord03_test({"F2", "G5", "D3", "C4"});
    // Chord chord03_answ({"D4", "F4", "C5", "G5"});
    // std::string test03;
    // std::string answer03;

    // for (int i = 0; i < chord03_answ.size(); i++) {
    //     test03 += chord03_test.getCloseStackChord().getNotes()[i].getPitch() + " ";
    //     answer03 += chord03_answ.getNotes()[i].getPitch() + " ";
    // }

    // EXPECT_EQ(test03, answer03);

    // Four notes chromatic cluster
    Chord chord04_test({"C4", "C#4", "D4", "D#4"});
    Chord chord04_answ({"C4", "Eb4", "Bx4", "D5"}); // [C4, Eb4, <G4>, Bx4, D5] => Cm(7aug)9
    EXPECT_EQ(chord04_test.getCloseStackChord().getNotes(), chord04_answ.getNotes());

    std::string test04;
    std::string answer04;

    for (int i = 0; i < chord04_answ.size(); i++) {
        test04 += chord04_test.getCloseStackChord().getNotes()[i].getPitch() + " ";
        answer04 += chord04_answ.getNotes()[i].getPitch() + " ";
    }

    EXPECT_EQ(test04, answer04);
}

TEST(chordOperator, plus)
{
    Chord chord01({"C", "E", "G"});
    Chord chord02({"B", "D", "F"});

    Chord chord3 = chord01 + chord02;

    EXPECT_EQ(chord3.size(), chord01.size() + chord02.size());
}

TEST(chordOperator, StreamLeft)
{
    Note a("C");
    Note b("E");

    Chord chord01;
    chord01 << a;
    chord01 << b;

    Chord chord02;
    chord02.addNote(a);
    chord02.addNote(b);

    EXPECT_EQ(chord01, chord02);
}

TEST(chordOperator, StreamRight)
{
    Note a("C");
    Note b("E");
    Note c("G");

    Chord chord;
    chord.addNote(a);
    chord.addNote(b);
    chord.addNote(c);

    Note x;
    chord >> x;

    EXPECT_EQ(chord.size(), 2);
    EXPECT_EQ(x.getPitch(), c.getPitch());
}

#include "maiacore/chord.h"

#include <gtest/gtest.h>

#include "maiacore/note.h"
using namespace testing;

TEST(getName, majorTriad01) {
    Chord chord({"C4", "E4", "G4"});
    EXPECT_EQ(chord.getName(), "C");
}

TEST(getName, majorTriad02) {
    Chord chord({"C4", "D7", "G6", "E4"});
    EXPECT_EQ(chord.getName(), "C9");
}

TEST(getName, majorWithBass) {
    Chord chord({"C4", "E4", "G3", "B4"});
    EXPECT_EQ(chord.getName(), "C7M/G");
}

TEST(getName, majorMinorSeventh) {
    Chord chord({"E5", "C5", "G3", "Bb3"});
    EXPECT_EQ(chord.getName(), "C7/G");
}

TEST(getName, majorMajorSeventh) {
    Chord chord({"C", "E", "G", "B"});
    EXPECT_EQ(chord.getName(), "C7M");
}

TEST(getName, minorTriad01) {
    Chord chord({"A4", "C5", "E7"});
    EXPECT_EQ(chord.getName(), "Am");
}

TEST(getName, minorTriad02) {
    Chord chord({"C5", "Eb5", "G3"});
    EXPECT_EQ(chord.getName(), "Cm/G");
}

TEST(getName, minorSeventhWithBass) {
    Chord chord({"A5", "C5", "E7", "G3"});
    EXPECT_EQ(chord.getName(), "Am7/G");
}

TEST(isTonal, tonalMajorChord01) {
    Chord chord({"C", "E", "G"});
    EXPECT_EQ(chord.isTonal(), true);
}

TEST(isTonal, tonalMinorChord01) {
    Chord chord({"D", "F", "A"});
    EXPECT_EQ(chord.isTonal(), true);
}

TEST(isTonal, notTonalMinorChord01) {
    Chord chord({"D", "F", "A#"});
    EXPECT_EQ(chord.isTonal(), false);
}

TEST(getCloseStackChord, majorAndMinorChords) {
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

TEST(getCloseStackChord, complexChords01) {
    // Inverted major-seven-nine (5th ommited) chord
    Chord chord01_test({"D2", "C4", "E5", "B4"});
    Chord chord01_answ({"C4", "E4", "B4", "D5"});

    std::string test01;
    std::string answer01;

    for (int i = 0; i < chord01_answ.size(); i++) {
        test01 += chord01_test.getCloseStackChord().getNotes()[i].getPitch() + " ";
        answer01 += chord01_answ.getNotes()[i].getPitch() + " ";
    }

    EXPECT_EQ(test01, answer01);
}

TEST(getCloseStackChord, complexChords02) {
    // Inverted minor-seven-nine (5th ommited) chord
    Chord chord02_test({"F2", "E5", "D3", "C4"});
    Chord chord02_answ({"D4", "F4", "C5", "E5"});
    std::string test02;
    std::string answer02;

    for (int i = 0; i < chord02_answ.size(); i++) {
        test02 += chord02_test.getCloseStackChord().getNotes()[i].getPitch() + " ";
        answer02 += chord02_answ.getNotes()[i].getPitch() + " ";
    }

    EXPECT_EQ(test02, answer02);
}

// TEST(getCloseStackChord, complexChords03) {
// // Inverted minor-seven-eleven (5th ommited) chord
// Chord chord03_test({"F2", "G5", "D3", "C4"});
// Chord chord03_answ({"D4", "F4", "C5", "G5"});
// std::string test03;
// std::string answer03;

// for (int i = 0; i < chord03_answ.size(); i++) {
//   test03 += chord03_test.getCloseStackChord().getNotes()[i].getPitch() + " ";
//   answer03 += chord03_answ.getNotes()[i].getPitch() + " ";
// }

// EXPECT_EQ(test03, answer03);
// }

TEST(getCloseStackChord, complexChords04) {
    // Four notes chromatic cluster
    Chord chord04_test({"C4", "C#4", "D4", "D#4"});
    Chord chord04_answ({"C4", "Eb4", "Bx4", "D5"});  // [C4, Eb4, <G4>, Bx4, D5] => Cm(7aug)9
    EXPECT_EQ(chord04_test.getCloseStackChord().getNotes(), chord04_answ.getNotes());

    std::string test04;
    std::string answer04;

    for (int i = 0; i < chord04_answ.size(); i++) {
        test04 += chord04_test.getCloseStackChord().getNotes()[i].getPitch() + " ";
        answer04 += chord04_answ.getNotes()[i].getPitch() + " ";
    }

    EXPECT_EQ(test04, answer04);
}

TEST(chordOperator, plus) {
    Chord chord01({"C", "E", "G"});
    Chord chord02({"B", "D", "F"});

    Chord chord3 = chord01 + chord02;

    EXPECT_EQ(chord3.size(), chord01.size() + chord02.size());
}

TEST(isInRootPosition, majorChords) {
    Chord myChord01({"C4", "E4", "G4"});
    EXPECT_EQ(myChord01.isInRootPosition(), true);

    Chord myChord02({"E4", "G4", "C5"});
    EXPECT_EQ(myChord02.isInRootPosition(), false);

    Chord myChord03({"G4", "C5", "E5"});
    EXPECT_EQ(myChord03.isInRootPosition(), false);
}

TEST(isInRootPosition, minorChords) {
    Chord myChord01({"C4", "Eb4", "G4"});
    EXPECT_EQ(myChord01.isInRootPosition(), true);

    Chord myChord02({"Eb4", "G4", "C5"});
    EXPECT_EQ(myChord02.isInRootPosition(), false);

    Chord myChord03({"G4", "C5", "Eb5"});
    EXPECT_EQ(myChord03.isInRootPosition(), false);
}

TEST(isInRootPosition, otherChords) {
    Chord myChord01({"C4", "C#4", "D4", "D#4"});
    EXPECT_EQ(myChord01.isInRootPosition(), true);

    Chord myChord02({"G4", "C4", "E4"});
    EXPECT_EQ(myChord02.isInRootPosition(), true);

    Chord myChord03({"F4", "C4", "Bb4"});
    EXPECT_EQ(myChord03.isInRootPosition(), false);
}

TEST(transpose, throwsWhenPastTopOfSupportedRange) {
    // B11 is MIDI 155 (natural); the highest spellable MIDI is 157 (Bx11), and no natural
    // spelling exists at MIDI 157, so transposing this chord by 2 semitones must throw
    const std::vector<std::string> pitches = {"C4", "B11"};
    Chord myChord(pitches);
    EXPECT_THROW(myChord.transpose(2), std::runtime_error);
}

TEST(stackInThirds, throwsOnEightDistinctPitchClassChord) {
    // A 10th out-of-bounds site the original audit missed: Chord::computeBestOpenStackHeap()
    // reads stackedHeaps[0] unconditionally. Every note has at most 3 enharmonic spellings
    // (itself + 2 alternates), all drawn from only 7 possible pitch letters (A-G). A chord with
    // exactly 8 distinct pitch classes can therefore never be respelled with 8 mutually distinct
    // letters (pigeonhole), so removeHeapsWithDuplicatedPitchSteps() rejects every candidate and
    // 'stackedHeaps' stays empty -- this is a mathematical certainty, not a maybe, for any
    // 8-distinct-pitch-class chord. (More than 8 distinct pitch classes throws earlier, in
    // computeEnharmonicHeaps()'s "Invalid chord size" case, so 8 is the only size that reaches
    // this particular guard.)
    const std::vector<std::string> pitches = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "Db5"};
    Chord myChord(pitches);
    EXPECT_EQ(myChord.size(), 8);
    EXPECT_THROW(myChord.getName(), std::runtime_error);
}

// ====================
// Out-of-bounds guard tests
// ====================

TEST(getNote, throwsOnNegativeIndex) {
    Chord myChord({"C4", "E4", "G4"});
    EXPECT_THROW(myChord.getNote(-1), std::runtime_error);
}

TEST(getNote, throwsOnIndexEqualToSize) {
    Chord myChord({"C4", "E4", "G4"});
    EXPECT_THROW(myChord.getNote(3), std::runtime_error);
}

TEST(getNote, throwsOnEmptyChord) {
    Chord myChord;
    EXPECT_THROW(myChord.getNote(0), std::runtime_error);
}

TEST(getNote, constOverloadThrowsOnEmptyChord) {
    const Chord myChord;
    EXPECT_THROW(myChord.getNote(0), std::runtime_error);
}

TEST(getNote, validIndexDoesNotThrow) {
    Chord myChord({"C4", "E4", "G4"});
    EXPECT_NO_THROW(myChord.getNote(0));
    EXPECT_EQ(myChord.getNote(0).getPitch(), "C4");

    // Pin the upper valid boundary too: a one-off-wrong guard (e.g. '>= size() - 1') would
    // still pass every other test here, since none of them exercised the last valid index.
    EXPECT_NO_THROW(myChord.getNote(2));
    EXPECT_EQ(myChord.getNote(2).getPitch(), "G4");
}

TEST(info, throwsOnEmptyChord) {
    Chord myChord;
    EXPECT_THROW(myChord.info(), std::runtime_error);
}

TEST(toInversion, throwsOnEmptyChord) {
    Chord myChord;
    EXPECT_THROW(myChord.toInversion(1), std::runtime_error);
}

TEST(isInRootPosition, emptyChordReturnsFalse) {
    Chord myChord;
    EXPECT_FALSE(myChord.isInRootPosition());
}

TEST(isInRootPosition, staleCloseStackAfterClearReturnsFalse) {
    // clear() (chord.cpp:36-39) empties '_originalNotes'/'_openStack' but leaves
    // '_closeStack' and '_isStackedInThirds' untouched. A guard that only checks
    // '_closeStack.empty()' would miss this: '_closeStack' is still the stale 3-note stack
    // from before clear(), so it would pass the guard and then read 'tempNotes[0]' out of
    // bounds ('tempNotes' is freshly built from the now-empty '_originalNotes').
    Chord myChord({"C4", "E4", "G4"});
    myChord.getName();  // populate _closeStack
    myChord.clear();
    EXPECT_FALSE(myChord.isInRootPosition());
}

TEST(isInRootPosition, staleCloseStackAfterRemoveNoteToEmptyReturnsFalse) {
    // removeNote() resets '_isStackedInThirds' to false, so isInRootPosition() re-enters
    // stackInThirds(), which early-returns on an empty chord (chord.cpp:362-365) without
    // touching '_closeStack' -- so it is still the stale, non-empty stack from before the
    // notes were removed.
    Chord myChord({"C4", "E4", "G4"});
    myChord.getName();  // populate _closeStack
    myChord.removeNote(0);
    myChord.removeNote(0);
    myChord.removeNote(0);
    EXPECT_FALSE(myChord.isInRootPosition());
}

TEST(getOpenStackIntervals, emptyChordReturnsEmptyVector) {
    Chord myChord;
    EXPECT_TRUE(myChord.getOpenStackIntervals().empty());
}

TEST(getCloseStackIntervals, emptyChordReturnsEmptyVector) {
    Chord myChord;
    EXPECT_TRUE(myChord.getCloseStackIntervals().empty());
}

TEST(toCents, emptyChordReturnsEmptyVector) {
    Chord myChord;
    EXPECT_TRUE(myChord.toCents().empty());
}

TEST(toCents, singleNoteChordReturnsEmptyVector) {
    Chord myChord({"C4"});
    EXPECT_TRUE(myChord.toCents().empty());
}

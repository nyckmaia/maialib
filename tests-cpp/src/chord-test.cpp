#include "maiacore/chord.h"

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

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

// Chord::transpose() and Chord::transposeStackOnly() share Helper::transposePitch() with
// Note::transpose(), which computes on exact pitch positions: a chord keeps its quarter tones when
// transposed by whole semitones, and can be transposed BY a quarter tone (the interval is a
// float).
TEST(transpose, preservesQuarterTonesAndMovesByThem) {
    // Transposing a chord that CONTAINS a quarter tone by whole semitones keeps the quarter tone:
    // "D1x4" for the first note, not "D4".
    const std::vector<std::string> quarterTonePitches = {"C1x4", "E4", "G4"};
    Chord quarterToneChord(quarterTonePitches);
    quarterToneChord.transpose(2);
    EXPECT_EQ(quarterToneChord.getNote(0).getPitch(), "D1x4");
    EXPECT_EQ(quarterToneChord.getNote(1).getPitch(), "F#4");
    EXPECT_EQ(quarterToneChord.getNote(2).getPitch(), "A4");

    // Transposing BY a quarter tone.
    const std::vector<std::string> semitonePitches = {"C4", "G4"};
    Chord semitoneChord(semitonePitches);
    semitoneChord.transpose(0.5f);
    EXPECT_EQ(semitoneChord.getNote(0).getPitch(), "C1x4");
    EXPECT_EQ(semitoneChord.getNote(1).getPitch(), "G1x4");
}

TEST(transpose, rejectsAnIntervalOffTheQuarterToneGrid) {
    const std::vector<std::string> pitches = {"C4", "E4"};
    Chord myChord(pitches);
    try {
        myChord.transpose(0.3f);
        FAIL() << "Expected std::runtime_error for a transposition off the quarter-tone grid";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("multiple of 0.5"), std::string::npos) << "message: " << what;
        EXPECT_NE(what.find("0.3"), std::string::npos) << "message: " << what;
    }
    EXPECT_EQ(myChord.getNote(0).getPitch(), "C4");  // the refused call changed nothing

    // An EMPTY chord rejects it too: the validation runs before the note loop, which would
    // otherwise have no note to reject it on.
    Chord emptyChord;
    try {
        emptyChord.transpose(0.3f);
        FAIL() << "Expected std::runtime_error for an empty chord as well";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("multiple of 0.5"), std::string::npos);
    }
}

// A note transposed out of the representable range raises, and every note is transposed before
// any is stored, so the chord is left exactly as it was: neither half-transposed ({"B3", "C-1"})
// nor with the out-of-range note turned into a rest ({"B3", "rest"}).
TEST(transpose, outOfRangeRaisesAndLeavesTheChordUnchanged) {
    const std::vector<std::string> pitches = {"C4", "C-1"};
    Chord chord(pitches);

    try {
        chord.transpose(-1.0f);
        FAIL() << "Expected std::runtime_error for a note transposed below MIDI 0";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("outside the representable range"), std::string::npos)
            << "message: " << what;
    }

    ASSERT_EQ(chord.size(), 2);
    EXPECT_EQ(chord.getNote(0).getPitch(), "C4");   // not "B3": nothing was stored
    EXPECT_EQ(chord.getNote(1).getPitch(), "C-1");  // not "rest"
}

// The open stack, at the top of the range: a raise part-way through leaves it exactly as it was,
// not half-transposed ({"F4", "A4", "G11"}).
TEST(transposeStackOnly, aFailureLeavesTheStackUnchanged) {
    const std::vector<std::string> pitches = {"C4", "E4", "G11"};
    Chord chord(pitches);
    ASSERT_EQ(chord.getOpenStackNotes().size(), 3u);  // stacks the chord: {C4, E4, G11}

    // G11 + 5 is MIDI 156, which only "B#11" spells; the note's own (natural) spelling overflows
    // to octave 12.
    try {
        chord.transposeStackOnly(5.0f);
        FAIL() << "Expected std::runtime_error for G11 transposed by 5 with a natural spelling";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("within octaves"), std::string::npos)
            << "message: " << e.what();
    }

    const std::vector<Note> stack = chord.getOpenStackNotes();
    ASSERT_EQ(stack.size(), 3u);
    EXPECT_EQ(stack[0].getPitch(), "C4");  // not "F4"
    EXPECT_EQ(stack[1].getPitch(), "E4");  // not "A4"
    EXPECT_EQ(stack[2].getPitch(), "G11");
}

// toInversion() needs no transposition of its own -- its body calls
// _originalNotes[0].transpose(12), so it shares Note::transpose()'s exact arithmetic. Pinned so
// the inversion is not given a private copy of the operation.
TEST(toInversion, movesTheLowestNoteUpOneOctaveThroughNoteTranspose) {
    const std::vector<std::string> pitches = {"C4", "E4", "G4"};
    Chord myChord(pitches);
    myChord.toInversion(1);

    ASSERT_EQ(myChord.size(), 3);
    EXPECT_EQ(myChord.getNote(0).getPitch(), "E4");
    EXPECT_EQ(myChord.getNote(1).getPitch(), "G4");
    EXPECT_EQ(myChord.getNote(2).getPitch(), "C5");

    // The semitone case above DEMONSTRATES the delegation but does not GUARD the quarter tone --
    // it would pass unchanged if the inverted note were rounded back to a semitone. This half
    // guards it: the note moved up an octave keeps its quarter tone, because toInversion() goes
    // through Note::transpose() and so through the exact arithmetic rather than a MIDI integer.
    // C1x4 is exactly 60.5, so an octave up is 72.5, spelled from the base semitone it rounds up
    // to (C#5, alter +1) as C1x5.
    const std::vector<std::string> quarterTonePitches = {"C1x4", "E4", "G4"};
    Chord quarterToneChord(quarterTonePitches);
    quarterToneChord.toInversion(1);

    ASSERT_EQ(quarterToneChord.size(), 3);
    EXPECT_EQ(quarterToneChord.getNote(0).getPitch(), "E4");
    EXPECT_EQ(quarterToneChord.getNote(1).getPitch(), "G4");
    EXPECT_EQ(quarterToneChord.getNote(2).getPitch(), "C1x5");
    EXPECT_TRUE(quarterToneChord.getNote(2).isQuarterTone());
}

TEST(stackInThirds, throwsOnEightDistinctPitchClassChord) {
    // Chord::computeBestOpenStackHeap() reads stackedHeaps[0], so it must throw when no heap
    // survives. Every note has at most 3 enharmonic spellings
    // (itself + 2 alternates), all drawn from only 7 possible pitch letters (A-G), and
    // removeHeapsWithDuplicatedPitchSteps() rejects any respelling where two notes land on the
    // same letter. A chord with more than 8 distinct pitch classes already throws earlier and
    // unconditionally, in computeEnharmonicHeaps()'s "Invalid chord size" case (its switch only
    // has cases 2..8). This 8-distinct-pitch-class chord (one per natural letter, plus Db5 to
    // force an 8th distinct pitch class) is one concrete case that reaches this guard --
    // see 'throwsOnThreeNoteClusterChord' below for a much smaller one: there is no simple
    // "N distinct pitch classes" threshold (see that test for why).
    const std::vector<std::string> pitches = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "Db5"};
    Chord myChord(pitches);
    EXPECT_EQ(myChord.size(), 8);
    EXPECT_THROW(myChord.getName(), std::runtime_error);
}

TEST(stackInThirds, throwsOnThreeNoteClusterChord) {
    // The guard above is reachable at far fewer than 8 notes: pitch classes are de-duplicated by
    // spelling string (chord.cpp hashes getPitchClass()), not by enharmonic-equivalent pitch, so
    // C, C#, Db and B# are four separate pitch classes that all draw their possible letters from
    // just {B, C, D}. Even a 3-note chord this closely clustered can fail to find any respelling
    // that both (a) gives every note a distinct letter and (b) forms a valid stacked-in-thirds
    // heap (STEP 5 additionally requires the first interval, after respelling, to be an exact
    // third) -- so "8 distinct pitch classes" is not a threshold, just one sufficient case.
    const std::vector<std::string> pitches = {"C4", "C#4", "Db4"};
    Chord myChord(pitches);
    EXPECT_EQ(myChord.size(), 3);
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

TEST(isInRootPosition, clearInvalidatesCacheReflectedByNextMutation) {
    // clear() calls invalidateStackCache(), so a stale '_closeStack' is not reachable through the
    // public mutator API, and a bare 'EXPECT_FALSE(isInRootPosition())' here would only duplicate
    // 'emptyChordReturnsFalse' above.
    //
    // addNote() calls invalidateStackCache() unconditionally (chord.cpp), so the three addNote()
    // calls further down would mask a broken clear() regardless of what clear() itself did. The
    // assertion that pins clear()'s own invalidation is the one immediately below, BEFORE any
    // further mutation runs: clear() leaves '_isStackedInThirds' false and '_closeStack' empty, so
    // getName() re-enters stackInThirds(), early-returns on the now-empty '_originalNotes', finds
    // no minor/major third, and returns "". Without that invalidation, '_isStackedInThirds' would
    // stay true, getName() would skip re-stacking entirely, and it would return the STALE
    // pre-clear() name "C" instead.
    Chord myChord({"C4", "E4", "G4"});
    myChord.getName();  // populate the stack cache for the 3-note chord
    myChord.clear();
    EXPECT_EQ(myChord.getName(),
              "");  // pins clear()'s own invalidation, before any other mutator runs
    EXPECT_FALSE(myChord.isInRootPosition());  // empty chord; guard kept as defense in depth

    // Secondary check: confirms the chord behaves correctly after further mutation. Since
    // addNote() invalidates unconditionally on its own, this does NOT by itself prove clear()
    // invalidated anything -- the assertion above does that.
    myChord.addNote("D4");
    myChord.addNote("F4");
    myChord.addNote("A4");
    EXPECT_EQ(myChord.getName(), "Dm");  // must reflect the new chord, not any pre-clear cache
}

TEST(isInRootPosition, removeNoteToEmptyInvalidatesCacheReflectedByNextMutation) {
    // The same check as the test above, via removeNote() instead of clear(): removeNote() calls
    // invalidateStackCache(), so a stale '_closeStack' cannot survive stackInThirds()'s
    // empty-chord early return.
    //
    // Unlike clear()'s test above, this one deliberately does NOT add a getName() == "" check
    // immediately after removeNote() -- that would crash, not fail: clear() explicitly clears
    // '_openStack' itself (chord.cpp), but removeNote() does not (only invalidateStackCache()
    // runs, which deliberately excludes '_openStack' -- see its own doc comment). After
    // removeNote()-to-empty, '_openStack' is therefore stale (still the pre-removal size) while
    // '_closeStack' is correctly emptied; isTonal()'s loop, bounded by the stale
    // stackSize()/_openStack, then reads the now-empty '_closeStack' out of bounds. That is a
    // known defect of the '_originalNotes'/'_openStack' dual representation, unrelated to
    // clear()'s invalidation.
    //
    // removeNote()'s invalidation is independently and safely pinned by
    // chordMutation.getNameAfterRemoveNoteReflectsNewChord, which shrinks a chord from 4 notes to
    // 3 (not to empty) and queries getName() directly afterward with no intervening mutator to
    // mask a broken invalidation -- that path rebuilds '_openStack' via stackInThirds()'s normal
    // (not empty-chord-early-return) path, so it does not hit this crash.
    Chord myChord({"C4", "E4", "G4"});
    myChord.getName();  // populate the stack cache for the 3-note chord
    myChord.removeNote(0);
    myChord.removeNote(0);
    myChord.removeNote(0);
    EXPECT_FALSE(myChord.isInRootPosition());  // empty chord; guard kept as defense in depth

    myChord.addNote("D4");
    myChord.addNote("F4");
    myChord.addNote("A4");
    EXPECT_EQ(myChord.getName(), "Dm");  // must reflect the new chord, not the stale cache
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

// ====================
// Stack cache invalidation tests (build -> query -> mutate -> query again)
//
// A chord re-stacked after a mutation must start from fresh heaps: appending to the previous
// run's '_stackedHeaps' would let computeBestOpenStackHeap() pick a heap sized for the OLD note
// count. That would both let a grown chord bypass the "no valid heap" guard (leaving _closeStack
// undersized relative to the new _openStack -- an out-of-bounds read through
// getCloseStackIntervals/isTonal) and, on the shrink side, index _originalNotes past its new,
// smaller size. Every mutator calls the private invalidateStackCache(), and stackInThirds() also
// clears '_stackedHeaps' itself. These tests pin the sequence, not just the end state.
// ====================

TEST(chordMutation, getNameAfterAddNoteReflectsNewChord) {
    // Stack, grow, stack again: the second getName() must describe the grown chord.
    Chord myChord({"C4", "E4", "G4"});
    EXPECT_EQ(myChord.getName(), "C");

    myChord.addNote("B4");
    EXPECT_EQ(myChord.getName(), "C7M");
}

TEST(chordMutation, getNameAfterRemoveNoteReflectsNewChord) {
    Chord myChord({"C4", "E4", "G4", "B4"});
    EXPECT_EQ(myChord.getName(), "C7M");

    myChord.removeNote(3);  // remove B4
    EXPECT_EQ(myChord.getName(), "C");
}

TEST(chordMutation, growingPastValidHeapAfterStackingStillThrows) {
    // A chord stacked once (populating _stackedHeaps for 3 notes), then grown to 8 distinct
    // pitch classes, must throw from the "no valid heap" guard exactly as an identical
    // freshly-built 8-note chord does, not keep the stale 3-note heap and skip the guard.
    Chord myChord({"C4", "E4", "G4"});
    myChord.getName();  // populate the stack cache at size 3

    myChord.addNote("D4");
    myChord.addNote("F4");
    myChord.addNote("A4");
    myChord.addNote("B4");
    myChord.addNote("Db5");

    EXPECT_EQ(myChord.size(), 8);
    EXPECT_THROW(myChord.getName(), std::runtime_error);
}

TEST(chordMutation, shrinkingAfterStackingDoesNotReadStaleHeap) {
    // A chord stacked once, then shrunk via removeNote(), must not index _originalNotes with a
    // heap size from before the shrink -- past the end of the (now smaller) vector, which reads
    // garbage (e.g. a std::length_error, "string too long") instead of giving a correct,
    // deterministic answer.
    Chord myChord({"C4", "E4", "G4", "B4", "D5"});  // C major ninth
    myChord.getName();                              // populate the stack cache at size 5

    myChord.removeNote(4);  // remove D5
    myChord.removeNote(3);  // remove B4
    EXPECT_EQ(myChord.size(), 3);

    EXPECT_EQ(myChord.getName(), "C");
}

TEST(chordMutation, getCloseStackIntervalsAfterGrowingMatchesNewSize) {
    // getCloseStackIntervals() bounds its loop by stackSize() (i.e. _openStack.size()) but
    // indexes _closeStack, so a stale, undersized _closeStack, left behind by stale heaps, would
    // make it read past the end of _closeStack once the chord had grown.
    Chord myChord({"C4", "E4", "G4"});
    myChord.getName();  // populate the stack cache at size 3

    myChord.addNote("B4");
    myChord.addNote("D5");

    const auto intervals = myChord.getCloseStackIntervals();
    EXPECT_EQ(static_cast<int>(intervals.size()), myChord.stackSize() - 1);
}

// ====================
// operator<< tests
// ====================

TEST(chordStreamOperator, emptyChordDoesNotThrow) {
    // getNote(-1) on an empty chord throws; operator<< must not let that propagate, since gtest
    // itself calls operator<< to format values in failure messages, and a throwing stream
    // operator can turn a clean test failure into a process abort. Mirrors __repr__'s "[]" for an
    // empty chord.
    Chord myChord;
    std::ostringstream out;
    EXPECT_NO_THROW(out << myChord);
    EXPECT_EQ(out.str(), "[]");
}

TEST(chordStreamOperator, nonEmptyChordPrintsPitches) {
    Chord myChord({"C4", "E4", "G4"});
    std::ostringstream out;
    out << myChord;
    EXPECT_EQ(out.str(), "[C4,E4,G4]");
}

// ====================
// setDuration out-of-bounds write tests
//
// stackInThirds() dedups '_openStack' down to one note per unique pitch class, so it can be
// strictly smaller than '_originalNotes'. setDuration() bounds each container's loop by its own
// size: indexing both by '_originalNotes.size()' would be a Python-reachable out-of-bounds WRITE.
// ====================

TEST(setDuration, floatOverloadSafeAfterPitchClassDedupShrinksOpenStack) {
    // Chord(["C4","C5"]); getName(); setDuration(...): the open stack is smaller than the chord.
    // A typed intermediate vector (not a bare 2-element braced-init-list) sidesteps a
    // Chord(vector<Note>) / Chord(vector<string>) overload-resolution ambiguity
    // Clang hits for exactly-2-element lists here; the 3+-element lists used everywhere else in
    // this file don't trigger it. Same pattern already used by the 'transpose' test above.
    const std::vector<std::string> pitches = {"C4", "C5"};
    Chord myChord(pitches);  // 2 notes, same pitch class
    myChord.getName();       // stacks: _openStack dedups to 1 note; _originalNotes stays 2
    ASSERT_EQ(myChord.size(), 2);
    ASSERT_EQ(myChord.stackSize(), 1);

    EXPECT_NO_THROW(myChord.setDuration(1.0f, 256));

    // Duration applied to every original note; the chord is still well-formed afterward (no
    // corrupted state from writing past _openStack's smaller size).
    EXPECT_EQ(myChord.getNote(0).getDurationTicks(), myChord.getNote(1).getDurationTicks());
    EXPECT_EQ(myChord.size(), 2);
    EXPECT_NO_THROW(myChord.getName());
}

TEST(setDuration, durationOverloadSafeAfterPitchClassDedupShrinksOpenStack) {
    const std::vector<std::string> pitches = {"C4", "C5"};
    Chord myChord(pitches);
    myChord.getName();
    ASSERT_EQ(myChord.stackSize(), 1);

    const Duration halfNote(2.0f, 256);
    EXPECT_NO_THROW(myChord.setDuration(halfNote));

    EXPECT_EQ(myChord.getNote(0).getDurationTicks(), myChord.getNote(1).getDurationTicks());
    EXPECT_EQ(myChord.size(), 2);
}

// ====================
// removeTopNote / insertNote / removeNote bounds guard tests
//
// Each of these three checks its index before touching the vector: unchecked, removeTopNote()
// would pop an empty vector (UB), insertNote() would insert at an invalid index (UB for
// noteIndex < 0 or > size()), and removeNote() would erase at one (UB for noteIndex < 0 or
// >= size()). All three are Python-bound. The guards throw with LOG_ERROR, as chord.cpp's other
// index checks do (e.g. getNote).
// ====================

TEST(removeTopNote, throwsOnEmptyChord) {
    Chord myChord;
    EXPECT_THROW(myChord.removeTopNote(), std::runtime_error);
}

TEST(removeTopNote, validCallDoesNotThrow) {
    Chord myChord({"C4", "E4", "G4"});
    EXPECT_NO_THROW(myChord.removeTopNote());
    EXPECT_EQ(myChord.size(), 2);
}

TEST(insertNote, throwsOnNegativeIndex) {
    Chord myChord({"C4", "E4", "G4"});
    Note note("D4");
    EXPECT_THROW(myChord.insertNote(note, -1), std::runtime_error);
}

TEST(insertNote, throwsOnIndexPastSize) {
    Chord myChord({"C4", "E4", "G4"});
    Note note("D4");
    EXPECT_THROW(myChord.insertNote(note, 4), std::runtime_error);
}

TEST(insertNote, indexEqualToSizeAppendsWithoutThrowing) {
    // noteIndex == size() is the valid "append at the end" case (matches std::vector::insert
    // semantics), must not throw.
    Chord myChord({"C4", "E4", "G4"});
    Note note("B4");
    EXPECT_NO_THROW(myChord.insertNote(note, 3));
    EXPECT_EQ(myChord.size(), 4);
}

TEST(removeNote, throwsOnNegativeIndex) {
    Chord myChord({"C4", "E4", "G4"});
    EXPECT_THROW(myChord.removeNote(-1), std::runtime_error);
}

TEST(removeNote, throwsOnIndexEqualToSize) {
    Chord myChord({"C4", "E4", "G4"});
    EXPECT_THROW(myChord.removeNote(3), std::runtime_error);
}

TEST(removeNote, throwsOnEmptyChord) {
    Chord myChord;
    EXPECT_THROW(myChord.removeNote(0), std::runtime_error);
}

// ====================
// TASK 9: THE HARMONIC ANALYSIS REJECTS QUARTER TONES
//
// Every answer the stacked-in-thirds analysis produces -- the thirds, the fifths, the chord
// quality, the scale degrees -- is defined over twelve-tone equal temperament. Given a quarter
// tone the analysis does not fail: it returns a confident wrong answer, which in an analysis
// library is worse than an error. Chord therefore rejects at the single chokepoint every
// analysis method funnels through (stackInThirds()), and roundQuarterTones() is the escape hatch
// for callers who want the analysis anyway.
// ====================

namespace {
// Redirects std::cout for the lifetime of the object, so a test can assert on what the
// LOG_INFO-based printing methods actually wrote. Restores the original buffer in the
// destructor, so an exception escaping the captured call cannot leave std::cout dangling into
// the rest of the suite.
class CoutCapture {
   public:
    CoutCapture() : _oldBuffer(std::cout.rdbuf(_buffer.rdbuf())) {}
    ~CoutCapture() { std::cout.rdbuf(_oldBuffer); }

    std::string str() const { return _buffer.str(); }

   private:
    std::stringstream _buffer;
    std::streambuf* _oldBuffer;
};
}  // namespace

// Asserts that 'statement' is rejected by a quarter-tone guard, identified by a part of its
// message: the remedy "roundQuarterTones" that every Chord guard names, or a guard's own wording.
//
// Asserting merely that the call throws would NOT discriminate these guards: without the Chord
// guard, the stacking code still throws a std::runtime_error for a quarter tone, from the first
// Interval it builds between two of the chord's notes (stackInThirds() ->
// filterTertianHeapsOnly()), whose message names Note::roundToSemitone(), a remedy that cannot
// reach the chord's own notes. What the analysis guard actually adds is a diagnosable failure --
// a message naming the offending note and the chord's escape hatch -- raised at the entry point,
// before stackInThirds() has overwritten '_openStack'. So the message is the behaviour worth
// pinning.
#define EXPECT_REJECTED_NAMING(statement, remedy)                                               \
    do {                                                                                        \
        try {                                                                                   \
            statement;                                                                          \
            ADD_FAILURE() << #statement " did not throw on a quarter-tone chord";               \
        } catch (const std::runtime_error& error) {                                             \
            const std::string actualMessage(error.what());                                      \
            EXPECT_NE(actualMessage.find(remedy), std::string::npos)                            \
                << #statement " threw, but not from the quarter-tone guard: " << actualMessage; \
        }                                                                                       \
    } while (false)

TEST(quarterToneAnalysisGuard, getNameIsRejectedByTheGuard) {
    // Deliberately inconsistent with getName()'s neighbouring behaviour for a NON-TONAL chord,
    // which warns and returns an empty string: a quarter tone is not "an atonal chord", it is
    // input the analyser cannot represent at all. Returning empty would hide the loss silently,
    // and the roundQuarterTones() escape hatch only makes sense if the normal path fails visibly.
    Chord myChord({"C4", "E1b4", "G4"});
    EXPECT_THROW(myChord.getName(), std::runtime_error);
    EXPECT_REJECTED_NAMING(myChord.getName(), "roundQuarterTones");
}

TEST(quarterToneAnalysisGuard, errorMessageNamesTheOffendingNoteAndTheEscapeHatch) {
    // A caller hitting this must be able to learn what to do from the message alone.
    Chord myChord({"C4", "E1b4", "G4"});

    try {
        myChord.getName();
        FAIL() << "getName() did not throw on a chord containing a quarter tone";
    } catch (const std::runtime_error& error) {
        const std::string message(error.what());
        EXPECT_NE(message.find("E1b4"), std::string::npos) << message;
        EXPECT_NE(message.find("roundQuarterTones"), std::string::npos) << message;
    }
}

TEST(quarterToneAnalysisGuard, coversEveryAnalysisEntryPoint) {
    // The analysis surface is defined by measurement rather than taste: a method belongs here if
    // it reaches stackInThirds(), whose prologue -- `if (!_isStackedInThirds) { stackInThirds();
    // }` -- every one of these opens with. stackSize() is included despite looking like a
    // trivial accessor, because it does reach the chokepoint.
    //
    // The guard sits before stackInThirds() mutates anything, so a rejected chord is left
    // untouched and every assertion below sees the same state as the first.
    Chord myChord({"C4", "E1b4", "G4"});
    const Key cMajor("C");

    EXPECT_REJECTED_NAMING(myChord.getName(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getRoot(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getBassNote(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.stackSize(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getStackedHeaps(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getOpenStackNotes(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getOpenStackIntervals(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getCloseStackIntervals(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getCloseStackHarmonicComplexity(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getOpenStackChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getCloseStackChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getCloseChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getDegree(cMajor), "roundQuarterTones");

    // Reached through the chokepoint indirectly, via isTonal()/stackSize()/getDegree().
    EXPECT_REJECTED_NAMING(myChord.getQuality(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getRomanDegree(cMajor), "roundQuarterTones");

    // The isXxx() family.
    EXPECT_REJECTED_NAMING(myChord.isTonal(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isMajorChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isMinorChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isAugmentedChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isDiminishedChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isHalfDiminishedChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isWholeDiminishedChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isDominantSeventhChord(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isSus(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isDyad(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.isInRootPosition(), "roundQuarterTones");

    // The haveXxx() predicates that stack the chord in thirds.
    EXPECT_REJECTED_NAMING(myChord.haveMinorSecond(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMajorSecond(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMinorThird(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMajorThird(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.havePerfectFourth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveAugmentedFourth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveDiminishedFifth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.havePerfectFifth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveAugmentedFifth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMinorSixth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMajorSixth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveDiminishedSeventh(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMinorSeventh(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMajorSeventh(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveDiminishedOctave(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.havePerfectOctave(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveAugmentedOctave(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMinorNinth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMajorNinth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.havePerfectEleventh(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveSharpEleventh(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMinorThirdteenth(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.haveMajorThirdteenth(), "roundQuarterTones");
}

namespace {
// The interval family's rejection, which names the chord-level remedy.
const char* const kIntervalFamilyRejection =
    "Cannot compute the intervals of a chord containing the quarter tone E1b4: interval analysis "
    "is defined only over twelve-tone equal temperament. Call Chord::roundQuarterTones()";
}  // namespace

TEST(quarterToneAnalysisGuard, coversTheIntervalBasedHaveFamilyThroughTheIntervalGuard) {
    // These haveXxx() overloads do NOT reach stackInThirds(): they build Intervals from the
    // original sorted notes instead, and are covered by the interval family's own guard, which
    // names Chord::roundQuarterTones() -- the remedy a Chord caller can apply, unlike the
    // Note::roundToSemitone() an Interval would name.
    Chord myChord({"C4", "E1b4", "G4"});

    EXPECT_REJECTED_NAMING(myChord.haveThird(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.haveFifth(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.haveSeventh(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.haveMajorInterval(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.haveMinorInterval(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.havePerfectInterval(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.haveAnyOctaveMajorThird(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.haveAnyOctavePerfectFifth(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.havePerfectUnisson(), kIntervalFamilyRejection);
}

TEST(quarterToneAnalysisGuard, accessorsAndMutatorsKeepWorking) {
    // The counterpart of the coverage test above, and the reason the guard sits at the analysis
    // chokepoint rather than on the class as a whole: nothing that merely reads or edits the
    // chord's notes touches the stack, so none of it may throw on a quarter-tone chord. A user
    // must still be able to hold, inspect and repair such a chord.
    Chord myChord({"C4", "E1b4", "G4"});

    EXPECT_EQ(myChord.size(), 3);
    EXPECT_NO_THROW(myChord.getNote(1));
    EXPECT_EQ(myChord.getNote(1).getPitch(), "E1b4");
    EXPECT_NO_THROW(myChord.getNotes());
    EXPECT_NO_THROW(myChord[1]);
    EXPECT_NO_THROW(myChord.getDuration());
    EXPECT_NO_THROW(myChord.getQuarterDuration());
    EXPECT_NO_THROW(myChord.getDurationTicks());
    EXPECT_NO_THROW(myChord.setDuration(2.0f));
    EXPECT_NO_THROW(myChord.print());
    EXPECT_NO_THROW(myChord.printStack());
    EXPECT_NO_THROW(myChord.addNote("B4"));
    EXPECT_EQ(myChord.size(), 4);
    EXPECT_NO_THROW(myChord.removeNote(3));
    EXPECT_NO_THROW(myChord.removeTopNote());
    EXPECT_NO_THROW(myChord.clear());
}

TEST(info, degradesInsteadOfThrowingOnQuarterToneChord) {
    // info() is the diagnostic a caller reaches for precisely when holding a chord they do not
    // understand, so it must work on any chord that can be built. It reaches the chokepoint by
    // two routes -- getName() and stackSize() -- and both are skipped here.
    Chord myChord({"C4", "E1b4", "G4"});

    std::string output;
    {
        CoutCapture capture;
        EXPECT_NO_THROW(myChord.info());
        output = capture.str();
    }

    // Still prints what it can: the size and the full note list, quarter tone included.
    EXPECT_NE(output.find("Size:3"), std::string::npos) << output;
    EXPECT_NE(output.find("[C4, E1b4, G4]"), std::string::npos) << output;

    // Says why the analysis is missing instead of printing a name, and names the escape hatch.
    EXPECT_NE(output.find("unavailable"), std::string::npos) << output;
    EXPECT_NE(output.find("roundQuarterTones"), std::string::npos) << output;

    // Skips the stack section rather than throwing on stackSize().
    EXPECT_EQ(output.find("Open Stack Size"), std::string::npos) << output;
}

TEST(info, stillPrintsTheFullAnalysisForASemitoneChord) {
    // The other half of the test above: the degraded path must not leak into ordinary chords.
    Chord myChord({"C4", "E4", "G4"});

    std::string output;
    {
        CoutCapture capture;
        EXPECT_NO_THROW(myChord.info());
        output = capture.str();
    }

    EXPECT_NE(output.find("Name: C"), std::string::npos) << output;
    EXPECT_NE(output.find("Open Stack Size"), std::string::npos) << output;
    EXPECT_EQ(output.find("unavailable"), std::string::npos) << output;
}

TEST(roundQuarterTones, enablesAnalysis) {
    Chord myChord({"C4", "E1b4", "G4"});

    EXPECT_EQ(myChord.roundQuarterTones(), 1);
    EXPECT_EQ(myChord.getName(), "C");
}

TEST(roundQuarterTones, roundsTiesUpwardOnBothSides) {
    // Delegates to Pitch::roundToSemitone(), which rounds with roundTiesUpward() (utils.h), the
    // library's single implementation of the ties-upward rule, floor(alter + 0.5). The flat-side
    // cases are the ones that discriminate it from ties-away-from-zero: std::round(-0.5) would
    // give Eb4 here, and std::round(-1.5) Ebb4.
    // Spelled out rather than braced: a TWO-element braced list of string literals is ambiguous
    // between Chord's vector<string> and vector<Note> constructors, because std::vector's
    // iterator-pair constructor also matches two const char*. Three-element lists elsewhere in
    // this file are unaffected.
    Chord flatSide(std::vector<std::string>{"E1b4", "E3b4"});
    EXPECT_EQ(flatSide.roundQuarterTones(), 2);
    EXPECT_EQ(flatSide.getNote(0).getPitch(), "E4");
    EXPECT_EQ(flatSide.getNote(1).getPitch(), "Eb4");

    Chord sharpSide(std::vector<std::string>{"C1x4", "C3x4"});
    EXPECT_EQ(sharpSide.roundQuarterTones(), 2);
    EXPECT_EQ(sharpSide.getNote(0).getPitch(), "C#4");
    EXPECT_EQ(sharpSide.getNote(1).getPitch(), "Cx4");
}

TEST(roundQuarterTones, returnsZeroAndLeavesASemitoneChordAlone) {
    Chord myChord({"C4", "E4", "G4"});

    EXPECT_EQ(myChord.roundQuarterTones(), 0);
    EXPECT_EQ(myChord.getNote(1).getPitch(), "E4");
    EXPECT_EQ(myChord.getName(), "C");
}

TEST(roundQuarterTones, invalidatesTheStackCache) {
    // The mutable getNote() reference bypasses every mutator, so the stacked-in-thirds cache is
    // still marked valid after the edit below (see operator[]'s @warning in chord.h). Without
    // the invalidateStackCache() call in roundQuarterTones(), getName() would keep answering
    // from the stack computed for the PRE-round pitches and still say "C" at the end.
    Chord myChord({"C4", "E4", "G4"});
    EXPECT_EQ(myChord.getName(), "C");

    myChord.getNote(1).setAlter(-1.5f);  // E4 -> E3b4, a quarter tone, cache left marked valid
    EXPECT_EQ(myChord.getNote(1).getPitch(), "E3b4");

    EXPECT_EQ(myChord.roundQuarterTones(), 1);  // E3b4 -> Eb4, ties upward
    EXPECT_EQ(myChord.getName(), "Cm");
}

TEST(roundQuarterTones, emptyChordReturnsZero) {
    Chord myChord;
    EXPECT_EQ(myChord.roundQuarterTones(), 0);
}

// ===== The MIDI-domain family, which reaches neither analysis guard ===== //
//
// These methods never build an Interval and never stack the chord in thirds, so neither the
// Interval constructor guard nor the stackInThirds() chokepoint can see them. They answer in the
// MIDI integer domain, where Note::getMidiNumber() has already rounded a quarter tone ties-upward.
//
// Each one is classified by a single question -- can its RETURN TYPE express a quarter tone? A
// method that cannot rejects; a method that can computes the true value, because rejecting there
// would destroy functionality that works.

TEST(quarterToneMidiDomainGuard, getMidiIntervalsIsRejected) {
    // Without the guard this would return [4, 3], byte for byte what a plain C major triad
    // returns, because getMidiNumber() rounds E1b4 (63.5) up to E4 (64). A vector<int> of
    // semitone counts cannot hold the 3.5 semitones of a neutral third, so there is no honest
    // value to return.
    Chord myChord({"C4", "E1b4", "G4"});

    EXPECT_REJECTED_NAMING(myChord.getMidiIntervals(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getMidiIntervals(true), "roundQuarterTones");
}

TEST(quarterToneMidiDomainGuard, meanMidiValueFamilyIsRejected) {
    // An int cannot express 63.5, the true mean of {60, 63.5, 67}: answering 63 and "D#4" would
    // give the same answers as a C major triad.
    Chord myChord({"C4", "E1b4", "G4"});

    EXPECT_REJECTED_NAMING(myChord.getMeanMidiValue(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getMeanOfExtremesMidiValue(), "roundQuarterTones");

    // These two spell whatever the int mean above produced, so they inherit that rejection rather
    // than carrying a guard of their own. The mean of a quarter-tone chord's exact positions is
    // generally not on the quarter-tone grid, so there is generally no exact spelling to return.
    // This chord's mean, 63.5, happens to be spellable ("D3x4"), but the rejection does not
    // depend on the particular mean.
    EXPECT_REJECTED_NAMING(myChord.getMeanPitch(), "roundQuarterTones");
    EXPECT_REJECTED_NAMING(myChord.getMeanOfExtremesPitch(), "roundQuarterTones");
}

TEST(quarterToneMidiDomainGuard, messageNamesTheOffendingNoteAndTheEscapeHatch) {
    // A caller hitting this must be able to learn what to do from the message alone.
    Chord myChord({"C4", "E1b4", "G4"});

    try {
        myChord.getMidiIntervals();
        FAIL() << "getMidiIntervals() did not throw on a chord containing a quarter tone";
    } catch (const std::runtime_error& error) {
        const std::string message(error.what());
        EXPECT_NE(message.find("E1b4"), std::string::npos) << message;
        EXPECT_NE(message.find("roundQuarterTones"), std::string::npos) << message;
    }
}

TEST(quarterToneMidiDomainGuard, everyRejectedMethodWorksAfterRoundQuarterTones) {
    // The escape hatch has to open every door its name appears on.
    Chord myChord({"C4", "E1b4", "G4"});
    EXPECT_EQ(myChord.roundQuarterTones(), 1);

    EXPECT_EQ(myChord.getMidiIntervals(), (std::vector<int>{4, 3}));
    EXPECT_EQ(myChord.getMeanMidiValue(), 63);
    EXPECT_EQ(myChord.getMeanOfExtremesMidiValue(), 63);
    EXPECT_EQ(myChord.getMeanPitch(), "D#4");
    EXPECT_EQ(myChord.getMeanOfExtremesPitch(), "D#4");
}

TEST(quarterToneMidiDomainGuard, semitoneChordsAreUntouchedByTheGuard) {
    // The counterpart of the rejections: a chord with no quarter tone is not affected by the
    // guard.
    Chord myChord({"C4", "E4", "G4"});

    EXPECT_EQ(myChord.getMidiIntervals(), (std::vector<int>{4, 3}));
    EXPECT_EQ(myChord.getMidiIntervals(true), (std::vector<int>{4, 7}));
    EXPECT_EQ(myChord.getMeanMidiValue(), 63);
    EXPECT_EQ(myChord.getMeanOfExtremesMidiValue(), 63);
    EXPECT_EQ(myChord.getMeanPitch(), "D#4");
}

TEST(quarterToneAnalysisGuard, getIntervalsIsPinnedToTheIntervalGuard) {
    // getIntervals() builds its Intervals from the unsorted notes, apart from
    // getIntervalsFromOriginalSortedNotes(), so both carry the interval family's guard.
    Chord myChord({"C4", "E1b4", "G4"});

    EXPECT_REJECTED_NAMING(myChord.getIntervals(), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.getIntervals(true), kIntervalFamilyRejection);
    EXPECT_REJECTED_NAMING(myChord.getIntervalsFromOriginalSortedNotes(), kIntervalFamilyRejection);
}

// The remedy the interval family names is one a Chord caller can apply: after
// roundQuarterTones(), the same calls answer.
TEST(quarterToneAnalysisGuard, theIntervalFamilysRemedyWorks) {
    Chord myChord({"C4", "E1b4", "G4"});
    EXPECT_EQ(myChord.roundQuarterTones(), 1);

    EXPECT_EQ(myChord.getIntervals().size(), 2u);
    EXPECT_EQ(myChord.getIntervalsFromOriginalSortedNotes().size(), 2u);
    EXPECT_TRUE(myChord.haveMajorInterval());
}

// A chord with fewer than two notes builds no interval, so a quarter tone in it is not rejected.
TEST(quarterToneAnalysisGuard, aSingleQuarterToneBuildsNoIntervalAndIsNotRejected) {
    const Chord myChord({"E1b4"});
    EXPECT_TRUE(myChord.getIntervals().empty());
    EXPECT_TRUE(myChord.getIntervalsFromOriginalSortedNotes().empty());
}

TEST(toCents, neutralThirdReadsThreeHundredAndFiftyCents) {
    // toCents() computes rather than rejects: cents are the one unit in this library that
    // expresses a quarter tone exactly, so an int loses nothing, and rejecting would destroy
    // working functionality.
    //
    // 350 is the whole point -- not a throw, and not 300 or 400, which is what any route through
    // the rounded getMidiNumber() produces.
    Chord myChord({"C4", "E1b4", "G4"});

    EXPECT_EQ(myChord.toCents(), (std::vector<int>{350, 350}));
}

TEST(toCents, semitoneIntervalsAreExactHundreds) {
    // Integer arithmetic on doubled step positions gives exact hundreds. A frequency route cannot
    // guarantee them: Interval::toCents() compares two frequencies, and
    // Helper::frequencies2cents() truncates 299.9999 to 299 rather than rounding it.
    Chord myChord({"C4", "E4", "G4"});

    EXPECT_EQ(myChord.toCents(), (std::vector<int>{400, 300}));
}

TEST(toCents, quarterAndThreeQuarterToneStepsAndDescendingIntervals) {
    // Spelled out rather than braced: a TWO-element braced list of string literals is ambiguous
    // between Chord's vector<string> and vector<Note> constructors.
    Chord quarterTone(std::vector<std::string>{"C4", "C1x4"});  // 60 -> 60.5
    EXPECT_EQ(quarterTone.toCents(), (std::vector<int>{50}));

    Chord threeQuarterTone(std::vector<std::string>{"C4", "D1b4"});  // 60 -> 61.5
    EXPECT_EQ(threeQuarterTone.toCents(), (std::vector<int>{150}));

    // Descending intervals stay negative, and the half step survives the sign.
    Chord descending(std::vector<std::string>{"E1b4", "C4"});  // 63.5 -> 60
    EXPECT_EQ(descending.toCents(), (std::vector<int>{-350}));
}

TEST(isSorted, quarterToneOrderingIsComputedExactly) {
    // A bool expresses the true answer for a quarter tone exactly, so this computes rather than
    // rejects. Compared by rounded MIDI numbers, BOTH of these would be false: E1b4 (63.5) and E4
    // (64) both round to 64, and the comparator reads equal values as unsorted.
    Chord ascending(std::vector<std::string>{"E1b4", "E4"});
    EXPECT_TRUE(ascending.isSorted());

    Chord descending(std::vector<std::string>{"E4", "E1b4"});
    EXPECT_FALSE(descending.isSorted());
}

TEST(isSorted, semitoneChordsAreUnchanged) {
    Chord sorted({"C4", "E4", "G4"});
    EXPECT_TRUE(sorted.isSorted());

    Chord unsorted({"G4", "E4", "C4"});
    EXPECT_FALSE(unsorted.isSorted());
}

TEST(getMidiValueStd, isTheRealStandardDeviationAndSeparatesAQuarterTone) {
    // A float expresses this spread exactly, so it computes rather than rejects, and the quarter
    // tone separates the two chords. Each value is the population standard deviation of the
    // chord's own positions, with no zero padding (that of {0, 0, 0, 60, 64, 67} would be
    // 31.8978).
    Chord semitone({"C4", "E4", "G4"});  // {60, 64, 67}
    EXPECT_NEAR(semitone.getMidiValueStd(), 2.8674f, 0.001f);

    Chord quarterTone({"C4", "E1b4", "G4"});  // {60, 63.5, 67}
    EXPECT_NEAR(quarterTone.getMidiValueStd(), 2.8577f, 0.001f);
}

TEST(getMidiValueStd, emptyChordReturnsZero) {
    // Not a division by zero: no NaN.
    Chord myChord;
    EXPECT_FLOAT_EQ(myChord.getMidiValueStd(), 0.0f);
}

TEST(getHarmonicDensity, aQuarterToneExtremeWidensTheRangeByHalfASemitone) {
    // A float expresses a 6.5-semitone span exactly, so this computes rather than rejects. C1x4 is
    // 60.5, so its span to G4 (67) is 6.5 semitones, not the 6 that rounding it to 61 produces.
    //
    // Both bounds are passed explicitly because getHarmonicDensity() with NO arguments is
    // ambiguous in C++: both overloads have every parameter defaulted. Python is unaffected --
    // pybind11 resolves the overloads in declaration order.
    Chord quarterTone(std::vector<std::string>{"C1x4", "G4"});
    EXPECT_NEAR(quarterTone.getHarmonicDensity(-1, -1), 2.0f / 7.5f, 0.0001f);

    // A chord with no quarter tone, for comparison.
    Chord semitone({"C4", "E4", "G4"});
    EXPECT_NEAR(semitone.getHarmonicDensity(-1, -1), 0.375f, 0.0001f);
}

// ===== Exact ordering, spread and density ===== //

TEST(sortNotes, sortsAQuarterToneChordAndAgreesWithIsSorted) {
    // sortNotes() and isSorted() must agree. A sorter ordering by the rounded MIDI number would see
    // E4 (64) and E1b4 (rounded to 64) as equal and leave the pair untouched, after which
    // isSorted(), comparing exact positions, would call the result unsorted: a chord sortNotes()
    // could not make sorted.
    Chord myChord(std::vector<std::string>{"E4", "E1b4"});

    myChord.sortNotes();

    EXPECT_EQ(myChord.getNote(0).getPitch(), "E1b4");
    EXPECT_EQ(myChord.getNote(1).getPitch(), "E4");
    EXPECT_TRUE(myChord.isSorted());
}

TEST(sortNotes, semitoneChordRoundTripIsUnchanged) {
    // An ordinary chord sorts, and reads as sorted afterwards.
    Chord myChord({"G4", "E4", "C4"});

    myChord.sortNotes();

    EXPECT_EQ(myChord.getNote(0).getPitch(), "C4");
    EXPECT_EQ(myChord.getNote(1).getPitch(), "E4");
    EXPECT_EQ(myChord.getNote(2).getPitch(), "G4");
    EXPECT_TRUE(myChord.isSorted());
}

TEST(noteOrdering, comparesExactPitchPositionsNotRoundedMidiNumbers) {
    // The operators sortNotes() and isSorted() both go through, and user-visible in their own
    // right: they are bound straight to Python. By rounded MIDI numbers, `quarterTone < semitone`
    // would be false: 63.5 and 64 both round to 64.
    const Note quarterTone("E1b4");  // 63.5
    const Note semitone("E4");       // 64

    EXPECT_TRUE(quarterTone < semitone);
    EXPECT_FALSE(semitone < quarterTone);
    EXPECT_TRUE(semitone > quarterTone);
    EXPECT_FALSE(quarterTone > semitone);
    EXPECT_TRUE(quarterTone <= semitone);
    EXPECT_FALSE(semitone <= quarterTone);
    EXPECT_TRUE(semitone >= quarterTone);
    EXPECT_FALSE(quarterTone >= semitone);

    // operator==/!= compare pitch strings, so they tell these two apart too.
    EXPECT_FALSE(quarterTone == semitone);
    EXPECT_TRUE(quarterTone != semitone);
}

TEST(noteOrdering, semitoneComparisonsAreUnchanged) {
    // Exact and rounded positions coincide for any note without a quarter tone.
    EXPECT_TRUE(Note("C4") < Note("E4"));
    EXPECT_FALSE(Note("G4") < Note("E4"));
    EXPECT_TRUE(Note("C4") <= Note("C4"));
    EXPECT_TRUE(Note("G4") >= Note("G4"));
    EXPECT_TRUE(Note("G4") > Note("C4"));
}

TEST(getFrequencyStd, isTheRealStandardDeviationNotAZeroPaddedOne) {
    // The population standard deviation of the three frequencies themselves, with no zero padding
    // (that of {0, 0, 0, 261.63, 329.63, 392.00} would be 168.144).
    //
    // This does NOT assert anything about quarter tones: Note::getFrequency() derives each
    // frequency from the rounded MIDI number.
    Chord myChord({"C4", "E4", "G4"});

    EXPECT_NEAR(myChord.getFrequencyStd(), 53.24f, 0.05f);
}

TEST(getFrequencyStd, emptyChordReturnsZero) {
    // Not a division by zero: no NaN.
    Chord myChord;

    EXPECT_FLOAT_EQ(myChord.getFrequencyStd(), 0.0f);
}

TEST(getHarmonicDensity, stringBoundsAreExactLikeTheNumericOverload) {
    // The string overload reads its bounds' exact positions. Converting them with
    // Helper::pitch2midiNote(), which rounds, would make "C1x4" (60.5) 61 and give 2/7 = 0.2857,
    // while the numeric overload's auto-detected path gives the true 2/7.5 = 0.2667.
    Chord myChord(std::vector<std::string>{"C1x4", "G4"});

    EXPECT_NEAR(myChord.getHarmonicDensity(std::string("C1x4"), std::string("G4")), 2.0f / 7.5f,
                0.0001f);

    // The two overloads agree about the same span.
    EXPECT_NEAR(myChord.getHarmonicDensity(std::string("C1x4"), std::string("G4")),
                myChord.getHarmonicDensity(-1, -1), 0.0001f);

    // Semitone bounds.
    EXPECT_NEAR(myChord.getHarmonicDensity(std::string("C4"), std::string("G4")), 2.0f / 8.0f,
                0.0001f);
}

#include "maiacore/chord.h"

#include <gtest/gtest.h>

#include <sstream>

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
    // (itself + 2 alternates), all drawn from only 7 possible pitch letters (A-G), and
    // removeHeapsWithDuplicatedPitchSteps() rejects any respelling where two notes land on the
    // same letter. A chord with more than 8 distinct pitch classes already throws earlier and
    // unconditionally, in computeEnharmonicHeaps()'s "Invalid chord size" case (its switch only
    // has cases 2..8). This 8-distinct-pitch-class chord (one per natural letter, plus Db5 to
    // force an 8th distinct pitch class) is one concrete case that reaches this guard --
    // see 'throwsOnThreeNoteClusterChord' below for a much smaller one: there is no simple
    // "N distinct pitch classes" threshold (an earlier version of this comment claimed one; it
    // was wrong -- see that test for why).
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
    // third) -- so "8 distinct pitch classes" was never a real threshold, just one sufficient
    // case found first.
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
    // Renamed and rewritten: this used to be named/worded around the round-1 bug where clear()
    // left '_closeStack' stale (chord.cpp:36-39, before 4158b63). Since 4158b63, clear() calls
    // invalidateStackCache(), so that stale state is no longer reachable through the public
    // mutator API, and a bare 'EXPECT_FALSE(isInRootPosition())' here would only duplicate
    // 'emptyChordReturnsFalse' above.
    //
    // Re-review found that first rewrite still didn't discriminate: addNote() calls
    // invalidateStackCache() unconditionally (chord.cpp), so the three addNote() calls further
    // down would mask a broken clear() regardless of what clear() itself did. The assertion that
    // actually pins clear()'s own invalidation is the one immediately below, BEFORE any further
    // mutation runs. With the fix, clear() leaves '_isStackedInThirds' false and '_closeStack'
    // empty, so getName() re-enters stackInThirds(), early-returns on the now-empty
    // '_originalNotes', finds no minor/major third, and returns "". Without the fix,
    // '_isStackedInThirds' would stay true, getName() would skip re-stacking entirely, and it
    // would return the STALE pre-clear() name "C" instead. Confirmed by deletion: removing
    // clear()'s invalidateStackCache() call made this exact assertion fail (returned "C" instead
    // of ""), and restoring it made it pass again -- see the implementation report for both runs.
    Chord myChord({"C4", "E4", "G4"});
    myChord.getName();  // populate the stack cache for the 3-note chord
    myChord.clear();
    EXPECT_EQ(myChord.getName(), "");  // pins clear()'s own invalidation, before any other mutator runs
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
    // Renamed and rewritten for the same reason as the test above, via removeNote() instead of
    // clear(): removeNote() now calls invalidateStackCache() (4158b63), so the old "stale
    // _closeStack survives stackInThirds()'s empty-chord early return" scenario this test used
    // to set up is no longer reachable.
    //
    // Unlike clear()'s test above, this one deliberately does NOT add a getName() == "" check
    // immediately after removeNote() -- that would crash, not fail: clear() explicitly clears
    // '_openStack' itself (chord.cpp), but removeNote() does not (only invalidateStackCache()
    // runs, which deliberately excludes '_openStack' -- see its own doc comment). After
    // removeNote()-to-empty, '_openStack' is therefore stale (still the pre-removal size) while
    // '_closeStack' is correctly emptied; isTonal()'s loop, bounded by the stale
    // stackSize()/_openStack, then reads the now-empty '_closeStack' out of bounds. This is a
    // real, currently-reachable, pre-existing bug (found incidentally while fixing this test,
    // reported rather than fixed here: it is the '_originalNotes'/'_openStack' dual-
    // representation drift this branch is explicitly not chartered to redesign). Confirmed by
    // running this exact sequence with clear()'s invalidation both removed and restored: the
    // crash reproduces identically either way, proving it is unrelated to clear() and would
    // exist even with every fix on this branch applied.
    //
    // removeNote()'s invalidation is independently and safely pinned by
    // chordMutation.getNameAfterRemoveNoteReflectsNewChord, which shrinks a chord from 4 notes to
    // 3 (not to empty) and queries getName() directly afterward with no intervening mutator to
    // mask a regression -- that path rebuilds '_openStack' via stackInThirds()'s normal (not
    // empty-chord-early-return) path, so it does not hit this crash.
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
// Re-review found stackInThirds() never clears the member '_stackedHeaps': a chord re-stacked
// after a mutation kept appending to the previous run's heaps instead of starting fresh, so
// computeBestOpenStackHeap() could pick a heap sized for the OLD note count. That both let a
// grown chord bypass the "no valid heap" guard (leaving _closeStack undersized relative to the
// new _openStack -- a live out-of-bounds read through getCloseStackIntervals/isTonal) and, on
// the shrink side, indexed _originalNotes past its new, smaller size. Fixed with a private
// invalidateStackCache() helper called from every mutator, plus clearing '_stackedHeaps' inside
// stackInThirds() itself. These tests pin the sequence, not just the end state.
// ====================

TEST(chordMutation, getNameAfterAddNoteReflectsNewChord) {
    // The reviewer's exact repro sequence, which aborted the process before this fix.
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
    // Reviewer's Probe group F: a chord stacked once (populating _stackedHeaps for 3 notes),
    // then grown to 8 distinct pitch classes, used to keep the stale 3-note heap and skip the
    // "no valid heap" guard entirely -- unlike an identical freshly-built 8-note chord, which
    // correctly threw.
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
    // Reviewer's Probe group E: a chord stacked once, then shrunk via removeNote(), used to
    // index _originalNotes with a heap size from before the shrink -- past the end of the
    // (now smaller) vector -- degrading into "THREW: string too long" (std::length_error from
    // garbage) instead of a correct, deterministic answer.
    Chord myChord({"C4", "E4", "G4", "B4", "D5"});  // C major ninth
    myChord.getName();  // populate the stack cache at size 5

    myChord.removeNote(4);  // remove D5
    myChord.removeNote(3);  // remove B4
    EXPECT_EQ(myChord.size(), 3);

    EXPECT_EQ(myChord.getName(), "C");
}

TEST(chordMutation, getCloseStackIntervalsAfterGrowingMatchesNewSize) {
    // The specific live out-of-bounds read the re-review demonstrated: getCloseStackIntervals()
    // bounds its loop by stackSize() (i.e. _openStack.size()) but indexes _closeStack. Before
    // this fix, a stale, undersized _closeStack (left behind by the _stackedHeaps bug) made this
    // read past the end of _closeStack once the chord had grown.
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
    // getNote(-1) on an empty chord now throws instead of reading out of bounds; operator<< must
    // not let that propagate, since gtest itself calls operator<< to format values in failure
    // messages, and a throwing stream operator can turn a clean test failure into a process
    // abort. Mirrors __repr__'s "[]" for an empty chord.
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
// Final review found a real, Python-reachable out-of-bounds WRITE (more severe than the reads
// this branch had fixed so far): setDuration() indexed both '_originalNotes' and '_openStack'
// by '_originalNotes.size()', but stackInThirds() dedups '_openStack' down to one note per
// unique pitch class, so it can be strictly smaller. Fixed by bounding each container's loop by
// its own size instead of assuming they match.
// ====================

TEST(setDuration, floatOverloadSafeAfterPitchClassDedupShrinksOpenStack) {
    // Exact repro from the final review: Chord(["C4","C5"]); getName(); setDuration(...).
    // A typed intermediate vector (not a bare 2-element braced-init-list) sidesteps a
    // pre-existing Chord(vector<Note>) / Chord(vector<string>) overload-resolution ambiguity
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
// Final review found these three had no bounds checks at all, despite sitting inside the very
// functions round 2 (4158b63) edited to add cache invalidation: removeTopNote() popped an empty
// vector (UB), insertNote() inserted at an unchecked index (UB for noteIndex < 0 or > size()),
// removeNote() erased at an unchecked index (UB for noteIndex < 0 or >= size()). All three are
// Python-bound. Guarded with LOG_ERROR, matching this file's own convention (e.g. getNote).
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

#include <gtest/gtest.h>

#include "maiacore/helper.h"
#include "maiacore/note.h"
#include "maiacore/pitch.h"
#include "quarter-tone-characterization-data.h"

TEST(Characterization, semitoneBehaviourIsUnchanged) {
    for (const auto& e : kSemitoneCharTable) {
        EXPECT_EQ(Helper::pitch2midiNote(e.pitch), e.midiNumber) << e.pitch;
        EXPECT_EQ(Note(e.pitch).getEnharmonicPitch(false), e.enharmonicDefault) << e.pitch;
    }
}

TEST(Pitch, quarterToneStringRoundTrip) {
    Pitch p("C1x4");
    EXPECT_EQ(p.getPitchStep(), "C");
    EXPECT_FLOAT_EQ(p.getAlter(), 0.5f);
    EXPECT_EQ(p.getOctave().value(), 4);
    EXPECT_EQ(p.getPitch(), "C1x4");
    EXPECT_EQ(p.getMidiNumber(), 61);          // ties round up
    EXPECT_FLOAT_EQ(p.getQuarterToneSteps(), 60.5f);
}

TEST(Pitch, restHasNoOctave) {
    Pitch r("rest");
    EXPECT_TRUE(r.isRest());
    EXPECT_FALSE(r.getOctave().has_value());
    EXPECT_EQ(r.getMidiNumber(), MUSIC_XML::MIDI::NUMBER::MIDI_REST);
    EXPECT_EQ(r.getPitch(), "rest");
}

TEST(Pitch, setAlterRejectsNonMultipleOfHalf) {
    Pitch p("C4");
    EXPECT_THROW(p.setAlter(0.3f), std::runtime_error);
}

TEST(Pitch, roundToSemitoneTiesUp) {
    Pitch p("C1x4");
    p.roundToSemitone();
    EXPECT_EQ(p.getPitch(), "C#4");
}

// CONTROLLER ADDITION — flat-side cases, which the original brief lacked.
// Every test above uses a positive alter, where the two candidate rounding
// rules agree; only these distinguish them. Task 4 shipped a rounding bug
// past review for precisely this reason.
TEST(Pitch, midiNumberRoundsHalfUpOnFlatSide) {
    EXPECT_EQ(Pitch("D1b4").getMidiNumber(), 62);   // 61.5 -> 62, not 61
    EXPECT_EQ(Pitch("D3b4").getMidiNumber(), 61);   // 60.5 -> 61, not 60
    EXPECT_EQ(Pitch("C1x-1").getMidiNumber(), 1);   // negative octave, sharp side
    EXPECT_FLOAT_EQ(Pitch("D1b4").getQuarterToneSteps(), 61.5f);
}

// REVIEW ROUND 1 (task-2-review.md) — I1: roundToSemitone() is a second, independent
// implementation of the ties-upward rule (pitch.cpp's own std::floor(_alter + 0.5f), not a
// delegation to Helper::spelling2midiNote), and the sharp-side-only test above cannot
// discriminate it from ties-away-from-zero. These flat-side cases do: reverting
// roundToSemitone() to `_alter = std::round(_alter);` fails both EXPECT_EQ below.
TEST(Pitch, roundToSemitoneTiesUpFlatSide) {
    Pitch p("D1b4");  // alter -0.5
    p.roundToSemitone();
    EXPECT_EQ(p.getPitch(), "D4");  // floor(-0.5 + 0.5) == 0; std::round(-0.5) would give "Db4"

    Pitch q("D3b4");  // alter -1.5
    q.roundToSemitone();
    EXPECT_EQ(q.getPitch(), "Db4");  // floor(-1.5 + 0.5) == -1; std::round(-1.5) would give "Dbb4"
}

// REVIEW ROUND 2 (task-2-review.md C1, superseded by the user's split-policy ruling) — a rest
// refuses setAlter() as a boundary condition, not a caller error: it must NOT throw, must warn
// (LOG_WARN), and must leave the rest exactly as it was. Before this fix, setAlter() had no
// rest guard at all, and this exact sequence produced isRest() == true with
// getAlterSymbol() == "1x" and getPitchClass() == "rest1x" while getPitch() still reported
// "rest" — a state spec section 4.4's table says cannot exist. Test name matches the user's
// ruling verbatim. Removing the isRest() guard in Pitch::setAlter() makes this fail: the alter
// would become 0.5f and getPitchClass() would become "rest1x".
TEST(Pitch, setAlterOnRestIsRefusedAndWarns) {
    Pitch r("rest");
    r.setAlter(0.5f);  // must not throw
    EXPECT_TRUE(r.isRest());
    EXPECT_FLOAT_EQ(r.getAlter(), 0.0f);
    EXPECT_EQ(r.getPitchClass(), "rest");  // not "rest1x"
}

// REVIEW ROUND 2 — the user's ruling: setStep() is PERMISSIVE on a rest, resurrecting it into
// a note with the octave defaulted to 4 (this is the original round-1 behaviour, restored).
// Reverting Pitch::setStep()'s isRest() branch to refuse instead (as round 1 briefly required)
// fails this.
TEST(Pitch, setStepResurrectsRestToOctave4) {
    Pitch r("rest");
    r.setStep("D");
    EXPECT_FALSE(r.isRest());
    EXPECT_EQ(r.getPitch(), "D4");
    EXPECT_EQ(r.getOctave().value(), 4);
}

// REVIEW ROUND 2 — setOctave() refuses on a rest as a boundary condition: must not throw, must
// warn, must leave the rest unchanged. Removing the isRest() guard in Pitch::setOctave() fails
// this (the rest would gain an octave of 4).
TEST(Pitch, setOctaveOnRestIsRefusedAndWarns) {
    Pitch r("rest");
    r.setOctave(4);  // must not throw
    EXPECT_TRUE(r.isRest());
    EXPECT_FALSE(r.getOctave().has_value());
}

// REVIEW ROUND 1 — I2: setStep() had no test at all. Reverting Pitch::setStep() to not
// assign _step (a no-op stub) fails this.
TEST(Pitch, setStepValidTransitionKeepsOctave) {
    Pitch p("C4");
    p.setStep("D");
    EXPECT_EQ(p.getPitch(), "D4");
    EXPECT_EQ(p.getOctave().value(), 4);
}

// REVIEW ROUND 1 — I2. Reverting the isValidStep check in Pitch::setStep() fails this (the
// invalid step would otherwise reach Helper::spelling2midiNote() and throw a different,
// unintended message, or in principle be accepted).
TEST(Pitch, setStepRejectsInvalidStep) {
    Pitch p("C4");
    EXPECT_THROW(p.setStep("H"), std::runtime_error);
    EXPECT_EQ(p.getPitchStep(), "C");  // unchanged
}

// REVIEW ROUND 2 — C2's MIDI floor check, re-scoped by the user's ruling to warn-and-ignore
// rather than throw: it is a boundary condition, not a caller error. Removing the
// Helper::spelling2midiNote(...) < 0 check in Pitch::setStep() fails this (the step would
// become "C" and p.getMidiNumber() would return -1, colliding with
// MUSIC_XML::MIDI::NUMBER::MIDI_REST, for a non-rest Pitch).
TEST(Pitch, setStepBelowMidiZeroIsRefusedAndWarns) {
    Pitch p("Db-1");  // MIDI 1: D(2) + alter(-1) at octave -1
    p.setStep("C");   // must not throw; C(0) + (-1) at octave -1 == -1
    EXPECT_EQ(p.getPitchStep(), "D");  // unchanged
    EXPECT_EQ(p.getMidiNumber(), 1);
}

// REVIEW ROUND 1 — I2: setOctave()'s success path had no test.
TEST(Pitch, setOctaveValidTransition) {
    Pitch p("C4");
    p.setOctave(5);
    EXPECT_EQ(p.getPitch(), "C5");
}

// REVIEW ROUND 1 — I2. Reverting the range check in Pitch::setOctave() fails this.
TEST(Pitch, setOctaveRejectsOutOfRange) {
    Pitch p("C4");
    EXPECT_THROW(p.setOctave(12), std::runtime_error);
    EXPECT_EQ(p.getOctave().value(), 4);  // unchanged
}

// REVIEW ROUND 2 — C2's own example, re-scoped by the user's ruling to warn-and-ignore:
// Pitch("Cbb4").setOctave(-1) used to leave getMidiNumber() == -2 (round 1: it threw instead,
// which was also wrong per the user's later ruling). Neither may happen: it must warn and leave
// the object unchanged. Removing the Helper::spelling2midiNote(...) < 0 check in
// Pitch::setOctave() fails this.
TEST(Pitch, setOctaveBelowMidiZeroIsRefusedAndWarns) {
    Pitch p("Cbb4");   // valid: MIDI 58
    p.setOctave(-1);   // must not throw; C(0) + (-2) at octave -1 == -2
    EXPECT_EQ(p.getOctave().value(), 4);  // unchanged
    EXPECT_EQ(p.getMidiNumber(), 58);
}

// REVIEW ROUND 1 — I2: setAlter()'s [-2, 2] range branch had no test; only the
// multiple-of-0.5 branch was exercised.
TEST(Pitch, setAlterRejectsOutOfRange) {
    Pitch p("C4");
    EXPECT_THROW(p.setAlter(2.5f), std::runtime_error);
    EXPECT_FLOAT_EQ(p.getAlter(), 0.0f);  // unchanged
}

// REVIEW ROUND 2 — C2's own example, re-scoped by the user's ruling: must not throw, must
// leave the object untouched. Pitch("C-1").setAlter(-1.0f) used to leave isRest() == false
// with getMidiNumber() == -1 == MUSIC_XML::MIDI::NUMBER::MIDI_REST, directly falsifying the
// premise spec section 4.4 relies on for that sentinel (round 1's throwing fix was also
// superseded). Test name and body match the user's ruling verbatim. Removing the
// Helper::spelling2midiNote(...) < 0 check in Pitch::setAlter() fails this.
TEST(Pitch, setAlterBelowMidiZeroIsRefusedAndWarns) {
    Pitch p("C-1");
    p.setAlter(-1.0f);  // must not throw
    EXPECT_FLOAT_EQ(p.getAlter(), 0.0f);  // unchanged
    EXPECT_EQ(p.getMidiNumber(), 0);      // still C-1
    EXPECT_FALSE(p.isRest());
}

// REVIEW ROUND 1 — I2, the review's own example: setPitchClass()'s octave-preservation could
// be deleted (replaced with `setPitch(pitchClass)`) and the suite stayed green. Reverting
// Pitch::setPitchClass() to `setPitch(pitchClass);` fails this (octave would reset to the
// splitPitch() default of 4 instead of staying 5).
TEST(Pitch, setPitchClassPreservesOctave) {
    Pitch p("C5");
    p.setPitchClass("D1x");
    EXPECT_EQ(p.getPitch(), "D1x5");
}

// REVIEW ROUND 1 — I2: setPitchClass()'s rest-transition path had no test.
TEST(Pitch, setPitchClassCanBecomeRest) {
    Pitch p("C4");
    p.setPitchClass("rest");
    EXPECT_TRUE(p.isRest());
    EXPECT_EQ(p.getPitch(), "rest");
}

// REVIEW ROUND 1 — I2: setMidiNumber() had no test at all.
TEST(Pitch, setMidiNumberRoundTrip) {
    Pitch p("rest");
    p.setMidiNumber(61);
    EXPECT_EQ(p.getPitch(), "C#4");
    EXPECT_EQ(p.getMidiNumber(), 61);
}

// REVIEW ROUND 1 — I2: setMidiNumber()'s rest-transition path had no test.
TEST(Pitch, setMidiNumberNegativeIsRest) {
    Pitch p("C4");
    p.setMidiNumber(-1);
    EXPECT_TRUE(p.isRest());
}

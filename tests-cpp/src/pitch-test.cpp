#include <gtest/gtest.h>

#include <cmath>
#include <limits>

#include "maiacore/config.h"
#include "maiacore/helper.h"
#include "maiacore/note.h"
#include "maiacore/pitch.h"
#include "quarter-tone-characterization-data.h"
#include "test-capture.h"

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

// Task 6b, section K: the rest check must run BEFORE the range check. Before this reordering, a
// rest handed an OUT-OF-RANGE octave hit the throwing range check first and threw instead of
// warning (setOctaveOnRestIsRefusedAndWarns above only exercises an IN-range octave (4), which
// never reached the range check to begin with, so it could not catch this). -2 is used
// deliberately: it is the exact value Note::getOctave() used to answer for a rest before Task
// 6b, so this closes fix round 5's finding F2 round-trip hazard at the Pitch layer too.
// Reverting the check order in Pitch::setOctave() fails this.
TEST(Pitch, setOctaveOutOfRangeOnRestIsRefusedAndWarnsNotThrows) {
    Pitch r("rest");
    EXPECT_NO_THROW(r.setOctave(-2));
    EXPECT_TRUE(r.isRest());
    EXPECT_FALSE(r.getOctave().has_value());

    EXPECT_NO_THROW(r.setOctave(12));
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

// ===== TASK 3 — frequency, and the MIDI/frequency constructors ===== //

// REVIEW ROUND 4 (task-3-review.md F2) — moved here, ahead of every other Task 3 test, from its
// original position after fromExtremeFiniteFrequencyClampsToCeiling further down this file
// (round 3, item 2). It was registered *after* two tests (fromInfinityFrequencyClampsToCeiling...
// and fromExtremeFiniteFrequencyClampsToCeiling) that hang under the exact regression this test
// pins -- a broken clampToRepresentableMidi() -- so in a full-suite run the binary never reached
// it: it could only discriminate that regression under --gtest_filter, a cousin of the "coverage
// that cannot deliver a verdict when it counts" problem this project has hit before. This is a
// pure static-method test with no dependency on tuning-system state or anything else in this
// file, so placing it first costs nothing and guarantees it always gets to report a result.
//
// REVIEW ROUND 3 (task-3-review.md item 2) — the deterministic replacement for the timing
// assertion round 2 used: clampToRepresentableMidi() is the exact function setFrequency() calls
// for both ends of its range clamp (there is no separate copy of this logic inside setFrequency()
// any more), so asserting its return value directly proves the same clamp setFrequency() applies,
// without going through the frequency-to-steps pipeline or timing anything.
TEST(Pitch, clampToRepresentableMidiClampsAnyMidiToTheRepresentableRange) {
    EXPECT_EQ(Pitch::clampToRepresentableMidi(1000000), 157);  // "Bx11", the documented ceiling
    EXPECT_EQ(Pitch::clampToRepresentableMidi(157), 157);      // already at the ceiling
    EXPECT_EQ(Pitch::clampToRepresentableMidi(156), 156);      // unchanged, in range
    EXPECT_EQ(Pitch::clampToRepresentableMidi(0), 0);          // already at the floor
    EXPECT_EQ(Pitch::clampToRepresentableMidi(-1000000), 0);   // "C-1", the floor
}

// task-3-brief.md Step 1, verbatim. Resets the global tuning system first: config-test.cpp
// (linked into the same binary) leaves it changed, and getFrequency() reads that global state.
TEST(Pitch, frequencyOfA4) {
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_NEAR(Pitch("A4").getFrequency(), 440.0f, 0.01f);
}

// REVIEW ROUND 1 (task-3-review.md M5) — replaces the original 444 Hz version. The
// orchestrator's own brief was wrong: 444 Hz's step position is 69.157, and the A4/A1x4 midpoint
// is 69.25 (446.4 Hz), so 444 Hz rounds to "A4" under *both* granularities and could not tell
// them apart. 69.4 actually straddles the boundary with real margin on both sides (0.1 from the
// semitone tie at 69.5, 0.15 from the quarter-tone tie at 69.25): under the default (semitone)
// granularity it rounds down to "A4"; enableQuarterToneRoundDefaultsToFalse below pins that this
// really is the default by also checking the quarter-tone-enabled result at the same frequency.
TEST(Pitch, fromFrequencyRoundsToSemitoneByDefault) {
    const float freq = 440.0f * std::pow(2.0f, 0.4f / 12.0f);
    Pitch p(freq);
    EXPECT_EQ(p.getPitch(), "A4");
}

// task-3-brief.md Step 1, verbatim.
TEST(Pitch, fromFrequencyRoundsToQuarterToneWhenEnabled) {
    Pitch p(449.0f, "#", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "A1x4");
}

// REVIEW ROUND 1 (task-3-review.md M5) — no test pinned enableQuarterToneRound's *default*
// value on its own terms; 449 Hz already proves the flag-true side ("A1x4", above). Reusing the
// same frequency without the flag proves the default really is false ("A4"), independent of
// fromFrequencyRoundsToSemitoneByDefault's own frequency.
TEST(Pitch, enableQuarterToneRoundDefaultsToFalse) {
    EXPECT_EQ(Pitch(449.0f).getPitch(), "A4");
}

// task-3-brief.md Step 1, verbatim.
TEST(Pitch, nonPositiveFrequencyIsARest) {
    EXPECT_TRUE(Pitch(0.0f).isRest());
    EXPECT_TRUE(Pitch(-5.0f).isRest());
}

// REVIEW ROUND 2 (task-3-review.md N4) — renamed from fromFrequencyRoundsHalfUpOnFlatSide: that
// name claimed a rounding-rule discrimination this test does not perform (round 1's I3 finding,
// see pitch.cpp's setFrequency() for why no frequency-based test can). Not vacuous regardless --
// it fails under round 1's own granularity-ternary-swap and finalAlter-hardcode reverts -- but
// what it actually pins is narrower: this exact boundary (steps == -0.5) resolves to "C-1", not
// a rest, whichever rounding rule produced it.
TEST(Pitch, fromFrequencyAtFloorBoundaryIsNotARest) {
    const float freq = static_cast<float>(440.0 * std::pow(2.0, (-0.5 - 69.0) / 12.0));
    Pitch p(freq);
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "C-1");
    EXPECT_EQ(p.getMidiNumber(), 0);
}

// REVIEW ROUND 1 (task-3-review.md C1) — the general case, distinct from the exact-tie tests
// above (which, as their own comments explain, resolve directly via ties-up without ever
// reaching the clamp) and from the exact-rescue case below (whose residual is exactly 0.5). A
// frequency clearly, not just marginally, below the representable floor -- 1 Hz, steps roughly
// -36 -- must clamp to the lowest representable pitch (C-1) and warn, never a silent rest
// indistinguishable from the one rest case spec section 4.3 sanctions (freq <= 0). This is the
// only test that actually exercises the floor-clamp branch under the *correct* implementation.
TEST(Pitch, fromFrequencyClampsBelowFloorInsteadOfSilentRest) {
    Pitch p(1.0f);
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "C-1");
    EXPECT_EQ(p.getMidiNumber(), 0);
}

// REVIEW ROUND 1 (task-3-review.md C1) — the controller's specified sharpest reproducer.
// Pitch("C1b-1") is constructible: its exact position (steps -0.5) still satisfies
// Helper::spelling2midiNote(...) >= 0 via ties-upward rounding to MIDI 0. Round-tripping its own
// frequency through setFrequency() with the quarter-tone flag enabled must recover it exactly --
// not the silent rest the unchecked negative baseMidi used to produce (Helper::midiNote2pitch()
// returns the *string* "rest" for a negative MIDI number rather than throwing, so the old
// try/catch around it never fired).
TEST(Pitch, fromFrequencyRecoversExactQuarterToneAtFloor) {
    Pitch original("C1b-1");
    const float freq = original.getFrequency();
    Pitch recovered(freq, "", 440.0f, true);
    EXPECT_FALSE(recovered.isRest());
    EXPECT_EQ(recovered.getPitch(), "C1b-1");
}

// REVIEW ROUND 1 (task-3-review.md M6, the ruling's ceiling mirror of C1) — a frequency whose
// rounded step position overflows every spelling's octave range must clamp to the highest
// representable pitch and warn, never throw. "Bx11" (steps 157, the class's own documented
// midiNote2pitch() example) is the sharpest case: its own default-accType round-trip overflows
// to octave 12 ("C12") since only accType "x" specifically reaches octave 11 there, exercising
// the clamp-and-retry loop two full steps down to "B11".
TEST(Pitch, fromFrequencyClampsAboveCeilingInsteadOfThrowing) {
    Pitch original("Bx11");
    const float freq = original.getFrequency();
    Pitch recovered(freq);  // default accType: natural spelling alone cannot reach "Bx11"
    EXPECT_FALSE(recovered.isRest());
    EXPECT_EQ(recovered.getPitch(), "B11");
}

// REVIEW ROUND 1 (task-3-review.md C2) — accType "x" (double sharp, alter +2.0) combined with a
// +0.5 quarter-tone residual needs alter +2.5, which Helper::alterValue2symbol() cannot express
// (outside this class's own [-2, 2] invariant) and which used to escape as an uncaught throw
// from a call the old code's try/catch did not wrap. accType is a preference, not a demand: this
// must fall back to the default spelling instead of throwing.
TEST(Pitch, fromFrequencyFallsBackWhenAccTypeCannotExpressResidual) {
    const float freq = static_cast<float>(440.0 * std::pow(2.0, (62.5 - 69.0) / 12.0));
    Pitch p(freq, "x", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "D1x4");
}

// REVIEW ROUND 2 (task-3-review.md N1) — the controller's confirmed reproducer: the C2 fallback
// above used to call Helper::midiNote2pitch(baseMidi, {}) directly instead of through the shared
// clamp-and-retry helper, so at a baseMidi only reachable with a specific accType (157, "Bx11")
// the default spelling ALSO failed (octave 12 overflow), Helper::splitPitch("") took its
// empty-string rest branch, and a valid positive frequency became a rest -- the exact C1 defect
// reopened at the ceiling by the fix that closed it at the floor. Must now clamp to "B11" (the
// same result fromFrequencyClampsAboveCeilingInsteadOfThrowing reaches from the natural-spelling
// side), never a rest.
TEST(Pitch, fromFrequencyClampsAboveCeilingWhenAccTypeAlsoOverflowsAlter) {
    Pitch p(73038.0f, "x", 440.0f, true);
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "B11");
}

// REVIEW ROUND 2 (task-3-review.md N2) — a non-finite frequency must never reach
// static_cast<int>(std::floor(...)) in setFrequency(): that cast is undefined behaviour for
// non-finite input, measured (not assumed) to silently return INT_MIN on x86-64/MSVC -- masked
// into a wrong-end-of-range "C-1" by the floor clamp -- and to saturate to INT_MAX on AArch64
// (Apple Silicon; this library targets macOS), where the ceiling clamp would need on the order
// of 2^31 iterations: a hang, not a wrong answer. +infinity genuinely lies above the
// representable range and must clamp to the ceiling, never crashing or hanging on any platform.
//
// REVIEW ROUND 3 (task-3-review.md ruling 1) — NaN is a *different* case from +infinity, split
// out below: NaN satisfies neither half of spec section 4.3's dichotomy ("<= 0" or "positive")
// and is a caller error (LOG_ERROR), not a boundary condition. This test previously asserted NaN
// also clamped to "B11"; that assertion moved to fromNanFrequencyThrows below when the ruling
// changed NaN's behaviour.
TEST(Pitch, fromInfinityFrequencyClampsToCeilingInsteadOfUndefinedBehavior) {
    Pitch p(std::numeric_limits<float>::infinity());
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "B11");
}

// REVIEW ROUND 3 (task-3-review.md ruling 1) — NaN is unordered under IEEE 754 (every comparison
// against it, including frequency <= 0.0f, is false), so it satisfies neither of spec section
// 4.3's two cases ("<= 0" or "positive"). Fabricating a pitch from it -- as round 2 did, clamping
// it to "B11" alongside +infinity -- would hand a caller a valid-looking result and a warning
// buried in the log for what is actually a caller error (e.g. an FFT result divided by zero).
// Must throw, on the same footing as a malformed accType or an unimplemented tuning system.
TEST(Pitch, fromNanFrequencyThrows) {
    const float nanFrequency = std::numeric_limits<float>::quiet_NaN();
    EXPECT_THROW(Pitch p(nanFrequency), std::runtime_error);
}

// REVIEW ROUND 2 (task-3-review.md N3) — FLT_MAX is finite, so it survives the NaN/infinity
// handling above, but an unbounded baseMidi computed from it must still clamp to the ceiling
// without walking down from an astronomically large starting value (~1345 iterations, each a
// failed, cpptrace-resolved spelling attempt, for this specific input -- measured at 9033 ms
// locally, against 11 ms with the O(1) clamp).
//
// REVIEW ROUND 3 (task-3-review.md item 2) — this used to assert on elapsed wall-clock time
// (correctness alone does not discriminate an O(1)-vs-O(n) regression: both eventually reach the
// same right answer). Replaced with the strictly better alternative the re-review suggested:
// clampToRepresentableMidi() is now a pure, directly-testable function
// (clampToRepresentableMidiClampsAnyMidiToTheRepresentableRange, moved to the top of this Task 3
// section in round 4 -- see F2 there for why), and since setFrequency() calls that exact function
// for its own clamp (not a parallel copy of the logic), testing it pins the same regression with
// no clock, no shared-machine sensitivity, and no flaky margin. This test now checks only
// correctness at the end-to-end level. (Round 4, F1, also bounds this method's walk-down loop
// independently, so even a badly broken clamp can no longer make this specific test hang.)
TEST(Pitch, fromExtremeFiniteFrequencyClampsToCeiling) {
    Pitch p(std::numeric_limits<float>::max());
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "B11");
}

// REVIEW ROUND 2 (task-3-review.md, credited to the re-reviewer) — pins that IEEE 754 negative
// zero is already, correctly, treated as <= 0.0f (a rest), same as positive zero
// (nonPositiveFrequencyIsARest above): -0.0f == 0.0f under IEEE 754 comparison, so this needed
// no code change, only a regression test recording that it was checked.
TEST(Pitch, fromNegativeZeroFrequencyIsARest) { EXPECT_TRUE(Pitch(-0.0f).isRest()); }

// REVIEW ROUND 1 (task-3-review.md I2) — a malformed accType is a caller error, matching
// Pitch(int, accType) (which already throws for this via Helper::midiNote2pitch()'s own
// validation), not something the accType-applicability fallback should silently absorb.
// Previously this constructed "C#4" without complaint.
TEST(Pitch, setFrequencyRejectsMalformedAccType) {
    EXPECT_THROW(Pitch(277.18f, "garbage"), std::runtime_error);
}

// REVIEW ROUND 1 (task-3-review.md I1) — setFrequency() must guard the tuning system exactly
// like getFrequency() does (see getFrequencyThrowsForNonEqualTemperament below), not silently
// apply the 12-TET inverse while the getter refuses to use a tuning system it does not
// implement. Resets EQUAL_TEMPERAMENT afterward regardless of whether EXPECT_THROW passes.
TEST(Pitch, setFrequencyThrowsForNonEqualTemperament) {
    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
    EXPECT_THROW(Pitch(277.18f), std::runtime_error);
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
}

// TASK 3 ADDITION — a flat-side quarter-tone spelling case. Every other frequency-rounding test
// above requests (or defaults to) a sharp/natural spelling; this is the only one that exercises
// accType == "b" landing on a genuine quarter-tone (residual 0.5) result, proving setFrequency()
// honours the caller's accidental preference for the base semitone (here G#4/Ab4, one semitone
// below A4) rather than always falling back to the natural/sharp default.
TEST(Pitch, fromFrequencyQuarterToneFlatSide) {
    const float freq = 440.0f * std::pow(2.0f, (68.5f - 69.0f) / 12.0f);
    Pitch p(freq, "b", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "A1b4");
    EXPECT_EQ(p.getMidiNumber(), 69);  // ties upward, matching the D1b4 precedent above
}

// TASK 3 ADDITION — THE TRAP in task-3-brief.md: Helper::freq2midiNote() rounds to an integer
// MIDI number before reporting a deviation, which is exactly the bug class this sub-project
// exists to remove. This proves getFrequency() reads the exact getQuarterToneSteps() position
// instead: a quarter-tone pitch's frequency must differ from both of its neighboring semitones'
// frequencies, not silently collapse onto one of them.
TEST(Pitch, getFrequencyUsesExactQuarterToneSteps) {
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);  // see frequencyOfA4's comment above
    Pitch p("C1x4");  // quarter-tone steps 60.5 (see quarterToneStringRoundTrip above)
    const float expected = 440.0f * std::pow(2.0f, (60.5f - 69.0f) / 12.0f);
    EXPECT_NEAR(p.getFrequency(), expected, 0.01f);
    EXPECT_NE(p.getFrequency(), Pitch("C4").getFrequency());   // midi 60
    EXPECT_NE(p.getFrequency(), Pitch("C#4").getFrequency());  // midi 61, ties-up neighbor
}

// TASK 3 ADDITION — the non-EQUAL_TEMPERAMENT caller-error case task-3-brief.md's Step 3
// requires. Resets the global tuning system back to EQUAL_TEMPERAMENT afterward (config.h's
// getTuningSystem()/setTuningSystem() are process-global state) so no later test observes it
// changed, regardless of whether EXPECT_THROW above passes or fails.
TEST(Pitch, getFrequencyThrowsForNonEqualTemperament) {
    setTuningSystem(TuningSystem::JUST_INTONATION);
    EXPECT_THROW(Pitch("A4").getFrequency(), std::runtime_error);
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
}

// TASK 3 ADDITION — Pitch(int, accType) had no test: this proves accType is actually forwarded
// to Helper::midiNote2pitch() (spelling MIDI 61 as "Db4"), not silently dropped in favour of the
// default spelling ("C#4"), and that a negative midiNumber constructs a rest like
// setMidiNumber() does.
TEST(Pitch, midiNumberConstructorHonoursAccType) {
    EXPECT_EQ(Pitch(61, "b").getPitch(), "Db4");
    EXPECT_EQ(Pitch(61, "#").getPitch(), "C#4");
    EXPECT_TRUE(Pitch(-1).isRest());
}

// ===== setAlter(): a malformed alter is rejected, a valid one is stored canonically ===== //

// NaN passes both a tolerance test and a range test, since every comparison with it is false, and
// an infinity is merely "out of range"; both are rejected as what they are, and the pitch is left
// unchanged.
TEST(PitchSetAlter, rejectsNonFiniteValues) {
    for (const float alter : {std::numeric_limits<float>::quiet_NaN(),
                              std::numeric_limits<float>::infinity(),
                              -std::numeric_limits<float>::infinity()}) {
        Pitch p("C4");
        const std::string message = thrownFirstLine([&] { p.setAlter(alter); });
        EXPECT_NE(message.find("must be a finite number"), std::string::npos)
            << "alter " << alter << ": " << message;
        EXPECT_EQ(p.getPitch(), "C4");
    }
}

// Grid membership is exact: a value near a multiple of 0.5 is rejected, never stored as it is
// and never rounded onto the grid.
TEST(PitchSetAlter, rejectsAValueNearTheGrid) {
    for (const float alter : {0.99996f, 1.00004f, 0.46f}) {
        Pitch p("C4");
        const std::string message = thrownFirstLine([&] { p.setAlter(alter); });
        EXPECT_NE(message.find("multiple of 0.5"), std::string::npos)
            << "alter " << alter << ": " << message;
        EXPECT_EQ(p.getAlter(), 0.0f);
    }
}

// -0.0 is a natural, stored as the one value +0.0.
TEST(PitchSetAlter, storesNegativeZeroAsPositiveZero) {
    Pitch p("C1x4");
    p.setAlter(-0.0f);
    EXPECT_FALSE(std::signbit(p.getAlter()));
    EXPECT_EQ(p.getPitch(), "C4");
    EXPECT_EQ(p.getAlterSymbol(), "");
}

// ===== freqA4: a finite number of Hz greater than 0, checked before any arithmetic ===== //

TEST(PitchReferenceFrequency, fromFrequencyRejectsAnInvalidFreqA4) {
    for (const float freqA4 : {0.0f, -440.0f, std::numeric_limits<float>::quiet_NaN(),
                               std::numeric_limits<float>::infinity(),
                               -std::numeric_limits<float>::infinity()}) {
        const std::string message = thrownFirstLine([&] { Pitch p(440.0f, "", freqA4); });
        EXPECT_NE(message.find("reference frequency freqA4"), std::string::npos)
            << "freqA4 " << freqA4 << ": " << message;

        // Checked first, whatever the frequency: also for one that would make a rest.
        const std::string restMessage = thrownFirstLine([&] { Pitch p(0.0f, "", freqA4); });
        EXPECT_NE(restMessage.find("reference frequency freqA4"), std::string::npos)
            << "freqA4 " << freqA4 << ": " << restMessage;
    }
}

TEST(PitchReferenceFrequency, setFrequencyRejectsAnInvalidFreqA4AndLeavesThePitch) {
    Pitch p("C4");
    const std::string message = thrownFirstLine([&] { p.setFrequency(440.0f, "", 0.0f); });
    EXPECT_NE(message.find("reference frequency freqA4"), std::string::npos) << message;
    EXPECT_EQ(p.getPitch(), "C4");
}

TEST(PitchReferenceFrequency, getFrequencyRejectsAnInvalidFreqA4) {
    for (const float freqA4 : {0.0f, -440.0f, std::numeric_limits<float>::quiet_NaN(),
                               std::numeric_limits<float>::infinity()}) {
        const std::string message = thrownFirstLine([&] { Pitch("A4").getFrequency(freqA4); });
        EXPECT_NE(message.find("reference frequency freqA4"), std::string::npos)
            << "freqA4 " << freqA4 << ": " << message;

        const std::string restMessage =
            thrownFirstLine([&] { Pitch("rest").getFrequency(freqA4); });
        EXPECT_NE(restMessage.find("reference frequency freqA4"), std::string::npos)
            << "freqA4 " << freqA4 << ": " << restMessage;
    }
    EXPECT_FLOAT_EQ(Pitch("A4").getFrequency(442.0f), 442.0f);
}

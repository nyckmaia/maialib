#include "maiacore/pitch.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/config.h"
#include "maiacore/helper.h"
#include "maiacore/note.h"
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

// Flat-side cases. Every test above uses a positive alter, where rounding ties upward and
// rounding ties away from zero agree; only a negative alter tells them apart.
TEST(Pitch, midiNumberRoundsHalfUpOnFlatSide) {
    EXPECT_EQ(Pitch("D1b4").getMidiNumber(), 62);   // 61.5 -> 62, not 61
    EXPECT_EQ(Pitch("D3b4").getMidiNumber(), 61);   // 60.5 -> 61, not 60
    EXPECT_EQ(Pitch("C1x-1").getMidiNumber(), 1);   // negative octave, sharp side
    EXPECT_FLOAT_EQ(Pitch("D1b4").getQuarterToneSteps(), 61.5f);
}

// roundToSemitone() rounds the alter itself, with roundTiesUpward() (utils.h), rather than
// through Helper::spelling2midiNote(), and the sharp-side test above cannot tell ties upward from
// ties away from zero. These flat-side cases can: with `_alter = std::round(_alter);` instead,
// both EXPECT_EQ below fail.
TEST(Pitch, roundToSemitoneTiesUpFlatSide) {
    Pitch p("D1b4");  // alter -0.5
    p.roundToSemitone();
    EXPECT_EQ(p.getPitch(), "D4");  // floor(-0.5 + 0.5) == 0; std::round(-0.5) would give "Db4"

    Pitch q("D3b4");  // alter -1.5
    q.roundToSemitone();
    EXPECT_EQ(q.getPitch(), "Db4");  // floor(-1.5 + 0.5) == -1; std::round(-1.5) would give "Dbb4"
}

// A rest refuses setAlter() as a boundary condition, not a caller error: it must NOT throw, must
// warn (LOG_WARN), and must leave the rest exactly as it was. Without the isRest() guard in
// Pitch::setAlter(), this sequence would give a rest with an alter -- getAlterSymbol() "1x" and
// getPitchClass() "rest1x" while getPitch() still reports "rest" -- a state spec section 4.4's
// table says cannot exist.
TEST(Pitch, setAlterOnRestIsRefusedAndWarns) {
    Pitch r("rest");
    r.setAlter(0.5f);  // must not throw
    EXPECT_TRUE(r.isRest());
    EXPECT_FLOAT_EQ(r.getAlter(), 0.0f);
    EXPECT_EQ(r.getPitchClass(), "rest");  // not "rest1x"
}

// setStep() is PERMISSIVE on a rest, resurrecting it into a note with the octave defaulted to 4
// (see the Pitch class invariants): a setStep() that refused on a rest would fail this.
TEST(Pitch, setStepResurrectsRestToOctave4) {
    Pitch r("rest");
    r.setStep("D");
    EXPECT_FALSE(r.isRest());
    EXPECT_EQ(r.getPitch(), "D4");
    EXPECT_EQ(r.getOctave().value(), 4);
}

// setOctave() refuses on a rest as a boundary condition: it must not throw, must warn, and must
// leave the rest unchanged.
TEST(Pitch, setOctaveOnRestIsRefusedAndWarns) {
    Pitch r("rest");
    r.setOctave(4);  // must not throw
    EXPECT_TRUE(r.isRest());
    EXPECT_FALSE(r.getOctave().has_value());
}

// The rest check must run BEFORE the range check, so a rest handed an OUT-OF-RANGE octave warns
// instead of throwing. setOctaveOnRestIsRefusedAndWarns above only exercises an IN-range octave
// (4), which passes the range check either way, so it cannot tell the order; -2 and 12 are the
// nearest out-of-range octaves on each side. Reversing the check order in Pitch::setOctave()
// fails this.
TEST(Pitch, setOctaveOutOfRangeOnRestIsRefusedAndWarnsNotThrows) {
    Pitch r("rest");
    EXPECT_NO_THROW(r.setOctave(-2));
    EXPECT_TRUE(r.isRest());
    EXPECT_FALSE(r.getOctave().has_value());

    EXPECT_NO_THROW(r.setOctave(12));
    EXPECT_TRUE(r.isRest());
    EXPECT_FALSE(r.getOctave().has_value());
}

// setStep()'s success path: the step changes and the octave is kept.
TEST(Pitch, setStepValidTransitionKeepsOctave) {
    Pitch p("C4");
    p.setStep("D");
    EXPECT_EQ(p.getPitch(), "D4");
    EXPECT_EQ(p.getOctave().value(), 4);
}

// An invalid step is rejected, and the pitch is left unchanged.
TEST(Pitch, setStepRejectsInvalidStep) {
    Pitch p("C4");
    EXPECT_THROW(p.setStep("H"), std::runtime_error);
    EXPECT_EQ(p.getPitchStep(), "C");  // unchanged
}

// setStep()'s MIDI floor check warns and ignores rather than throws: moving below MIDI note 0 is
// a boundary condition, not a caller error. Without the Helper::spelling2midiNote(...) < 0 check,
// the step would become "C" and p.getMidiNumber() would return -1, colliding with
// MUSIC_XML::MIDI::NUMBER::MIDI_REST, for a non-rest Pitch.
TEST(Pitch, setStepBelowMidiZeroIsRefusedAndWarns) {
    Pitch p("Db-1");  // MIDI 1: D(2) + alter(-1) at octave -1
    p.setStep("C");   // must not throw; C(0) + (-1) at octave -1 == -1
    EXPECT_EQ(p.getPitchStep(), "D");  // unchanged
    EXPECT_EQ(p.getMidiNumber(), 1);
}

// setOctave()'s success path.
TEST(Pitch, setOctaveValidTransition) {
    Pitch p("C4");
    p.setOctave(5);
    EXPECT_EQ(p.getPitch(), "C5");
}

// An out-of-range octave is rejected, and the pitch is left unchanged.
TEST(Pitch, setOctaveRejectsOutOfRange) {
    Pitch p("C4");
    EXPECT_THROW(p.setOctave(12), std::runtime_error);
    EXPECT_EQ(p.getOctave().value(), 4);  // unchanged
}

// Moving below MIDI note 0 is a boundary condition: Pitch("Cbb4").setOctave(-1), which would
// spell MIDI -2, must neither throw nor store it. It warns and leaves the object unchanged.
TEST(Pitch, setOctaveBelowMidiZeroIsRefusedAndWarns) {
    Pitch p("Cbb4");   // valid: MIDI 58
    p.setOctave(-1);   // must not throw; C(0) + (-2) at octave -1 == -2
    EXPECT_EQ(p.getOctave().value(), 4);  // unchanged
    EXPECT_EQ(p.getMidiNumber(), 58);
}

// setAlter()'s [-2, 2] range check, separate from its multiple-of-0.5 check.
TEST(Pitch, setAlterRejectsOutOfRange) {
    Pitch p("C4");
    EXPECT_THROW(p.setAlter(2.5f), std::runtime_error);
    EXPECT_FLOAT_EQ(p.getAlter(), 0.0f);  // unchanged
}

// Moving below MIDI note 0 is a boundary condition: setAlter() must not throw and must leave the
// object untouched. Accepted, Pitch("C-1").setAlter(-1.0f) would give a non-rest with
// getMidiNumber() == -1 == MUSIC_XML::MIDI::NUMBER::MIDI_REST, falsifying the premise spec
// section 4.4 relies on for that sentinel.
TEST(Pitch, setAlterBelowMidiZeroIsRefusedAndWarns) {
    Pitch p("C-1");
    p.setAlter(-1.0f);  // must not throw
    EXPECT_FLOAT_EQ(p.getAlter(), 0.0f);  // unchanged
    EXPECT_EQ(p.getMidiNumber(), 0);      // still C-1
    EXPECT_FALSE(p.isRest());
}

// setPitchClass() keeps the octave: a plain `setPitch(pitchClass);` would reset it to the
// splitPitch() default of 4 instead of keeping 5.
TEST(Pitch, setPitchClassPreservesOctave) {
    Pitch p("C5");
    p.setPitchClass("D1x");
    EXPECT_EQ(p.getPitch(), "D1x5");
}

// setPitchClass()'s rest-transition path.
TEST(Pitch, setPitchClassCanBecomeRest) {
    Pitch p("C4");
    p.setPitchClass("rest");
    EXPECT_TRUE(p.isRest());
    EXPECT_EQ(p.getPitch(), "rest");
}

// setMidiNumber()'s success path, from a rest.
TEST(Pitch, setMidiNumberRoundTrip) {
    Pitch p("rest");
    p.setMidiNumber(61);
    EXPECT_EQ(p.getPitch(), "C#4");
    EXPECT_EQ(p.getMidiNumber(), 61);
}

// setMidiNumber()'s rest-transition path.
TEST(Pitch, setMidiNumberNegativeIsRest) {
    Pitch p("C4");
    p.setMidiNumber(-1);
    EXPECT_TRUE(p.isRest());
}

// ===== Frequency, and the MIDI/frequency constructors ===== //

// clampToRepresentableMidi() is the function setFrequency() calls for both ends of its range
// clamp (there is no separate copy of this logic inside setFrequency()), so asserting its return
// value directly pins the clamp setFrequency() applies, deterministically, without going through
// the frequency-to-steps pipeline.
TEST(Pitch, clampToRepresentableMidiClampsAnyMidiToTheRepresentableRange) {
    EXPECT_EQ(Pitch::clampToRepresentableMidi(1000000), 157);  // "Bx11", the documented ceiling
    EXPECT_EQ(Pitch::clampToRepresentableMidi(157), 157);      // already at the ceiling
    EXPECT_EQ(Pitch::clampToRepresentableMidi(156), 156);      // unchanged, in range
    EXPECT_EQ(Pitch::clampToRepresentableMidi(0), 0);          // already at the floor
    EXPECT_EQ(Pitch::clampToRepresentableMidi(-1000000), 0);   // MIDI note 0, "C-1"
}

// Resets the global tuning system first: it is process-global state, which another test in the
// same binary may have changed, and getFrequency() reads it.
TEST(Pitch, frequencyOfA4) {
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_NEAR(Pitch("A4").getFrequency(), 440.0f, 0.01f);
}

// Step position 69.4 tells the two granularities apart with margin on both sides: 0.1 from the
// semitone tie at 69.5 and 0.15 from the quarter-tone tie at 69.25. A frequency below the A4/A1x4
// midpoint, 69.25 (446.4 Hz) -- 444 Hz, at 69.157, say -- rounds to "A4" under both granularities,
// so it cannot tell them apart. Under the default semitone granularity 69.4 rounds down to "A4".
TEST(Pitch, fromFrequencyRoundsToSemitoneByDefault) {
    const float freq = 440.0f * std::pow(2.0f, 0.4f / 12.0f);
    Pitch p(freq);
    EXPECT_EQ(p.getPitch(), "A4");
}

// 449 Hz lies at step position 69.35, nearer the quarter tone A1x4 (69.5) than A4 (69).
TEST(Pitch, fromFrequencyRoundsToQuarterToneWhenEnabled) {
    Pitch p(449.0f, "#", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "A1x4");
}

// enableQuarterToneRound's default, on its own terms: 449 Hz gives "A1x4" with the flag (above)
// and "A4" without it, so the default is false.
TEST(Pitch, enableQuarterToneRoundDefaultsToFalse) {
    EXPECT_EQ(Pitch(449.0f).getPitch(), "A4");
}

TEST(Pitch, nonPositiveFrequencyIsARest) {
    EXPECT_TRUE(Pitch(0.0f).isRest());
    EXPECT_TRUE(Pitch(-5.0f).isRest());
}

// A frequency at the floor boundary (step position -0.5) resolves to a real pitch, "C-1", not a
// rest, whichever rounding rule produced it. It does not tell the rounding rules apart: no
// frequency lands exactly on a tie (see pitch.cpp's setFrequency()).
TEST(Pitch, fromFrequencyAtFloorBoundaryIsNotARest) {
    const float freq = static_cast<float>(440.0 * std::pow(2.0, (-0.5 - 69.0) / 12.0));
    Pitch p(freq);
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "C-1");
    EXPECT_EQ(p.getMidiNumber(), 0);
}

// The general below-floor case, distinct from the boundary test above and from the exact-rescue
// case below (whose residual is exactly 0.5): a frequency clearly, not just marginally, below the
// representable range -- 1 Hz, step position about -36 -- is clamped to MIDI note 0, "C-1", with
// a warning, never a silent rest indistinguishable from the one rest case spec section 4.3
// sanctions (freq <= 0).
TEST(Pitch, fromFrequencyClampsBelowFloorInsteadOfSilentRest) {
    Pitch p(1.0f);
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "C-1");
    EXPECT_EQ(p.getMidiNumber(), 0);
}

// Pitch("C1b-1") is constructible: its exact position (steps -0.5) rounds, ties upward, to MIDI 0.
// Round-tripping its own frequency through setFrequency() with the quarter-tone flag enabled must
// recover it exactly, not a silent rest: the position splits into baseMidi -1 plus a +0.5
// residual, and Helper::midiNote2pitch() answers the *string* "rest" for a negative MIDI number
// rather than throwing, so setFrequency() respells it as baseMidi 0 minus 0.5.
TEST(Pitch, fromFrequencyRecoversExactQuarterToneAtFloor) {
    Pitch original("C1b-1");
    const float freq = original.getFrequency();
    Pitch recovered(freq, "", 440.0f, true);
    EXPECT_FALSE(recovered.isRest());
    EXPECT_EQ(recovered.getPitch(), "C1b-1");
}

// The ceiling mirror of the floor case: a frequency whose rounded step position the requested
// spelling cannot reach within octave 11 is moved down to a pitch it can, with a warning, never
// a throw. "Bx11" (step 157) is the sharpest case: only accType "x" reaches it within octave 11,
// and with the default accType 157 would be "C#12" and 156 "C12", so the walk-down goes two full
// steps, to "B11".
TEST(Pitch, fromFrequencyClampsAboveCeilingInsteadOfThrowing) {
    Pitch original("Bx11");
    const float freq = original.getFrequency();
    Pitch recovered(freq);  // default accType: natural spelling alone cannot reach "Bx11"
    EXPECT_FALSE(recovered.isRest());
    EXPECT_EQ(recovered.getPitch(), "B11");
}

// accType "x" (double sharp, alter +2.0) combined with a +0.5 quarter-tone residual needs alter
// +2.5, which Helper::alterValue2symbol() cannot express (outside this class's own [-2, 2]
// invariant). accType is a preference, not a demand: this falls back to the default spelling
// instead of throwing.
TEST(Pitch, fromFrequencyFallsBackWhenAccTypeCannotExpressResidual) {
    const float freq = static_cast<float>(440.0 * std::pow(2.0, (62.5 - 69.0) / 12.0));
    Pitch p(freq, "x", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "D1x4");
}

// The accidental-type fallback above, at the ceiling: 73038 Hz with "x" and quarter-tone rounding
// lies at step position 157.5, "Bx11" plus a +0.5 residual that needs alter +2.5. The default
// spelling cannot spell 157 either (octave 12), so the fallback goes through the same walk-down
// as the natural spelling (fromFrequencyClampsAboveCeilingInsteadOfThrowing): the result is
// "B11", never a rest, which an empty spelling handed to Helper::splitPitch() would give.
TEST(Pitch, fromFrequencyClampsAboveCeilingWhenAccTypeAlsoOverflowsAlter) {
    Pitch p(73038.0f, "x", 440.0f, true);
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "B11");
}

// A non-finite frequency must never reach static_cast<int>(std::floor(...)) in setFrequency():
// that cast is undefined behaviour for non-finite input (INT_MIN on x86-64/MSVC, which the floor
// clamp would turn into a wrong-end-of-range "C-1"; INT_MAX on AArch64). +infinity genuinely lies
// above the representable range, so it is moved to the ceiling like any too-high frequency: with
// the default spelling, "B11". NaN is a different case; see fromNanFrequencyThrows below.
TEST(Pitch, fromInfinityFrequencyClampsToCeilingInsteadOfUndefinedBehavior) {
    Pitch p(std::numeric_limits<float>::infinity());
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "B11");
}

// NaN is unordered under IEEE 754 (every comparison against it, including frequency <= 0.0f, is
// false), so it satisfies neither of spec section 4.3's two cases ("<= 0" or "positive").
// Fabricating a pitch from it -- clamping it to "B11" alongside +infinity, say -- would hand a
// caller a valid-looking result and a warning buried in the log for what is actually a caller
// error (e.g. an FFT result divided by zero). It must throw, on the same footing as a malformed
// accType or an unimplemented tuning system.
TEST(Pitch, fromNanFrequencyThrows) {
    const float nanFrequency = std::numeric_limits<float>::quiet_NaN();
    EXPECT_THROW(Pitch p(nanFrequency), std::runtime_error);
}

// FLT_MAX is finite, so it passes the NaN/infinity handling above, and the step position computed
// from it lies far above the ceiling: it must be clamped to the ceiling at once, not walked down
// one failed spelling at a time from an astronomically large start. Whether the clamp is O(1) is
// pinned directly, without a clock, by clampToRepresentableMidiClampsAnyMidiToTheRepresentableRange
// above; this test checks the end-to-end result.
TEST(Pitch, fromExtremeFiniteFrequencyClampsToCeiling) {
    Pitch p(std::numeric_limits<float>::max());
    EXPECT_FALSE(p.isRest());
    EXPECT_EQ(p.getPitch(), "B11");
}

// IEEE 754 negative zero is <= 0.0f, so it is a rest, like positive zero
// (nonPositiveFrequencyIsARest above): -0.0f == 0.0f under IEEE 754 comparison.
TEST(Pitch, fromNegativeZeroFrequencyIsARest) { EXPECT_TRUE(Pitch(-0.0f).isRest()); }

// A malformed accType is a caller error, as it is for Pitch(int, accType), which throws for it
// through Helper::midiNote2pitch()'s own validation: the accType-applicability fallback must not
// absorb it into the default spelling ("C#4").
TEST(Pitch, setFrequencyRejectsMalformedAccType) {
    EXPECT_THROW(Pitch(277.18f, "garbage"), std::runtime_error);
}

// setFrequency() must guard the tuning system exactly like getFrequency() does (see
// getFrequencyThrowsForNonEqualTemperament below), not silently apply the 12-TET inverse while
// the getter refuses to use a tuning system it does not implement. EQUAL_TEMPERAMENT is restored
// before the message is checked, so no later test observes the change.
TEST(Pitch, setFrequencyThrowsForNonEqualTemperament) {
    setTuningSystem(TuningSystem::MEANTONE_TEMPERAMENT);
    const std::string message = thrownFirstLine([] { Pitch(277.18f); });
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(message,
              "[maiacore] Tuning system is not implemented; only EQUAL_TEMPERAMENT is available");
}

// A flat-side quarter-tone spelling case. Every other frequency-rounding test above requests (or
// defaults to) a sharp/natural spelling; this is the only one that exercises accType == "b"
// landing on a genuine quarter-tone (residual 0.5) result, proving setFrequency() honours the
// caller's accidental preference for the base semitone (here G#4/Ab4, one semitone below A4)
// rather than always falling back to the natural/sharp default.
TEST(Pitch, fromFrequencyQuarterToneFlatSide) {
    const float freq = 440.0f * std::pow(2.0f, (68.5f - 69.0f) / 12.0f);
    Pitch p(freq, "b", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "A1b4");
    EXPECT_EQ(p.getMidiNumber(), 69);  // ties upward, matching the D1b4 precedent above
}

// Helper::freq2midiNote() rounds to an integer MIDI number before reporting a deviation, which
// would lose a quarter tone. getFrequency() reads the exact getQuarterToneSteps() position
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

// A tuning system other than EQUAL_TEMPERAMENT is a caller error. The global tuning system is
// restored to EQUAL_TEMPERAMENT before the message is checked (config.h's
// getTuningSystem()/setTuningSystem() are process-global state), so no later test observes it
// changed.
TEST(Pitch, getFrequencyThrowsForNonEqualTemperament) {
    setTuningSystem(TuningSystem::JUST_INTONATION);
    const std::string message = thrownFirstLine([] { Pitch("A4").getFrequency(); });
    setTuningSystem(TuningSystem::EQUAL_TEMPERAMENT);
    EXPECT_EQ(message,
              "[maiacore] Tuning system is not implemented; only EQUAL_TEMPERAMENT is available");
}

// Pitch(int, accType) forwards accType to Helper::midiNote2pitch() (spelling MIDI 61 as "Db4"),
// rather than dropping it in favour of the default spelling ("C#4"), and a negative midiNumber
// constructs a rest, as setMidiNumber() does.
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
    for (const float alter :
         {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
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
    for (const float freqA4 :
         {0.0f, -440.0f, std::numeric_limits<float>::quiet_NaN(),
          std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()}) {
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

// ===== setFrequency(): an accidental type that cannot spell the result is reported ===== //

// accType is a preference, not a demand: when it cannot spell the rounded pitch, the default
// spelling is used and a warning says so, naming the accidental type. 449 Hz rounds to the quarter
// tone 69.5, whose base semitone, MIDI 69, has no "#" spelling.
TEST(PitchAccTypeFallback, warnsWhenTheAccidentalTypeCannotSpellTheRoundedPitch) {
    StdoutCapture capture;
    const Pitch p(449.0f, "#", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "A1x4");
    EXPECT_NE(
        capture.str().find("[WARN] Pitch::setFrequency: the accidental type '#' cannot spell"),
        std::string::npos)
        << capture.str();
}

// A double accidental cannot absorb a quarter-tone residual (+2.5 has no spelling): the same pitch
// is spelled the default way and reported as an accidental-type fallback -- not as a clamp, since
// nothing about the pitch itself changed.
TEST(PitchAccTypeFallback, aDoubleAccidentalThatCannotAbsorbTheResidualIsReportedAsSuch) {
    StdoutCapture capture;
    const float frequency = static_cast<float>(440.0 * std::pow(2.0, (62.5 - 69.0) / 12.0));
    const Pitch p(frequency, "x", 440.0f, true);
    EXPECT_EQ(p.getPitch(), "D1x4");
    EXPECT_NE(capture.str().find("the accidental type 'x' cannot spell"), std::string::npos)
        << capture.str();
    EXPECT_EQ(capture.str().find("could not be represented exactly"), std::string::npos)
        << capture.str();
}

TEST(PitchAccTypeFallback, anAccidentalTypeThatAppliesPrintsNothing) {
    StdoutCapture capture;
    EXPECT_EQ(Pitch(466.16f, "b").getPitch(), "Bb4");
    EXPECT_EQ(Pitch(466.16f, "#").getPitch(), "A#4");
    EXPECT_EQ(capture.str(), "");
}

// ===== Equality: the canonical (step, alter, octave) spelling ===== //

TEST(PitchEquality, comparesTheSpellingNotThePosition) {
    EXPECT_TRUE(Pitch("C1x4") == Pitch("C1x4"));
    EXPECT_FALSE(Pitch("C1x4") != Pitch("C1x4"));

    // Enharmonic spellings of one position are different pitches here.
    EXPECT_FALSE(Pitch("C#4") == Pitch("Db4"));
    EXPECT_TRUE(Pitch("C#4") != Pitch("Db4"));
    EXPECT_FALSE(Pitch("C1x4") == Pitch("D3b4"));

    // Each component on its own tells two pitches apart.
    EXPECT_TRUE(Pitch("C4") != Pitch("D4"));
    EXPECT_TRUE(Pitch("C4") != Pitch("C1x4"));
    EXPECT_TRUE(Pitch("C4") != Pitch("C5"));
}

TEST(PitchEquality, twoRestsAreEqualAndARestIsNoNote) {
    EXPECT_TRUE(Pitch() == Pitch("rest"));
    EXPECT_TRUE(Pitch("") == Pitch(-1));
    EXPECT_TRUE(Pitch("rest") != Pitch("C4"));
    EXPECT_TRUE(Pitch("C4") != Pitch("rest"));
}

// The alter is stored canonically, so a pitch reached through a setter equals the one parsed
// from its string.
TEST(PitchEquality, holdsForAPitchReachedThroughASetter) {
    Pitch p("C#4");
    p.setAlter(-0.0f);
    EXPECT_TRUE(p == Pitch("C4"));
    p.setAlter(0.5f);
    EXPECT_TRUE(p == Pitch("C1x4"));
}

// ===== Pitch(step, alter, octave) ===== //

TEST(PitchComponentsConstructor, spellsTheComponents) {
    EXPECT_EQ(Pitch("C", 0.5f, 4).getPitch(), "C1x4");
    EXPECT_EQ(Pitch("D", -1.5f, 4).getPitch(), "D3b4");
    EXPECT_EQ(Pitch("A", 0.0f, 4).getPitch(), "A4");
    EXPECT_EQ(Pitch("B", 2.0f, 11).getPitch(), "Bx11");    // the highest representable pitch
    EXPECT_EQ(Pitch("C", -0.5f, -1).getPitch(), "C1b-1");  // the lowest
    EXPECT_EQ(Pitch("E", -1.0f, 4).getQuarterToneSteps(), 63.0f);

    const Pitch natural("C", -0.0f, 4);
    EXPECT_FALSE(std::signbit(natural.getAlter()));
    EXPECT_TRUE(natural == Pitch("C4"));
}

TEST(PitchComponentsConstructor, rejectsAnUnknownStep) {
    for (const std::string step : {"H", "c", "rest", ""}) {
        const std::string message = thrownFirstLine([&] { Pitch p(step, 0.0f, 4); });
        EXPECT_EQ(message, "[maiacore] Unknown diatonic pitch step: " + step) << step;
    }

    // The components are checked in order, the step first.
    EXPECT_EQ(thrownFirstLine([] { Pitch p("H", std::numeric_limits<float>::quiet_NaN(), 12); }),
              "[maiacore] Unknown diatonic pitch step: H");
}

TEST(PitchComponentsConstructor, rejectsAMalformedAlter) {
    for (const float alter :
         {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity()}) {
        const std::string message = thrownFirstLine([&] { Pitch p("C", alter, 4); });
        EXPECT_NE(message.find("must be a finite number"), std::string::npos) << message;
    }
    for (const float alter : {0.99996f, 0.25f, 0.46f}) {
        const std::string message = thrownFirstLine([&] { Pitch p("C", alter, 4); });
        EXPECT_NE(message.find("multiple of 0.5"), std::string::npos)
            << "alter " << alter << ": " << message;
    }
    for (const float alter : {2.5f, -2.5f}) {
        const std::string message = thrownFirstLine([&] { Pitch p("C", alter, 4); });
        EXPECT_NE(message.find("out of range [-2, 2]"), std::string::npos)
            << "alter " << alter << ": " << message;
    }
}

TEST(PitchComponentsConstructor, rejectsAnOctaveOutsideTheRange) {
    EXPECT_EQ(thrownFirstLine([] { Pitch p("C", 0.0f, 12); }),
              "[maiacore] Invalid octave value: 12");
    EXPECT_EQ(thrownFirstLine([] { Pitch p("C", 0.0f, -2); }),
              "[maiacore] Invalid octave value: -2");
}

// Every component is valid, but together they spell a pitch below MIDI note 0.
TEST(PitchComponentsConstructor, rejectsAPitchBelowTheLowestRepresentablePitch) {
    for (const auto& [alter, spelling] :
         std::vector<std::pair<float, std::string>>{{-1.0f, "Cb-1"}, {-1.5f, "C3b-1"}}) {
        // A C++17 lambda cannot capture a structured binding, hence the init-capture.
        const std::string message = thrownFirstLine([alter = alter] { Pitch p("C", alter, -1); });
        EXPECT_EQ(message, "[maiacore] The pitch '" + spelling +
                               "' is below MIDI note 0; the lowest representable pitch is C1b-1")
            << message;
    }
}

// ===== setMidiNumber(midiNumber, accType) ===== //

TEST(PitchSetMidiNumber, honoursTheAccidentalType) {
    Pitch p;
    p.setMidiNumber(61, "b");
    EXPECT_EQ(p.getPitch(), "Db4");
    p.setMidiNumber(61, "#");
    EXPECT_EQ(p.getPitch(), "C#4");
    p.setMidiNumber(61);
    EXPECT_EQ(p.getPitch(), "C#4");
    p.setMidiNumber(157, "x");
    EXPECT_EQ(p.getPitch(), "Bx11");
    p.setMidiNumber(-1, "b");
    EXPECT_TRUE(p.isRest());
}

// An accidental type that cannot spell the MIDI number is rejected as Pitch(int, accType) rejects
// it, and the pitch is left as it was.
TEST(PitchSetMidiNumber, rejectsAnAccidentalTypeThatCannotSpellTheNumberAndLeavesThePitch) {
    Pitch p("E1b4");
    EXPECT_EQ(thrownFirstLine([&] { p.setMidiNumber(60, "b"); }),
              "[maiacore] The MIDI Note '60' cannot be wrote using 'b' accident type");
    EXPECT_EQ(p.getPitch(), "E1b4");
    EXPECT_EQ(thrownFirstLine([&] { p.setMidiNumber(60, "q"); }),
              "[maiacore] Unknown accident type: q");
    EXPECT_EQ(p.getPitch(), "E1b4");
}

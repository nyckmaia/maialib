// The internal pitch spellings behind Note's pitch views (pitch-views.h): the concert spelling of
// a written pitch on a transposing instrument, and the simplest spelling of a pitch. The design,
// with decisions D3 (simplest spelling) and D4 (concert spelling) and the examples of its section
// 3, is docs/superpowers/specs/2026-09-30-note-pitch-views-design.md.

#include "pitch-views.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "maiacore/constants.h"
#include "maiacore/pitch.h"
#include "test-capture.h"

using detail::concertSpelling;
using detail::simplestSpelling;

namespace {
// A pitch string, the transposing interval it is read with, and the expected pitch string.
struct Case {
    std::string pitch;
    int diatonic;
    int chromatic;
    std::string expected;
};

std::string describe(const std::string& pitch, const int diatonic, const int chromatic) {
    return pitch + " (" + std::to_string(diatonic) + ", " + std::to_string(chromatic) + ")";
}

// concertSpelling() as text: the pitch string, or "raises: " and the first line of its error.
std::string concertOrError(const Pitch& written, const int diatonic, const int chromatic) {
    std::string pitch;
    const std::string error =
        thrownFirstLine([&] { pitch = concertSpelling(written, diatonic, chromatic).getPitch(); });
    return error.empty() ? pitch : "raises: " + error;
}

std::string concert(const std::string& written, const int diatonic, const int chromatic) {
    return concertOrError(Pitch(written), diatonic, chromatic);
}

std::string simplest(const std::string& pitch) { return simplestSpelling(Pitch(pitch)).getPitch(); }

// The error for a written pitch whose position 'position' (as std::to_string() writes it) lies
// below C1b-1, the lowest representable pitch.
std::string belowTheFloor(const std::string& written, const int diatonic, const int chromatic,
                          const std::string& position) {
    return "raises: [maiacore] The sounding pitch of the written pitch '" + written +
           "' with transposeDiatonic=" + std::to_string(diatonic) +
           " and transposeChromatic=" + std::to_string(chromatic) + " is at position " + position +
           ", below the lowest representable pitch C1b-1 (-0.5, MIDI note 0), so it has no "
           "sounding spelling, octave, MIDI number or frequency. The written pitch is still "
           "available from getWrittenPitch(); a transposing interval that keeps the sounding "
           "pitch at or above C1b-1 makes it spellable.";
}

// The error for a written pitch whose position lies above B11 and has no diatonic spelling.
std::string aboveTheCeiling(const std::string& written, const int diatonic, const int chromatic,
                            const std::string& position) {
    return "raises: [maiacore] The sounding pitch of the written pitch '" + written +
           "' with transposeDiatonic=" + std::to_string(diatonic) +
           " and transposeChromatic=" + std::to_string(chromatic) + " is at position " + position +
           ", above B11 (MIDI note 155), the highest sounding pitch that can be spelled within "
           "octaves -1..11, so it has no sounding spelling. A lower written pitch or a smaller "
           "transposing interval keeps the sounding pitch at or below B11.";
}

void expectConcert(const std::vector<Case>& cases) {
    for (const Case& c : cases) {
        EXPECT_EQ(concert(c.pitch, c.diatonic, c.chromatic), c.expected)
            << describe(c.pitch, c.diatonic, c.chromatic);
    }
}

// Every pitch this library can hold: the nine alters on the seven steps in octaves -1..11, less
// the three spellings below C1b-1 (Cbb-1, C3b-1 and Cb-1).
std::vector<Pitch> everyPitch() {
    const std::array<float, 9> alters = {-2.0f, -1.5f, -1.0f, -0.5f, 0.0f, 0.5f, 1.0f, 1.5f, 2.0f};
    std::vector<Pitch> pitches;
    for (int octave = c_minPitchOctave; octave <= c_maxPitchOctave; octave++) {
        for (size_t step = 0; step < c_C_diatonicScale.size(); step++) {
            for (const float alter : alters) {
                const float position = 12.0f * static_cast<float>(octave + 1) +
                                       static_cast<float>(c_diatonicStepSemitones[step]) + alter;
                if (position >= -0.5f) {
                    pitches.emplace_back(c_C_diatonicScale[step], alter, octave);
                }
            }
        }
    }
    return pitches;
}

size_t stepIndex(const std::string& step) {
    size_t index = 0;
    while (c_C_diatonicScale[index] != step) {
        index++;
    }
    return index;
}

// The letter and octave 'diatonic' letters away from (step, octave), counted one letter at a
// time as a musician counts them: up from B to the next octave's C, down from C to the previous
// octave's B.
std::pair<size_t, int> countLetters(size_t step, int octave, int diatonic) {
    for (; diatonic > 0; diatonic--) {
        step = (step + 1) % 7;
        if (step == 0) {
            octave++;
        }
    }
    for (; diatonic < 0; diatonic++) {
        if (step == 0) {
            step = 6;
            octave--;
        } else {
            step--;
        }
    }
    return {step, octave};
}

std::string firstFailures(const std::vector<std::string>& failures) {
    std::string text;
    for (size_t i = 0; i < failures.size() && i < 12; i++) {
        text += "\n  " + failures[i];
    }
    return text;
}
}  // namespace

// ===== concertSpelling ===== //

// An untransposed pitch is its own concert spelling, however it is spelled: the concert view
// never simplifies.
TEST(ConcertSpelling, anUntransposedPitchIsItsOwnConcertSpelling) {
    const std::vector<std::string> pitches = {"Cb4",  "B#3",  "Bx3",   "Ebb4", "C3x4",
                                              "E1x4", "Bx11", "C1b-1", "C4"};
    for (const std::string& pitch : pitches) {
        EXPECT_EQ(concert(pitch, 0, 0), pitch);
    }
}

// A rest has no pitch to move, whatever the interval.
TEST(ConcertSpelling, aRestIsUnchanged) {
    for (const auto& [diatonic, chromatic] :
         std::vector<std::pair<int, int>>{{0, 0}, {-1, -2}, {7, 12}, {0, -2}}) {
        const std::string result = concertOrError(Pitch("rest"), diatonic, chromatic);
        EXPECT_EQ(result, "rest") << describe("rest", diatonic, chromatic);
    }
}

// The design's section 3: its concert column, and the MIDI number each concert spelling keeps.
TEST(ConcertSpelling, theSpecExamplesTable) {
    const std::vector<std::tuple<std::string, int, int, std::string, int>> rows = {
        {"Cb4", 0, 0, "Cb4", 59},      // untransposed
        {"F#4", -1, -2, "E4", 64},     // B-flat clarinet
        {"Db4", -1, -2, "Cb4", 59},    // B-flat clarinet
        {"C1x4", -1, -2, "B1b3", 59},  // B-flat clarinet: 58.5 rounds up
        {"B4", -4, -7, "E4", 64},      // horn in F
        {"Bb4", 7, 12, "Bb5", 82},     // piccolo
        {"C4", 0, -2, "Bb3", 58},      // no <diatonic>: the chromatic fallback
    };
    for (const auto& [written, diatonic, chromatic, expected, midi] : rows) {
        EXPECT_EQ(concert(written, diatonic, chromatic), expected)
            << describe(written, diatonic, chromatic);
        EXPECT_EQ(concertSpelling(Pitch(written), diatonic, chromatic).getMidiNumber(), midi)
            << describe(written, diatonic, chromatic);
    }
}

// The letter is the written letter moved by the diatonic interval, so every transposing
// instrument spells what it plays with the letter a musician reads: a B-flat clarinet's F#4 is
// E4, not Fb4, and a piccolo's Bb4 is Bb5, not A#5.
TEST(ConcertSpelling, theLetterComesFromTheDiatonicInterval) {
    expectConcert({
        {"F#4", -1, -2, "E4"},   // B-flat clarinet
        {"G#4", -1, -2, "F#4"},  // B-flat clarinet
        {"D#4", -1, -2, "C#4"},  // B-flat clarinet
        {"Eb4", -1, -2, "Db4"},  // B-flat clarinet
        {"B4", -4, -7, "E4"},    // horn in F
        {"C5", -4, -7, "F4"},    // horn in F
        {"Db4", 1, 2, "Eb4"},    // up a major second
        {"C4", 2, 3, "Eb4"},     // E-flat clarinet
        {"A4", -5, -9, "C4"},    // alto saxophone in E-flat
        {"F#4", -5, -9, "A3"},   // alto saxophone in E-flat
        {"D4", -8, -14, "C3"},   // bass clarinet in B-flat
        {"E4", -7, -12, "E3"},   // an octave down
        {"Bb4", 7, 12, "Bb5"},   // piccolo
    });
}

// The octave is carried whenever the letter crosses C, in either direction. In each case below
// the chromatic fallback would spell the position differently, so the carry itself is checked.
TEST(ConcertSpelling, carriesTheOctaveAcrossC) {
    expectConcert({
        {"Cb4", -1, -2, "Bbb3"},   // the chromatic rule would give A3
        {"Db4", -8, -14, "Cb3"},   // B2
        {"Cb0", -1, -2, "Bbb-1"},  // A-1
        {"Ab4", 2, 3, "Cb5"},      // B4
        {"B#4", 1, 2, "Cx5"},      // D5
        {"B1x4", 1, 2, "C3x5"},    // D1b5
    });
}

// The alter comes from the exact position, so a quarter tone keeps its fraction.
TEST(ConcertSpelling, aQuarterToneKeepsItsFraction) {
    expectConcert({
        {"C1x4", -1, -2, "B1b3"},
        {"D3b4", -1, -2, "C3b4"},  // the chromatic rule would give B1b3
        {"F1x4", -4, -7, "B1b3"},
        {"E1b4", 1, 2, "F1x4"},
        {"G3x4", -5, -9, "B1x3"},  // C1b4
    });
}

// A MusicXML <transpose> without <diatonic> gives no letter to move: the chromatic rule spells
// the position, with the natural or sharp spelling going up and the flat spelling going down when
// one exists in the same octave.
TEST(ConcertSpelling, withoutADiatonicIntervalTheChromaticRuleSpellsThePosition) {
    expectConcert({
        {"C4", 0, -2, "Bb3"},
        {"F#4", 0, -2, "Fb4"},
        {"Db4", 0, -2, "B3"},  // Cb4, the flat spelling, lies in another octave
        {"C4", 0, 2, "D4"},
        {"C#4", 0, 2, "D#4"},
        {"C1x4", 0, -2, "B1b3"},
        {"C4", 0, -12, "C3"},
    });
}

// When the letter would need an alter beyond a double accidental, the chromatic rule spells the
// position.
TEST(ConcertSpelling, fallsBackWhenTheAlterWouldPassADoubleAccidental) {
    expectConcert({
        {"Fx4", 1, 3, "A#4"},      // G would need +3
        {"Ex4", 1, 2, "G#4"},      // F would need +3
        {"Dbb4", -1, -3, "A3"},    // C would need -3
        {"C3b4", -1, -2, "A1b3"},  // B would need -2.5
    });
}

// When the letter would land outside octaves -1..11, the chromatic rule spells the position.
TEST(ConcertSpelling, fallsBackWhenTheOctaveWouldLeaveTheRange) {
    expectConcert({
        {"Bb11", 1, 1, "B11"},       // Cb12
        {"D-1", -2, -1, "Db-1"},     // Bx-2
        {"C#-1", -1, -1, "C-1"},     // B#-2
        {"C1x-1", -1, -1, "C1b-1"},  // B1x-2; C1b-1 is the lowest representable pitch
    });
}

// A spelling the diatonic interval reaches is returned even above B11, up to Bx11.
TEST(ConcertSpelling, aDiatonicSpellingAboveB11IsReturned) {
    expectConcert({
        {"A#11", 1, 2, "B#11"},
        {"A#11", 1, 3, "Bx11"},
        {"B#10", 7, 12, "B#11"},
        {"Bx10", 7, 12, "Bx11"},
        {"Gx11", 2, 3, "B#11"},
        {"A1x11", 1, 2, "B1x11"},
    });
}

// A position above B11 without a diatonic spelling is rejected by the chromatic rule, with its
// error.
TEST(ConcertSpelling, aPositionAboveB11WithoutADiatonicSpellingIsRejected) {
    EXPECT_EQ(concert("B11", 1, 3), aboveTheCeiling("B11", 1, 3, "158.000000"));    // C#12
    EXPECT_EQ(concert("C11", 7, 12), aboveTheCeiling("C11", 7, 12, "156.000000"));  // C12
    EXPECT_EQ(concert("B11", 0, 1), aboveTheCeiling("B11", 0, 1, "156.000000"));    // no letter
}

// A position below C1b-1 has no spelling at all. It is rejected with the chromatic rule's error
// also when the diatonic interval reaches a letter within the octave range: Db-1 and D-1 would be
// spelled Cb-1, which lies below MIDI note 0.
TEST(ConcertSpelling, aPositionBelowTheFloorIsRejected) {
    EXPECT_EQ(concert("Db-1", -1, -2), belowTheFloor("Db-1", -1, -2, "-1.000000"));
    EXPECT_EQ(concert("D-1", -1, -3), belowTheFloor("D-1", -1, -3, "-1.000000"));
    EXPECT_EQ(concert("C#-1", -1, -2), belowTheFloor("C#-1", -1, -2, "-1.000000"));
    EXPECT_EQ(concert("C1b-1", -1, -1), belowTheFloor("C1b-1", -1, -1, "-1.500000"));
    EXPECT_EQ(concert("C1x-1", -1, -2), belowTheFloor("C1x-1", -1, -2, "-1.500000"));
}

// The floor is checked on the written MIDI number and the chromatic interval added in 64 bits, so
// an interval at the limits of int cannot wrap the position around to the other end of the range:
// the largest upward interval is rejected as above B11, the largest downward one as below C1b-1.
TEST(ConcertSpelling, anIntervalAtTheLimitsOfIntIsRejectedOnItsOwnSide) {
    const int up = std::numeric_limits<int>::max();
    const int down = std::numeric_limits<int>::min();
    EXPECT_EQ(concert("C4", 1, up), aboveTheCeiling("C4", 1, up, "2147483648.000000"));
    EXPECT_EQ(concert("C4", -1, down), belowTheFloor("C4", -1, down, "-2147483648.000000"));
}

// Every pitch this library can hold, with transposing intervals of every size and direction:
// the position always moves by the chromatic interval, and whenever the letter the diatonic
// interval reaches (counted one letter at a time) can spell it within a double accidental and
// octaves -1..11, that is the spelling. Otherwise the chromatic rule spells it, rejecting a
// position above B11; a position below C1b-1 is always rejected.
TEST(ConcertSpelling, everySpellingAndIntervalFollowsTheRule) {
    const std::vector<std::pair<int, int>> intervals = {
        {-1, -2},  {-2, -3}, {-4, -7}, {-5, -9},   {1, 2},  {2, 3}, {7, 12}, {-7, -12},
        {-8, -14}, {0, -2},  {0, 0},   {0, 1},     {1, 0},  {1, 1}, {1, 3},  {-1, 0},
        {-1, -1},  {-1, -3}, {-2, -1}, {3, 5},     {4, 7},  {5, 9}, {6, 11}, {-3, -5},
        {-6, -10}, {8, 14},  {14, 24}, {-14, -24}, {1, -1},
    };
    std::vector<std::string> failures;
    std::map<std::string, size_t> cases;
    for (const Pitch& written : everyPitch()) {
        for (const auto& [diatonic, chromatic] : intervals) {
            const float position = written.getQuarterToneSteps() + static_cast<float>(chromatic);
            if (position < -2.0f || position > 157.0f) {
                // No letter reaches such a position within a double accidental (Cbb-1 is -2, Bx11
                // is 157), so it is rejected as below C1b-1 or above B11, as the two tests above
                // check. Every rejection builds a stack trace, and there are hundreds here.
                cases["beyond every spelling"]++;
                continue;
            }

            const std::string where = describe(written.getPitch(), diatonic, chromatic);
            const std::string actual = concertOrError(written, diatonic, chromatic);
            const bool raised = actual.rfind("raises: ", 0) == 0;

            const auto [step, octave] = countLetters(stepIndex(written.getPitchStep()),
                                                     written.getOctave().value(), diatonic);
            const float alter = position - (12.0f * static_cast<float>(octave + 1) +
                                            static_cast<float>(c_diatonicStepSemitones[step]));
            const bool isDiatonic = diatonic != 0 && octave >= c_minPitchOctave &&
                                    octave <= c_maxPitchOctave && std::fabs(alter) <= 2.0f;

            if (diatonic == 0 && chromatic == 0) {
                cases["untransposed"]++;
                if (actual != written.getPitch()) {
                    failures.push_back(where + " gives " + actual);
                }
            } else if (position < -0.5f) {
                cases["below the floor"]++;
                if (!raised || actual.find("below the lowest representable pitch C1b-1") ==
                                   std::string::npos) {
                    failures.push_back(where + " gives " + actual);
                }
            } else if (isDiatonic) {
                cases["diatonic"]++;
                const std::string expected =
                    Pitch(c_C_diatonicScale[step], alter, octave).getPitch();
                if (actual != expected) {
                    failures.push_back(where + " gives " + actual + ", expected " + expected);
                }
            } else if (position > 155.0f) {
                cases["above B11 without a diatonic spelling"]++;
                if (!raised || actual.find("above B11 (MIDI note 155)") == std::string::npos) {
                    failures.push_back(where + " gives " + actual);
                }
            } else {
                cases["chromatic fallback"]++;
                if (raised || Pitch(actual).getQuarterToneSteps() != position) {
                    failures.push_back(where + " gives " + actual + ", not a spelling of " +
                                       std::to_string(position));
                }
            }
        }
    }

    // Every branch of the rule is exercised.
    EXPECT_EQ(cases.size(), 6u);
    for (const auto& [name, count] : cases) {
        EXPECT_GT(count, 0u) << name;
    }
    EXPECT_TRUE(failures.empty()) << failures.size() << " cases differ:" << firstFailures(failures);
}

// ===== simplestSpelling ===== //

// Decision D3's examples: the smallest alter wins, a tie keeps the pitch's own side, and the octave
// is the simplest spelling's own.
TEST(SimplestSpelling, theSpecExamples) {
    const std::vector<std::pair<std::string, std::string>> examples = {
        {"Db4", "Db4"},   {"C#4", "C#4"},   {"Cb4", "B3"},    {"E#4", "F4"},  {"B#3", "C4"},
        {"Fb4", "E4"},    {"Ebb4", "D4"},   {"Fx4", "G4"},    {"Bbb4", "A4"}, {"C1x4", "C1x4"},
        {"C3x4", "D1b4"}, {"E1x4", "E1x4"}, {"B1b3", "B1b3"},
    };
    for (const auto& [pitch, expected] : examples) {
        EXPECT_EQ(simplest(pitch), expected) << pitch;
    }
    EXPECT_EQ(simplestSpelling(Pitch("Cb4")).getOctave(), 3);
}

// The octave is the one the simplest spelling carries, which differs from the pitch's own when
// the letter crosses C.
TEST(SimplestSpelling, theOctaveIsTheSimplestSpellingsOwn) {
    const std::vector<std::tuple<std::string, std::string, int>> examples = {
        {"Cb4", "B3", 3},    {"B#3", "C4", 4},   {"Cbb5", "Bb4", 4}, {"B3x3", "C1x4", 4},
        {"C3b4", "B1b3", 3}, {"Cb0", "B-1", -1}, {"B#-1", "C0", 0},
    };
    for (const auto& [pitch, expected, octave] : examples) {
        const Pitch result = simplestSpelling(Pitch(pitch));
        EXPECT_EQ(result.getPitch(), expected) << pitch;
        EXPECT_EQ(result.getOctave(), octave) << pitch;
    }
}

// A black key has a sharp and a flat spelling equally close: the one on the pitch's own side is
// kept, also when the pitch is spelled with a double accidental.
TEST(SimplestSpelling, aTieBetweenASharpAndAFlatKeepsTheOwnSide) {
    const std::vector<std::pair<std::string, std::string>> examples = {
        {"C#4", "C#4"}, {"Db4", "Db4"},  {"D#4", "D#4"}, {"Eb4", "Eb4"},  {"F#4", "F#4"},
        {"Gb4", "Gb4"}, {"G#4", "G#4"},  {"Ab4", "Ab4"}, {"A#4", "A#4"},  {"Bb4", "Bb4"},
        {"Bx3", "C#4"}, {"Fbb4", "Eb4"}, {"Ex4", "F#4"}, {"Cbb5", "Bb4"},
    };
    for (const auto& [pitch, expected] : examples) {
        EXPECT_EQ(simplest(pitch), expected) << pitch;
    }
}

// Between E and F, and between B and C, a quarter tone is a quarter tone from both letters: the
// one on the pitch's own side is kept.
TEST(SimplestSpelling, aQuarterToneTieKeepsTheOwnSide) {
    const std::vector<std::pair<std::string, std::string>> examples = {
        {"E1x4", "E1x4"},
        {"F1b4", "F1b4"},
        {"B1x3", "B1x3"},
        {"C1b4", "C1b4"},
    };
    for (const auto& [pitch, expected] : examples) {
        EXPECT_EQ(simplest(pitch), expected) << pitch;
    }
}

// Every other quarter tone is a quarter tone from exactly one letter, which spells it.
TEST(SimplestSpelling, aQuarterToneTakesTheNearestLetter) {
    const std::vector<std::pair<std::string, std::string>> examples = {
        {"C3x4", "D1b4"}, {"E3b4", "D1x4"}, {"F3b4", "E1b4"}, {"B3x3", "C1x4"},
        {"C3b4", "B1b3"}, {"D3b4", "C1x4"}, {"A3x4", "B1b4"}, {"G1x4", "G1x4"},
    };
    for (const auto& [pitch, expected] : examples) {
        EXPECT_EQ(simplest(pitch), expected) << pitch;
    }
}

TEST(SimplestSpelling, aRestIsUnchanged) { EXPECT_TRUE(simplestSpelling(Pitch("rest")).isRest()); }

// A simpler spelling outside octaves -1..11 does not exist: Bx11 is not respelled C#12.
TEST(SimplestSpelling, staysWithinOctavesMinus1To11) {
    const std::vector<std::pair<std::string, std::string>> examples = {
        {"Bx11", "Bx11"}, {"B#11", "B#11"},   {"B3x11", "B3x11"}, {"B1x11", "B1x11"},
        {"Dbb-1", "C-1"}, {"C1b-1", "C1b-1"}, {"Cb0", "B-1"},
    };
    for (const auto& [pitch, expected] : examples) {
        EXPECT_EQ(simplest(pitch), expected) << pitch;
    }
}

// Every pitch this library can hold, against every other spelling of its exact position: the
// result is the one with the smallest |alter|, on the pitch's own side when a sharp-side and a
// flat-side spelling tie, and it is its own simplest spelling.
TEST(SimplestSpelling, everyPitchGetsTheSimplestOfItsSpellings) {
    const std::vector<Pitch> pitches = everyPitch();
    ASSERT_EQ(pitches.size(), 816u);

    std::map<float, std::vector<Pitch>> spellingsAt;
    for (const Pitch& pitch : pitches) {
        spellingsAt[pitch.getQuarterToneSteps()].push_back(pitch);
    }

    std::vector<std::string> failures;
    size_t ties = 0;
    for (const auto& [position, spellings] : spellingsAt) {
        float fewest = 2.0f;
        for (const Pitch& spelling : spellings) {
            fewest = std::fmin(fewest, std::fabs(spelling.getAlter()));
        }
        std::vector<Pitch> simplestOnes;
        for (const Pitch& spelling : spellings) {
            if (std::fabs(spelling.getAlter()) == fewest) {
                simplestOnes.push_back(spelling);
            }
        }

        for (const Pitch& pitch : spellings) {
            std::string expected = simplestOnes.front().getPitch();
            if (simplestOnes.size() > 1) {
                ties++;
                for (const Pitch& candidate : simplestOnes) {
                    if ((candidate.getAlter() > 0.0f) == (pitch.getAlter() > 0.0f)) {
                        expected = candidate.getPitch();
                    }
                }
            }
            const Pitch actual = simplestSpelling(pitch);
            if (actual.getPitch() != expected) {
                failures.push_back(pitch.getPitch() + " gives " + actual.getPitch() +
                                   ", expected " + expected);
            } else if (simplestSpelling(actual).getPitch() != expected) {
                failures.push_back(expected + " is not its own simplest spelling");
            }
        }
    }

    EXPECT_GT(ties, 0u);
    EXPECT_TRUE(failures.empty()) << failures.size()
                                  << " pitches differ:" << firstFailures(failures);
}

// ===== The sounding spelling: the simplest spelling of the concert spelling ===== //

// Decision D4's sounding examples and section 3's sounding column, with their octaves.
TEST(SoundingSpelling, isTheSimplestSpellingOfTheConcertSpelling) {
    const std::vector<std::tuple<std::string, int, int, std::string, int>> rows = {
        {"Db4", -1, -2, "B3", 3},     // concert Cb4
        {"Eb4", -1, -2, "Db4", 4},    // concert Db4, a tie with C#4 that keeps the flat side
        {"C1x4", -1, -2, "B1b3", 3},  // concert B1b3
        {"Cb4", 0, 0, "B3", 3},       // untransposed
        {"F#4", -1, -2, "E4", 4},     // B-flat clarinet
        {"B4", -4, -7, "E4", 4},      // horn in F
        {"Bb4", 7, 12, "Bb5", 5},     // piccolo
        {"C4", 0, -2, "Bb3", 3},      // no <diatonic>
    };
    for (const auto& [written, diatonic, chromatic, expected, octave] : rows) {
        const Pitch sounding =
            simplestSpelling(concertSpelling(Pitch(written), diatonic, chromatic));
        EXPECT_EQ(sounding.getPitch(), expected) << describe(written, diatonic, chromatic);
        EXPECT_EQ(sounding.getOctave(), octave) << describe(written, diatonic, chromatic);
    }
}

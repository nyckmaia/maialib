// The melodic lines the melody search reads (melodic-lines.h): one per part, staff and voice; a
// chord by its highest sounding note; tied notes as one event; rests as events; grace notes left
// out. The design is docs/superpowers/specs/2026-10-05-melody-search-design.md, section 3.

#include "melodic-lines.h"

#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

#include "maiacore/measure.h"
#include "maiacore/note.h"
#include "maiacore/part.h"
#include "maiacore/score.h"

using maiacore::detail::MelodicEvent;
using maiacore::detail::MelodicLine;
using maiacore::detail::melodicLines;

namespace {
// The parts of a score, in score order.
std::vector<Part> partsOf(Score& score) {
    std::vector<Part> parts;
    for (int p = 0; p < score.getNumParts(); p++) {
        parts.push_back(score.getPart(p));
    }
    return parts;
}

// The lines of a score, built from a copy of its parts.
std::vector<MelodicLine> linesOf(Score& score) { return melodicLines(partsOf(score)); }

// The (part, staff, voice) of each line.
std::vector<std::tuple<int, int, int>> keysOf(const std::vector<MelodicLine>& lines) {
    std::vector<std::tuple<int, int, int>> keys;
    for (const MelodicLine& line : lines) {
        keys.emplace_back(line.partIdx, line.staff, line.voice);
    }
    return keys;
}

// The written pitch of each event of a line.
std::vector<std::string> pitchesOf(const MelodicLine& line) {
    std::vector<std::string> pitches;
    for (const MelodicEvent& event : line.events) {
        pitches.push_back(event.note.getWrittenPitch());
    }
    return pitches;
}
}  // namespace

// One line per voice of each staff, listed by staff, then voice; the lower staff's voice 5 is a
// line of its own, and the voices of one staff never mix.
TEST(MelodicLines, EachVoiceOfEachStaffIsALine) {
    Score score("./test/xml_examples/unit_test/melody_staves_and_voices.musicxml");
    const std::vector<MelodicLine> lines = linesOf(score);

    ASSERT_EQ(keysOf(lines),
              (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {0, 0, 2}, {0, 1, 5}}));
    EXPECT_EQ(pitchesOf(lines[0]),
              (std::vector<std::string>{"E5", "F5", "G5", "A5", "B5", "C6", "D6"}));
    EXPECT_EQ(pitchesOf(lines[1]), (std::vector<std::string>{"C5", "B4", "A4"}));
    EXPECT_EQ(pitchesOf(lines[2]),
              (std::vector<std::string>{"C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4"}));
    EXPECT_EQ(lines[2].events[4].measureIdx, 1);
}

// Lines are listed by part first: a two-part score lists the first part's lines, then the
// second's.
TEST(MelodicLines, LinesAreListedByPart) {
    Score score("./test/xml_examples/unit_test/melody_transposing_instrument.musicxml");
    const std::vector<MelodicLine> lines = linesOf(score);

    ASSERT_EQ(keysOf(lines), (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {1, 0, 1}}));
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"D4", "E4", "F#4", "G4"}));
}

// A chord is one event, which stands for its highest sounding note, written first or last; a
// grace note is no event; a rest is one.
TEST(MelodicLines, AChordStandsForItsHighestNote) {
    Score score("./test/xml_examples/unit_test/melody_chord_top_note.musicxml");
    const std::vector<MelodicLine> lines = linesOf(score);

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"G4", "D5", "B4", "rest"}));
}

// The highest note is the highest it sounds: a horn in F's written C5 sounds F4, below a written
// G4 of the same chord on an untransposed note.
TEST(MelodicLines, TheHighestNoteIsTheHighestSounding) {
    Score score({"Horn in F"}, 1);
    Measure& measure = score.getPart(0).getMeasure(0);
    measure.addNote(Note("G4"));
    Note horn("C5");
    horn.setTransposingInterval(-4, -7);
    horn.setIsInChord(true);
    measure.addNote(horn);

    const std::vector<MelodicLine> lines = linesOf(score);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"G4"}));
}

// A note tied to the previous event's note extends it: the event keeps its first note's written
// pitch and measure, and its duration is the sum.
TEST(MelodicLines, ATiedNoteExtendsTheEventItContinues) {
    Score score("./test/xml_examples/unit_test/melody_tie_across_barline.musicxml");
    const std::vector<MelodicLine> lines = linesOf(score);

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"C4", "D4", "E4", "F4", "G4"}));
    std::vector<float> durations;
    std::vector<int> measures;
    for (const MelodicEvent& event : lines[0].events) {
        durations.push_back(event.note.getQuarterDuration());
        measures.push_back(event.measureIdx);
    }
    EXPECT_EQ(durations, (std::vector<float>{1.0f, 1.0f, 3.0f, 1.0f, 2.0f}));
    EXPECT_EQ(measures, (std::vector<int>{0, 0, 0, 1, 1}));
}

// Only a tie stop at the same sounding position continues an event. The durations add exactly
// in ticks at a multiple of 1024 divisions, whatever divisions each note counts in: a quarter at
// 256 divisions and an eighth at 2 make 1536 ticks at 1024.
TEST(MelodicLines, OnlyATieStopAtTheSamePitchContinuesAnEvent) {
    Score score({"Flute"}, 1);
    Measure& measure = score.getPart(0).getMeasure(0);
    Note first("C4");
    first.setDuration(256, 256);
    first.setTieStart();
    Note tied("C4");
    tied.setDuration(1, 2);
    tied.setTieStop();
    Note otherPitch("D4");
    otherPitch.setTieStop();
    measure.addNote({first, tied, otherPitch});

    const std::vector<MelodicLine> lines = linesOf(score);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"C4", "D4"}));
    EXPECT_EQ(lines[0].events[0].note.getDurationTicks(), 1536);
    EXPECT_EQ(lines[0].events[0].note.getDivisionsPerQuarterNote(), 1024);
    EXPECT_FLOAT_EQ(lines[0].events[0].note.getQuarterDuration(), 1.5f);
}

// A chord note that follows no note of its voice on its staff -- the rest of a chord written on
// another staff -- joins no event.
TEST(MelodicLines, AChordNoteWithoutItsFirstNoteOnTheStaffIsNoEvent) {
    Score score({"Piano"}, 1);
    score.getPart(0).getMeasure(0).setNumStaves(2);
    Measure& measure = score.getPart(0).getMeasure(0);
    measure.addNote(Note("C5"), 0);
    Note lower("C3");
    lower.setIsInChord(true);
    measure.addNote(lower, 1);

    const std::vector<MelodicLine> lines = linesOf(score);
    ASSERT_EQ(keysOf(lines), (std::vector<std::tuple<int, int, int>>{{0, 0, 1}}));
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"C5"}));
}

// A chord tied to a chord is one event when its highest note continues the highest note; a tie
// on an inner note alone extends nothing.
TEST(MelodicLines, ATiedChordIsOneEventWhenItsHighestNoteIsTied) {
    const auto chord = [](const char* lower, const char* upper, const bool stop) {
        Note bottom(lower);
        Note top(upper);
        top.setIsInChord(true);
        if (stop) {
            bottom.setTieStop();
            top.setTieStop();
        } else {
            bottom.setTieStart();
            top.setTieStart();
        }
        return std::vector<Note>{bottom, top};
    };
    Score tied({"Piano"}, 1);
    Measure& measure = tied.getPart(0).getMeasure(0);
    measure.addNote(chord("C4", "E4", false));
    measure.addNote(chord("C4", "E4", true));
    measure.addNote(Note("G4"));

    const std::vector<MelodicLine> lines = linesOf(tied);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(pitchesOf(lines[0]), (std::vector<std::string>{"E4", "G4"}));
    EXPECT_FLOAT_EQ(lines[0].events[0].note.getQuarterDuration(), 2.0f);

    Score inner({"Piano"}, 1);
    Measure& innerMeasure = inner.getPart(0).getMeasure(0);
    std::vector<Note> first = chord("C4", "E4", false);
    first[1].removeTies();
    std::vector<Note> second = chord("C4", "E4", true);
    second[1].removeTies();
    innerMeasure.addNote(first);
    innerMeasure.addNote(second);
    EXPECT_EQ(pitchesOf(linesOf(inner)[0]), (std::vector<std::string>{"E4", "E4"}));
}

// A measure with more staves than the part's first -- the staff count can grow mid-part -- adds
// the lines of its new staff from that measure on.
TEST(MelodicLines, AStaffAddedMidPartIsALineFromThatMeasure) {
    Score score({"Piano"}, 2);
    score.getPart(0).getMeasure(0).addNote(Note("C5"));
    Measure& second = score.getPart(0).getMeasure(1);
    second.setNumStaves(2);
    second.addNote(Note("D5"), 0);
    second.addNote(Note("C3"), 1);

    const std::vector<MelodicLine> lines = linesOf(score);
    ASSERT_EQ(keysOf(lines), (std::vector<std::tuple<int, int, int>>{{0, 0, 1}, {0, 1, 1}}));
    EXPECT_EQ(pitchesOf(lines[1]), (std::vector<std::string>{"C3"}));
    EXPECT_EQ(lines[1].events[0].measureIdx, 1);
}

#include "maiacore/interval.h"

#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

#include "test-capture.h"
#include "transposing-instruments.h"

using namespace testing;

TEST(getNumSemitones, absoluteValue_False_asc) {
    Interval interval;

    // C chromatic pitchClass block
    interval.setNotes("C4", "Cbb4");
    EXPECT_EQ(interval.getNumSemitones(false), -2);
    interval.setNotes("C4", "Cb4");
    EXPECT_EQ(interval.getNumSemitones(false), -1);
    interval.setNotes("C4", "C4");
    EXPECT_EQ(interval.getNumSemitones(false), 0);
    interval.setNotes("C4", "C#4");
    EXPECT_EQ(interval.getNumSemitones(false), 1);
    interval.setNotes("C4", "Cx4");
    EXPECT_EQ(interval.getNumSemitones(false), 2);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb4");
    EXPECT_EQ(interval.getNumSemitones(false), 0);
    interval.setNotes("C4", "Db4");
    EXPECT_EQ(interval.getNumSemitones(false), 1);
    interval.setNotes("C4", "D4");
    EXPECT_EQ(interval.getNumSemitones(false), 2);
    interval.setNotes("C4", "D#4");
    EXPECT_EQ(interval.getNumSemitones(false), 3);
    interval.setNotes("C4", "Dx4");
    EXPECT_EQ(interval.getNumSemitones(false), 4);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb4");
    EXPECT_EQ(interval.getNumSemitones(false), 2);
    interval.setNotes("C4", "Eb4");
    EXPECT_EQ(interval.getNumSemitones(false), 3);
    interval.setNotes("C4", "E4");
    EXPECT_EQ(interval.getNumSemitones(false), 4);
    interval.setNotes("C4", "E#4");
    EXPECT_EQ(interval.getNumSemitones(false), 5);
    interval.setNotes("C4", "Ex4");
    EXPECT_EQ(interval.getNumSemitones(false), 6);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb4");
    EXPECT_EQ(interval.getNumSemitones(false), 3);
    interval.setNotes("C4", "Fb4");
    EXPECT_EQ(interval.getNumSemitones(false), 4);
    interval.setNotes("C4", "F4");
    EXPECT_EQ(interval.getNumSemitones(false), 5);
    interval.setNotes("C4", "F#4");
    EXPECT_EQ(interval.getNumSemitones(false), 6);
    interval.setNotes("C4", "Fx4");
    EXPECT_EQ(interval.getNumSemitones(false), 7);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb4");
    EXPECT_EQ(interval.getNumSemitones(false), 5);
    interval.setNotes("C4", "Gb4");
    EXPECT_EQ(interval.getNumSemitones(false), 6);
    interval.setNotes("C4", "G4");
    EXPECT_EQ(interval.getNumSemitones(false), 7);
    interval.setNotes("C4", "G#4");
    EXPECT_EQ(interval.getNumSemitones(false), 8);
    interval.setNotes("C4", "Gx4");
    EXPECT_EQ(interval.getNumSemitones(false), 9);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb4");
    EXPECT_EQ(interval.getNumSemitones(false), 7);
    interval.setNotes("C4", "Ab4");
    EXPECT_EQ(interval.getNumSemitones(false), 8);
    interval.setNotes("C4", "A4");
    EXPECT_EQ(interval.getNumSemitones(false), 9);
    interval.setNotes("C4", "A#4");
    EXPECT_EQ(interval.getNumSemitones(false), 10);
    interval.setNotes("C4", "Ax4");
    EXPECT_EQ(interval.getNumSemitones(false), 11);

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb4");
    EXPECT_EQ(interval.getNumSemitones(false), 9);
    interval.setNotes("C4", "Bb4");
    EXPECT_EQ(interval.getNumSemitones(false), 10);
    interval.setNotes("C4", "B4");
    EXPECT_EQ(interval.getNumSemitones(false), 11);
    interval.setNotes("C4", "B#4");
    EXPECT_EQ(interval.getNumSemitones(false), 12);
    interval.setNotes("C4", "Bx4");
    EXPECT_EQ(interval.getNumSemitones(false), 13);

    // // C chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Cbb5");
    EXPECT_EQ(interval.getNumSemitones(false), 10);
    interval.setNotes("C4", "Cb5");
    EXPECT_EQ(interval.getNumSemitones(false), 11);
    interval.setNotes("C4", "C5");
    EXPECT_EQ(interval.getNumSemitones(false), 12);
    interval.setNotes("C4", "C#5");
    EXPECT_EQ(interval.getNumSemitones(false), 13);
    interval.setNotes("C4", "Cx5");
    EXPECT_EQ(interval.getNumSemitones(false), 14);
}

TEST(getNumSemitones, absoluteValue_True_asc) {
    Interval interval;

    // C chromatic pitchClass block
    interval.setNotes("C4", "Cbb4");
    EXPECT_EQ(interval.getNumSemitones(true), 2);
    interval.setNotes("C4", "Cb4");
    EXPECT_EQ(interval.getNumSemitones(true), 1);
    interval.setNotes("C4", "C4");
    EXPECT_EQ(interval.getNumSemitones(true), 0);
    interval.setNotes("C4", "C#4");
    EXPECT_EQ(interval.getNumSemitones(true), 1);
    interval.setNotes("C4", "Cx4");
    EXPECT_EQ(interval.getNumSemitones(true), 2);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb4");
    EXPECT_EQ(interval.getNumSemitones(true), 0);
    interval.setNotes("C4", "Db4");
    EXPECT_EQ(interval.getNumSemitones(true), 1);
    interval.setNotes("C4", "D4");
    EXPECT_EQ(interval.getNumSemitones(true), 2);
    interval.setNotes("C4", "D#4");
    EXPECT_EQ(interval.getNumSemitones(true), 3);
    interval.setNotes("C4", "Dx4");
    EXPECT_EQ(interval.getNumSemitones(true), 4);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb4");
    EXPECT_EQ(interval.getNumSemitones(true), 2);
    interval.setNotes("C4", "Eb4");
    EXPECT_EQ(interval.getNumSemitones(true), 3);
    interval.setNotes("C4", "E4");
    EXPECT_EQ(interval.getNumSemitones(true), 4);
    interval.setNotes("C4", "E#4");
    EXPECT_EQ(interval.getNumSemitones(true), 5);
    interval.setNotes("C4", "Ex4");
    EXPECT_EQ(interval.getNumSemitones(true), 6);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb4");
    EXPECT_EQ(interval.getNumSemitones(true), 3);
    interval.setNotes("C4", "Fb4");
    EXPECT_EQ(interval.getNumSemitones(true), 4);
    interval.setNotes("C4", "F4");
    EXPECT_EQ(interval.getNumSemitones(true), 5);
    interval.setNotes("C4", "F#4");
    EXPECT_EQ(interval.getNumSemitones(true), 6);
    interval.setNotes("C4", "Fx4");
    EXPECT_EQ(interval.getNumSemitones(true), 7);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb4");
    EXPECT_EQ(interval.getNumSemitones(true), 5);
    interval.setNotes("C4", "Gb4");
    EXPECT_EQ(interval.getNumSemitones(true), 6);
    interval.setNotes("C4", "G4");
    EXPECT_EQ(interval.getNumSemitones(true), 7);
    interval.setNotes("C4", "G#4");
    EXPECT_EQ(interval.getNumSemitones(true), 8);
    interval.setNotes("C4", "Gx4");
    EXPECT_EQ(interval.getNumSemitones(true), 9);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb4");
    EXPECT_EQ(interval.getNumSemitones(true), 7);
    interval.setNotes("C4", "Ab4");
    EXPECT_EQ(interval.getNumSemitones(true), 8);
    interval.setNotes("C4", "A4");
    EXPECT_EQ(interval.getNumSemitones(true), 9);
    interval.setNotes("C4", "A#4");
    EXPECT_EQ(interval.getNumSemitones(true), 10);
    interval.setNotes("C4", "Ax4");
    EXPECT_EQ(interval.getNumSemitones(true), 11);

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb4");
    EXPECT_EQ(interval.getNumSemitones(true), 9);
    interval.setNotes("C4", "Bb4");
    EXPECT_EQ(interval.getNumSemitones(true), 10);
    interval.setNotes("C4", "B4");
    EXPECT_EQ(interval.getNumSemitones(true), 11);
    interval.setNotes("C4", "B#4");
    EXPECT_EQ(interval.getNumSemitones(true), 12);
    interval.setNotes("C4", "Bx4");
    EXPECT_EQ(interval.getNumSemitones(true), 13);

    // // C chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Cbb5");
    EXPECT_EQ(interval.getNumSemitones(true), 10);
    interval.setNotes("C4", "Cb5");
    EXPECT_EQ(interval.getNumSemitones(true), 11);
    interval.setNotes("C4", "C5");
    EXPECT_EQ(interval.getNumSemitones(true), 12);
    interval.setNotes("C4", "C#5");
    EXPECT_EQ(interval.getNumSemitones(true), 13);
    interval.setNotes("C4", "Cx5");
    EXPECT_EQ(interval.getNumSemitones(true), 14);
}

TEST(getNumSemitones, absoluteValue_False_desc) {
    Interval interval;

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb3");
    EXPECT_EQ(interval.getNumSemitones(false), -3);
    interval.setNotes("C4", "Bb3");
    EXPECT_EQ(interval.getNumSemitones(false), -2);
    interval.setNotes("C4", "B3");
    EXPECT_EQ(interval.getNumSemitones(false), -1);
    interval.setNotes("C4", "B#3");
    EXPECT_EQ(interval.getNumSemitones(false), 0);
    interval.setNotes("C4", "Bx3");
    EXPECT_EQ(interval.getNumSemitones(false), 1);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb3");
    EXPECT_EQ(interval.getNumSemitones(false), -5);
    interval.setNotes("C4", "Ab3");
    EXPECT_EQ(interval.getNumSemitones(false), -4);
    interval.setNotes("C4", "A3");
    EXPECT_EQ(interval.getNumSemitones(false), -3);
    interval.setNotes("C4", "A#3");
    EXPECT_EQ(interval.getNumSemitones(false), -2);
    interval.setNotes("C4", "Ax3");
    EXPECT_EQ(interval.getNumSemitones(false), -1);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb3");
    EXPECT_EQ(interval.getNumSemitones(false), -7);
    interval.setNotes("C4", "Gb3");
    EXPECT_EQ(interval.getNumSemitones(false), -6);
    interval.setNotes("C4", "G3");
    EXPECT_EQ(interval.getNumSemitones(false), -5);
    interval.setNotes("C4", "G#3");
    EXPECT_EQ(interval.getNumSemitones(false), -4);
    interval.setNotes("C4", "Gx3");
    EXPECT_EQ(interval.getNumSemitones(false), -3);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb3");
    EXPECT_EQ(interval.getNumSemitones(false), -9);
    interval.setNotes("C4", "Fb3");
    EXPECT_EQ(interval.getNumSemitones(false), -8);
    interval.setNotes("C4", "F3");
    EXPECT_EQ(interval.getNumSemitones(false), -7);
    interval.setNotes("C4", "F#3");
    EXPECT_EQ(interval.getNumSemitones(false), -6);
    interval.setNotes("C4", "Fx3");
    EXPECT_EQ(interval.getNumSemitones(false), -5);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb3");
    EXPECT_EQ(interval.getNumSemitones(false), -10);
    interval.setNotes("C4", "Eb3");
    EXPECT_EQ(interval.getNumSemitones(false), -9);
    interval.setNotes("C4", "E3");
    EXPECT_EQ(interval.getNumSemitones(false), -8);
    interval.setNotes("C4", "E#3");
    EXPECT_EQ(interval.getNumSemitones(false), -7);
    interval.setNotes("C4", "Ex3");
    EXPECT_EQ(interval.getNumSemitones(false), -6);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb3");
    EXPECT_EQ(interval.getNumSemitones(false), -12);
    interval.setNotes("C4", "Db3");
    EXPECT_EQ(interval.getNumSemitones(false), -11);
    interval.setNotes("C4", "D3");
    EXPECT_EQ(interval.getNumSemitones(false), -10);
    interval.setNotes("C4", "D#3");
    EXPECT_EQ(interval.getNumSemitones(false), -9);
    interval.setNotes("C4", "Dx3");
    EXPECT_EQ(interval.getNumSemitones(false), -8);

    // C chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Cbb3");
    EXPECT_EQ(interval.getNumSemitones(false), -14);
    interval.setNotes("C4", "Cb3");
    EXPECT_EQ(interval.getNumSemitones(false), -13);
    interval.setNotes("C4", "C3");
    EXPECT_EQ(interval.getNumSemitones(false), -12);
    interval.setNotes("C4", "C#3");
    EXPECT_EQ(interval.getNumSemitones(false), -11);
    interval.setNotes("C4", "Cx3");
    EXPECT_EQ(interval.getNumSemitones(false), -10);
}

TEST(getNumSemitones, absoluteValue_True_desc) {
    Interval interval;

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb3");
    EXPECT_EQ(interval.getNumSemitones(true), 3);
    interval.setNotes("C4", "Bb3");
    EXPECT_EQ(interval.getNumSemitones(true), 2);
    interval.setNotes("C4", "B3");
    EXPECT_EQ(interval.getNumSemitones(true), 1);
    interval.setNotes("C4", "B#3");
    EXPECT_EQ(interval.getNumSemitones(true), 0);
    interval.setNotes("C4", "Bx3");
    EXPECT_EQ(interval.getNumSemitones(true), 1);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb3");
    EXPECT_EQ(interval.getNumSemitones(true), 5);
    interval.setNotes("C4", "Ab3");
    EXPECT_EQ(interval.getNumSemitones(true), 4);
    interval.setNotes("C4", "A3");
    EXPECT_EQ(interval.getNumSemitones(true), 3);
    interval.setNotes("C4", "A#3");
    EXPECT_EQ(interval.getNumSemitones(true), 2);
    interval.setNotes("C4", "Ax3");
    EXPECT_EQ(interval.getNumSemitones(true), 1);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb3");
    EXPECT_EQ(interval.getNumSemitones(true), 7);
    interval.setNotes("C4", "Gb3");
    EXPECT_EQ(interval.getNumSemitones(true), 6);
    interval.setNotes("C4", "G3");
    EXPECT_EQ(interval.getNumSemitones(true), 5);
    interval.setNotes("C4", "G#3");
    EXPECT_EQ(interval.getNumSemitones(true), 4);
    interval.setNotes("C4", "Gx3");
    EXPECT_EQ(interval.getNumSemitones(true), 3);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb3");
    EXPECT_EQ(interval.getNumSemitones(true), 9);
    interval.setNotes("C4", "Fb3");
    EXPECT_EQ(interval.getNumSemitones(true), 8);
    interval.setNotes("C4", "F3");
    EXPECT_EQ(interval.getNumSemitones(true), 7);
    interval.setNotes("C4", "F#3");
    EXPECT_EQ(interval.getNumSemitones(true), 6);
    interval.setNotes("C4", "Fx3");
    EXPECT_EQ(interval.getNumSemitones(true), 5);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb3");
    EXPECT_EQ(interval.getNumSemitones(true), 10);
    interval.setNotes("C4", "Eb3");
    EXPECT_EQ(interval.getNumSemitones(true), 9);
    interval.setNotes("C4", "E3");
    EXPECT_EQ(interval.getNumSemitones(true), 8);
    interval.setNotes("C4", "E#3");
    EXPECT_EQ(interval.getNumSemitones(true), 7);
    interval.setNotes("C4", "Ex3");
    EXPECT_EQ(interval.getNumSemitones(true), 6);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb3");
    EXPECT_EQ(interval.getNumSemitones(true), 12);
    interval.setNotes("C4", "Db3");
    EXPECT_EQ(interval.getNumSemitones(true), 11);
    interval.setNotes("C4", "D3");
    EXPECT_EQ(interval.getNumSemitones(true), 10);
    interval.setNotes("C4", "D#3");
    EXPECT_EQ(interval.getNumSemitones(true), 9);
    interval.setNotes("C4", "Dx3");
    EXPECT_EQ(interval.getNumSemitones(true), 8);

    // C chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Cbb3");
    EXPECT_EQ(interval.getNumSemitones(true), 14);
    interval.setNotes("C4", "Cb3");
    EXPECT_EQ(interval.getNumSemitones(true), 13);
    interval.setNotes("C4", "C3");
    EXPECT_EQ(interval.getNumSemitones(true), 12);
    interval.setNotes("C4", "C#3");
    EXPECT_EQ(interval.getNumSemitones(true), 11);
    interval.setNotes("C4", "Cx3");
    EXPECT_EQ(interval.getNumSemitones(true), 10);
}

TEST(getDiatonicSteps, useSingleOctave_False_absoluteValue_False_asc) {
    Interval interval;

    // C chromatic pitchClass block
    interval.setNotes("C4", "Cbb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 0);
    interval.setNotes("C4", "Cb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 0);
    interval.setNotes("C4", "C4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 0);
    interval.setNotes("C4", "C#4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 0);
    interval.setNotes("C4", "Cx4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 0);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 1);
    interval.setNotes("C4", "Db4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 1);
    interval.setNotes("C4", "D4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 1);
    interval.setNotes("C4", "D#4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 1);
    interval.setNotes("C4", "Dx4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 1);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 2);
    interval.setNotes("C4", "Eb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 2);
    interval.setNotes("C4", "E4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 2);
    interval.setNotes("C4", "E#4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 2);
    interval.setNotes("C4", "Ex4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 2);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 3);
    interval.setNotes("C4", "Fb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 3);
    interval.setNotes("C4", "F4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 3);
    interval.setNotes("C4", "F#4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 3);
    interval.setNotes("C4", "Fx4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 3);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 4);
    interval.setNotes("C4", "Gb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 4);
    interval.setNotes("C4", "G4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 4);
    interval.setNotes("C4", "G#4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 4);
    interval.setNotes("C4", "Gx4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 4);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 5);
    interval.setNotes("C4", "Ab4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 5);
    interval.setNotes("C4", "A4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 5);
    interval.setNotes("C4", "A#4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 5);
    interval.setNotes("C4", "Ax4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 5);

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 6);
    interval.setNotes("C4", "Bb4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 6);
    interval.setNotes("C4", "B4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 6);
    interval.setNotes("C4", "B#4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 6);
    interval.setNotes("C4", "Bx4");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 6);

    // // C chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Cbb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 7);
    interval.setNotes("C4", "Cb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 7);
    interval.setNotes("C4", "C5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 7);
    interval.setNotes("C4", "C#5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 7);
    interval.setNotes("C4", "Cx5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 7);

    // D chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Dbb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 8);
    interval.setNotes("C4", "Db5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 8);
    interval.setNotes("C4", "D5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 8);
    interval.setNotes("C4", "D#5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 8);
    interval.setNotes("C4", "Dx5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 8);

    // E chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Ebb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 9);
    interval.setNotes("C4", "Eb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 9);
    interval.setNotes("C4", "E5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 9);
    interval.setNotes("C4", "E#5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 9);
    interval.setNotes("C4", "Ex5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 9);

    // F chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Fbb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 10);
    interval.setNotes("C4", "Fb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 10);
    interval.setNotes("C4", "F5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 10);
    interval.setNotes("C4", "F#5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 10);
    interval.setNotes("C4", "Fx5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 10);

    // G chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Gbb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 11);
    interval.setNotes("C4", "Gb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 11);
    interval.setNotes("C4", "G5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 11);
    interval.setNotes("C4", "G#5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 11);
    interval.setNotes("C4", "Gx5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 11);

    // A chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Abb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 12);
    interval.setNotes("C4", "Ab5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 12);
    interval.setNotes("C4", "A5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 12);
    interval.setNotes("C4", "A#5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 12);
    interval.setNotes("C4", "Ax5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 12);

    // B chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Bbb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 13);
    interval.setNotes("C4", "Bb5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 13);
    interval.setNotes("C4", "B5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 13);
    interval.setNotes("C4", "B#5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 13);
    interval.setNotes("C4", "Bx5");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 13);

    // C chromatic pitchClass block (octave above 2 times)
    interval.setNotes("C4", "Cbb6");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 14);
    interval.setNotes("C4", "Cb6");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 14);
    interval.setNotes("C4", "C6");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 14);
    interval.setNotes("C4", "C#6");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 14);
    interval.setNotes("C4", "Cx6");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), 14);
}

TEST(getDiatonicSteps, useSingleOctave_False_absoluteValue_False_desc) {
    Interval interval;

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -1);
    interval.setNotes("C4", "Bb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -1);
    interval.setNotes("C4", "B3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -1);
    interval.setNotes("C4", "B#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -1);
    interval.setNotes("C4", "Bx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -1);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -2);
    interval.setNotes("C4", "Ab3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -2);
    interval.setNotes("C4", "A3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -2);
    interval.setNotes("C4", "A#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -2);
    interval.setNotes("C4", "Ax3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -2);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -3);
    interval.setNotes("C4", "Gb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -3);
    interval.setNotes("C4", "G3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -3);
    interval.setNotes("C4", "G#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -3);
    interval.setNotes("C4", "Gx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -3);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -4);
    interval.setNotes("C4", "Fb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -4);
    interval.setNotes("C4", "F3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -4);
    interval.setNotes("C4", "F#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -4);
    interval.setNotes("C4", "Fx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -4);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -5);
    interval.setNotes("C4", "Eb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -5);
    interval.setNotes("C4", "E3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -5);
    interval.setNotes("C4", "E#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -5);
    interval.setNotes("C4", "Ex3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -5);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -6);
    interval.setNotes("C4", "Db3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -6);
    interval.setNotes("C4", "D3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -6);
    interval.setNotes("C4", "D#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -6);
    interval.setNotes("C4", "Dx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -6);

    // C chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Cbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -7);
    interval.setNotes("C4", "Cb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -7);
    interval.setNotes("C4", "C3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -7);
    interval.setNotes("C4", "C#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -7);
    interval.setNotes("C4", "Cx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -7);

    // B chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Bbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -8);
    interval.setNotes("C4", "Bb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -8);
    interval.setNotes("C4", "B2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -8);
    interval.setNotes("C4", "B#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -8);
    interval.setNotes("C4", "Bx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -8);

    // A chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Abb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -9);
    interval.setNotes("C4", "Ab2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -9);
    interval.setNotes("C4", "A2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -9);
    interval.setNotes("C4", "A#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -9);
    interval.setNotes("C4", "Ax2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -9);

    // G chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Gbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -10);
    interval.setNotes("C4", "Gb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -10);
    interval.setNotes("C4", "G2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -10);
    interval.setNotes("C4", "G#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -10);
    interval.setNotes("C4", "Gx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -10);

    // F chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Fbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -11);
    interval.setNotes("C4", "Fb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -11);
    interval.setNotes("C4", "F2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -11);
    interval.setNotes("C4", "F#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -11);
    interval.setNotes("C4", "Fx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -11);

    // E chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Ebb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -12);
    interval.setNotes("C4", "Eb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -12);
    interval.setNotes("C4", "E2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -12);
    interval.setNotes("C4", "E#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -12);
    interval.setNotes("C4", "Ex2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -12);

    // D chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Dbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -13);
    interval.setNotes("C4", "Db2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -13);
    interval.setNotes("C4", "D2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -13);
    interval.setNotes("C4", "D#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -13);
    interval.setNotes("C4", "Dx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -13);

    // C chromatic pitchClass block (octave below 2 times)
    interval.setNotes("C4", "Cbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -14);
    interval.setNotes("C4", "Cb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -14);
    interval.setNotes("C4", "C2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -14);
    interval.setNotes("C4", "C#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -14);
    interval.setNotes("C4", "Cx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, false), -14);
}

TEST(getDiatonicSteps, useSingleOctave_True_absoluteValue_False_asc) {
    Interval interval;

    // C chromatic pitchClass block
    interval.setNotes("C4", "Cbb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "Cb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "C4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "C#4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "Cx4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);
    interval.setNotes("C4", "Db4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);
    interval.setNotes("C4", "D4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);
    interval.setNotes("C4", "D#4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);
    interval.setNotes("C4", "Dx4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);
    interval.setNotes("C4", "Eb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);
    interval.setNotes("C4", "E4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);
    interval.setNotes("C4", "E#4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);
    interval.setNotes("C4", "Ex4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);
    interval.setNotes("C4", "Fb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);
    interval.setNotes("C4", "F4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);
    interval.setNotes("C4", "F#4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);
    interval.setNotes("C4", "Fx4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);
    interval.setNotes("C4", "Gb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);
    interval.setNotes("C4", "G4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);
    interval.setNotes("C4", "G#4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);
    interval.setNotes("C4", "Gx4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);
    interval.setNotes("C4", "Ab4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);
    interval.setNotes("C4", "A4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);
    interval.setNotes("C4", "A#4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);
    interval.setNotes("C4", "Ax4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);
    interval.setNotes("C4", "Bb4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);
    interval.setNotes("C4", "B4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);
    interval.setNotes("C4", "B#4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);
    interval.setNotes("C4", "Bx4");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);

    // // C chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Cbb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "Cb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "C5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "C#5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "Cx5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);

    // D chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Dbb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);
    interval.setNotes("C4", "Db5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);
    interval.setNotes("C4", "D5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);
    interval.setNotes("C4", "D#5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);
    interval.setNotes("C4", "Dx5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 1);

    // E chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Ebb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);
    interval.setNotes("C4", "Eb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);
    interval.setNotes("C4", "E5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);
    interval.setNotes("C4", "E#5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);
    interval.setNotes("C4", "Ex5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 2);

    // F chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Fbb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);
    interval.setNotes("C4", "Fb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);
    interval.setNotes("C4", "F5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);
    interval.setNotes("C4", "F#5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);
    interval.setNotes("C4", "Fx5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 3);

    // G chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Gbb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);
    interval.setNotes("C4", "Gb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);
    interval.setNotes("C4", "G5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);
    interval.setNotes("C4", "G#5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);
    interval.setNotes("C4", "Gx5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 4);

    // A chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Abb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);
    interval.setNotes("C4", "Ab5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);
    interval.setNotes("C4", "A5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);
    interval.setNotes("C4", "A#5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);
    interval.setNotes("C4", "Ax5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 5);

    // B chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Bbb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);
    interval.setNotes("C4", "Bb5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);
    interval.setNotes("C4", "B5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);
    interval.setNotes("C4", "B#5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);
    interval.setNotes("C4", "Bx5");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 6);

    // C chromatic pitchClass block (octave above 2 times)
    interval.setNotes("C4", "Cbb6");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "Cb6");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "C6");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "C#6");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
    interval.setNotes("C4", "Cx6");
    EXPECT_EQ(interval.getDiatonicSteps(true, false), 0);
}

TEST(getDiatonicSteps, useSingleOctave_False_absoluteValue_True_desc) {
    Interval interval;

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 1);
    interval.setNotes("C4", "Bb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 1);
    interval.setNotes("C4", "B3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 1);
    interval.setNotes("C4", "B#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 1);
    interval.setNotes("C4", "Bx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 1);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 2);
    interval.setNotes("C4", "Ab3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 2);
    interval.setNotes("C4", "A3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 2);
    interval.setNotes("C4", "A#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 2);
    interval.setNotes("C4", "Ax3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 2);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 3);
    interval.setNotes("C4", "Gb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 3);
    interval.setNotes("C4", "G3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 3);
    interval.setNotes("C4", "G#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 3);
    interval.setNotes("C4", "Gx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 3);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 4);
    interval.setNotes("C4", "Fb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 4);
    interval.setNotes("C4", "F3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 4);
    interval.setNotes("C4", "F#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 4);
    interval.setNotes("C4", "Fx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 4);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 5);
    interval.setNotes("C4", "Eb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 5);
    interval.setNotes("C4", "E3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 5);
    interval.setNotes("C4", "E#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 5);
    interval.setNotes("C4", "Ex3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 5);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 6);
    interval.setNotes("C4", "Db3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 6);
    interval.setNotes("C4", "D3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 6);
    interval.setNotes("C4", "D#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 6);
    interval.setNotes("C4", "Dx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 6);

    // C chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Cbb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 7);
    interval.setNotes("C4", "Cb3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 7);
    interval.setNotes("C4", "C3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 7);
    interval.setNotes("C4", "C#3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 7);
    interval.setNotes("C4", "Cx3");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 7);

    // B chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Bbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 8);
    interval.setNotes("C4", "Bb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 8);
    interval.setNotes("C4", "B2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 8);
    interval.setNotes("C4", "B#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 8);
    interval.setNotes("C4", "Bx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 8);

    // A chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Abb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 9);
    interval.setNotes("C4", "Ab2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 9);
    interval.setNotes("C4", "A2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 9);
    interval.setNotes("C4", "A#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 9);
    interval.setNotes("C4", "Ax2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 9);

    // G chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Gbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 10);
    interval.setNotes("C4", "Gb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 10);
    interval.setNotes("C4", "G2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 10);
    interval.setNotes("C4", "G#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 10);
    interval.setNotes("C4", "Gx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 10);

    // F chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Fbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 11);
    interval.setNotes("C4", "Fb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 11);
    interval.setNotes("C4", "F2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 11);
    interval.setNotes("C4", "F#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 11);
    interval.setNotes("C4", "Fx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 11);

    // E chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Ebb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 12);
    interval.setNotes("C4", "Eb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 12);
    interval.setNotes("C4", "E2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 12);
    interval.setNotes("C4", "E#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 12);
    interval.setNotes("C4", "Ex2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 12);

    // D chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Dbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 13);
    interval.setNotes("C4", "Db2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 13);
    interval.setNotes("C4", "D2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 13);
    interval.setNotes("C4", "D#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 13);
    interval.setNotes("C4", "Dx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 13);

    // C chromatic pitchClass block (octave below 2 times)
    interval.setNotes("C4", "Cbb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 14);
    interval.setNotes("C4", "Cb2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 14);
    interval.setNotes("C4", "C2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 14);
    interval.setNotes("C4", "C#2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 14);
    interval.setNotes("C4", "Cx2");
    EXPECT_EQ(interval.getDiatonicSteps(false, true), 14);
}

TEST(getDiatonicSteps, useSingleOctave_True_absoluteValue_True_desc) {
    Interval interval;

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);
    interval.setNotes("C4", "Bb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);
    interval.setNotes("C4", "B3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);
    interval.setNotes("C4", "B#3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);
    interval.setNotes("C4", "Bx3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);
    interval.setNotes("C4", "Ab3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);
    interval.setNotes("C4", "A3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);
    interval.setNotes("C4", "A#3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);
    interval.setNotes("C4", "Ax3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);
    interval.setNotes("C4", "Gb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);
    interval.setNotes("C4", "G3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);
    interval.setNotes("C4", "G#3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);
    interval.setNotes("C4", "Gx3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);
    interval.setNotes("C4", "Fb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);
    interval.setNotes("C4", "F3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);
    interval.setNotes("C4", "F#3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);
    interval.setNotes("C4", "Fx3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);
    interval.setNotes("C4", "Eb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);
    interval.setNotes("C4", "E3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);
    interval.setNotes("C4", "E#3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);
    interval.setNotes("C4", "Ex3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);
    interval.setNotes("C4", "Db3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);
    interval.setNotes("C4", "D3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);
    interval.setNotes("C4", "D#3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);
    interval.setNotes("C4", "Dx3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);

    // C chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Cbb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
    interval.setNotes("C4", "Cb3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
    interval.setNotes("C4", "C3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
    interval.setNotes("C4", "C#3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
    interval.setNotes("C4", "Cx3");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);

    // B chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Bbb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);
    interval.setNotes("C4", "Bb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);
    interval.setNotes("C4", "B2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);
    interval.setNotes("C4", "B#2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);
    interval.setNotes("C4", "Bx2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 1);

    // A chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Abb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);
    interval.setNotes("C4", "Ab2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);
    interval.setNotes("C4", "A2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);
    interval.setNotes("C4", "A#2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);
    interval.setNotes("C4", "Ax2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 2);

    // G chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Gbb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);
    interval.setNotes("C4", "Gb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);
    interval.setNotes("C4", "G2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);
    interval.setNotes("C4", "G#2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);
    interval.setNotes("C4", "Gx2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 3);

    // F chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Fbb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);
    interval.setNotes("C4", "Fb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);
    interval.setNotes("C4", "F2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);
    interval.setNotes("C4", "F#2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);
    interval.setNotes("C4", "Fx2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 4);

    // E chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Ebb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);
    interval.setNotes("C4", "Eb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);
    interval.setNotes("C4", "E2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);
    interval.setNotes("C4", "E#2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);
    interval.setNotes("C4", "Ex2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 5);

    // D chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Dbb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);
    interval.setNotes("C4", "Db2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);
    interval.setNotes("C4", "D2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);
    interval.setNotes("C4", "D#2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);
    interval.setNotes("C4", "Dx2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 6);

    // C chromatic pitchClass block (octave below 2 times)
    interval.setNotes("C4", "Cbb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
    interval.setNotes("C4", "Cb2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
    interval.setNotes("C4", "C2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
    interval.setNotes("C4", "C#2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
    interval.setNotes("C4", "Cx2");
    EXPECT_EQ(interval.getDiatonicSteps(true, true), 0);
}

// ===== getDiatonicInterval =====

TEST(getDiatonicInterval, useSingleOctave_False_absoluteValue_False_asc) {
    Interval interval;

    // C chromatic pitchClass block
    interval.setNotes("C4", "Cbb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 1);
    interval.setNotes("C4", "Cb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 1);
    interval.setNotes("C4", "C4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 1);
    interval.setNotes("C4", "C#4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 1);
    interval.setNotes("C4", "Cx4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 1);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 2);
    interval.setNotes("C4", "Db4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 2);
    interval.setNotes("C4", "D4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 2);
    interval.setNotes("C4", "D#4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 2);
    interval.setNotes("C4", "Dx4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 2);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 3);
    interval.setNotes("C4", "Eb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 3);
    interval.setNotes("C4", "E4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 3);
    interval.setNotes("C4", "E#4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 3);
    interval.setNotes("C4", "Ex4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 3);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 4);
    interval.setNotes("C4", "Fb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 4);
    interval.setNotes("C4", "F4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 4);
    interval.setNotes("C4", "F#4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 4);
    interval.setNotes("C4", "Fx4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 4);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 5);
    interval.setNotes("C4", "Gb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 5);
    interval.setNotes("C4", "G4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 5);
    interval.setNotes("C4", "G#4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 5);
    interval.setNotes("C4", "Gx4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 5);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 6);
    interval.setNotes("C4", "Ab4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 6);
    interval.setNotes("C4", "A4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 6);
    interval.setNotes("C4", "A#4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 6);
    interval.setNotes("C4", "Ax4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 6);

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 7);
    interval.setNotes("C4", "Bb4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 7);
    interval.setNotes("C4", "B4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 7);
    interval.setNotes("C4", "B#4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 7);
    interval.setNotes("C4", "Bx4");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 7);

    // // C chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Cbb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 8);
    interval.setNotes("C4", "Cb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 8);
    interval.setNotes("C4", "C5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 8);
    interval.setNotes("C4", "C#5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 8);
    interval.setNotes("C4", "Cx5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 8);

    // D chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Dbb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 9);
    interval.setNotes("C4", "Db5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 9);
    interval.setNotes("C4", "D5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 9);
    interval.setNotes("C4", "D#5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 9);
    interval.setNotes("C4", "Dx5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 9);

    // E chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Ebb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 10);
    interval.setNotes("C4", "Eb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 10);
    interval.setNotes("C4", "E5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 10);
    interval.setNotes("C4", "E#5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 10);
    interval.setNotes("C4", "Ex5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 10);

    // F chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Fbb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 11);
    interval.setNotes("C4", "Fb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 11);
    interval.setNotes("C4", "F5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 11);
    interval.setNotes("C4", "F#5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 11);
    interval.setNotes("C4", "Fx5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 11);

    // G chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Gbb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 12);
    interval.setNotes("C4", "Gb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 12);
    interval.setNotes("C4", "G5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 12);
    interval.setNotes("C4", "G#5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 12);
    interval.setNotes("C4", "Gx5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 12);

    // A chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Abb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 13);
    interval.setNotes("C4", "Ab5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 13);
    interval.setNotes("C4", "A5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 13);
    interval.setNotes("C4", "A#5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 13);
    interval.setNotes("C4", "Ax5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 13);

    // B chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Bbb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 14);
    interval.setNotes("C4", "Bb5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 14);
    interval.setNotes("C4", "B5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 14);
    interval.setNotes("C4", "B#5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 14);
    interval.setNotes("C4", "Bx5");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 14);

    // C chromatic pitchClass block (octave above 2 times)
    interval.setNotes("C4", "Cbb6");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 15);
    interval.setNotes("C4", "Cb6");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 15);
    interval.setNotes("C4", "C6");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 15);
    interval.setNotes("C4", "C#6");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 15);
    interval.setNotes("C4", "Cx6");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 15);
}

TEST(getDiatonicInterval, useSingleOctave_False_absoluteValue_False_desc) {
    Interval interval;

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -2);
    interval.setNotes("C4", "Bb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -2);
    interval.setNotes("C4", "B3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -2);
    interval.setNotes("C4", "B#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -2);
    interval.setNotes("C4", "Bx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -2);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -3);
    interval.setNotes("C4", "Ab3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -3);
    interval.setNotes("C4", "A3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -3);
    interval.setNotes("C4", "A#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -3);
    interval.setNotes("C4", "Ax3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -3);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -4);
    interval.setNotes("C4", "Gb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -4);
    interval.setNotes("C4", "G3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -4);
    interval.setNotes("C4", "G#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -4);
    interval.setNotes("C4", "Gx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -4);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -5);
    interval.setNotes("C4", "Fb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -5);
    interval.setNotes("C4", "F3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -5);
    interval.setNotes("C4", "F#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -5);
    interval.setNotes("C4", "Fx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -5);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -6);
    interval.setNotes("C4", "Eb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -6);
    interval.setNotes("C4", "E3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -6);
    interval.setNotes("C4", "E#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -6);
    interval.setNotes("C4", "Ex3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -6);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -7);
    interval.setNotes("C4", "Db3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -7);
    interval.setNotes("C4", "D3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -7);
    interval.setNotes("C4", "D#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -7);
    interval.setNotes("C4", "Dx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -7);

    // C chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Cbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -8);
    interval.setNotes("C4", "Cb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -8);
    interval.setNotes("C4", "C3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -8);
    interval.setNotes("C4", "C#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -8);
    interval.setNotes("C4", "Cx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -8);

    // B chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Bbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -9);
    interval.setNotes("C4", "Bb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -9);
    interval.setNotes("C4", "B2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -9);
    interval.setNotes("C4", "B#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -9);
    interval.setNotes("C4", "Bx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -9);

    // A chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Abb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -10);
    interval.setNotes("C4", "Ab2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -10);
    interval.setNotes("C4", "A2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -10);
    interval.setNotes("C4", "A#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -10);
    interval.setNotes("C4", "Ax2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -10);

    // G chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Gbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -11);
    interval.setNotes("C4", "Gb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -11);
    interval.setNotes("C4", "G2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -11);
    interval.setNotes("C4", "G#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -11);
    interval.setNotes("C4", "Gx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -11);

    // F chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Fbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -12);
    interval.setNotes("C4", "Fb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -12);
    interval.setNotes("C4", "F2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -12);
    interval.setNotes("C4", "F#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -12);
    interval.setNotes("C4", "Fx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -12);

    // E chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Ebb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -13);
    interval.setNotes("C4", "Eb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -13);
    interval.setNotes("C4", "E2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -13);
    interval.setNotes("C4", "E#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -13);
    interval.setNotes("C4", "Ex2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -13);

    // D chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Dbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -14);
    interval.setNotes("C4", "Db2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -14);
    interval.setNotes("C4", "D2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -14);
    interval.setNotes("C4", "D#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -14);
    interval.setNotes("C4", "Dx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -14);

    // C chromatic pitchClass block (octave below 2 times)
    interval.setNotes("C4", "Cbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -15);
    interval.setNotes("C4", "Cb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -15);
    interval.setNotes("C4", "C2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -15);
    interval.setNotes("C4", "C#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -15);
    interval.setNotes("C4", "Cx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), -15);
}

TEST(getDiatonicInterval, useSingleOctave_True_absoluteValue_False_asc) {
    Interval interval;

    // C chromatic pitchClass block
    interval.setNotes("C4", "Cbb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "Cb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "C4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "C#4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "Cx4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);
    interval.setNotes("C4", "Db4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);
    interval.setNotes("C4", "D4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);
    interval.setNotes("C4", "D#4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);
    interval.setNotes("C4", "Dx4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);
    interval.setNotes("C4", "Eb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);
    interval.setNotes("C4", "E4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);
    interval.setNotes("C4", "E#4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);
    interval.setNotes("C4", "Ex4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);
    interval.setNotes("C4", "Fb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);
    interval.setNotes("C4", "F4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);
    interval.setNotes("C4", "F#4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);
    interval.setNotes("C4", "Fx4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);
    interval.setNotes("C4", "Gb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);
    interval.setNotes("C4", "G4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);
    interval.setNotes("C4", "G#4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);
    interval.setNotes("C4", "Gx4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);
    interval.setNotes("C4", "Ab4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);
    interval.setNotes("C4", "A4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);
    interval.setNotes("C4", "A#4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);
    interval.setNotes("C4", "Ax4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);
    interval.setNotes("C4", "Bb4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);
    interval.setNotes("C4", "B4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);
    interval.setNotes("C4", "B#4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);
    interval.setNotes("C4", "Bx4");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);

    // // C chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Cbb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "Cb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "C5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "C#5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "Cx5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);

    // D chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Dbb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);
    interval.setNotes("C4", "Db5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);
    interval.setNotes("C4", "D5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);
    interval.setNotes("C4", "D#5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);
    interval.setNotes("C4", "Dx5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 2);

    // E chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Ebb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);
    interval.setNotes("C4", "Eb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);
    interval.setNotes("C4", "E5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);
    interval.setNotes("C4", "E#5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);
    interval.setNotes("C4", "Ex5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 3);

    // F chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Fbb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);
    interval.setNotes("C4", "Fb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);
    interval.setNotes("C4", "F5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);
    interval.setNotes("C4", "F#5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);
    interval.setNotes("C4", "Fx5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 4);

    // G chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Gbb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);
    interval.setNotes("C4", "Gb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);
    interval.setNotes("C4", "G5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);
    interval.setNotes("C4", "G#5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);
    interval.setNotes("C4", "Gx5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 5);

    // A chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Abb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);
    interval.setNotes("C4", "Ab5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);
    interval.setNotes("C4", "A5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);
    interval.setNotes("C4", "A#5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);
    interval.setNotes("C4", "Ax5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 6);

    // B chromatic pitchClass block (octave above)
    interval.setNotes("C4", "Bbb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);
    interval.setNotes("C4", "Bb5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);
    interval.setNotes("C4", "B5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);
    interval.setNotes("C4", "B#5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);
    interval.setNotes("C4", "Bx5");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 7);

    // C chromatic pitchClass block (octave above 2 times)
    interval.setNotes("C4", "Cbb6");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "Cb6");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "C6");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "C#6");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
    interval.setNotes("C4", "Cx6");
    EXPECT_EQ(interval.getDiatonicInterval(true, false), 1);
}

TEST(getDiatonicInterval, useSingleOctave_False_absoluteValue_True_desc) {
    Interval interval;

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 2);
    interval.setNotes("C4", "Bb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 2);
    interval.setNotes("C4", "B3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 2);
    interval.setNotes("C4", "B#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 2);
    interval.setNotes("C4", "Bx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 2);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 3);
    interval.setNotes("C4", "Ab3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 3);
    interval.setNotes("C4", "A3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 3);
    interval.setNotes("C4", "A#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 3);
    interval.setNotes("C4", "Ax3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 3);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 4);
    interval.setNotes("C4", "Gb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 4);
    interval.setNotes("C4", "G3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 4);
    interval.setNotes("C4", "G#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 4);
    interval.setNotes("C4", "Gx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 4);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 5);
    interval.setNotes("C4", "Fb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 5);
    interval.setNotes("C4", "F3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 5);
    interval.setNotes("C4", "F#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 5);
    interval.setNotes("C4", "Fx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 5);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 6);
    interval.setNotes("C4", "Eb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 6);
    interval.setNotes("C4", "E3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 6);
    interval.setNotes("C4", "E#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 6);
    interval.setNotes("C4", "Ex3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 6);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 7);
    interval.setNotes("C4", "Db3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 7);
    interval.setNotes("C4", "D3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 7);
    interval.setNotes("C4", "D#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 7);
    interval.setNotes("C4", "Dx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 7);

    // C chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Cbb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 8);
    interval.setNotes("C4", "Cb3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 8);
    interval.setNotes("C4", "C3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 8);
    interval.setNotes("C4", "C#3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 8);
    interval.setNotes("C4", "Cx3");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 8);

    // B chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Bbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 9);
    interval.setNotes("C4", "Bb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 9);
    interval.setNotes("C4", "B2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 9);
    interval.setNotes("C4", "B#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 9);
    interval.setNotes("C4", "Bx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 9);

    // A chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Abb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 10);
    interval.setNotes("C4", "Ab2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 10);
    interval.setNotes("C4", "A2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 10);
    interval.setNotes("C4", "A#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 10);
    interval.setNotes("C4", "Ax2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 10);

    // G chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Gbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 11);
    interval.setNotes("C4", "Gb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 11);
    interval.setNotes("C4", "G2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 11);
    interval.setNotes("C4", "G#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 11);
    interval.setNotes("C4", "Gx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 11);

    // F chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Fbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 12);
    interval.setNotes("C4", "Fb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 12);
    interval.setNotes("C4", "F2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 12);
    interval.setNotes("C4", "F#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 12);
    interval.setNotes("C4", "Fx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 12);

    // E chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Ebb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 13);
    interval.setNotes("C4", "Eb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 13);
    interval.setNotes("C4", "E2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 13);
    interval.setNotes("C4", "E#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 13);
    interval.setNotes("C4", "Ex2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 13);

    // D chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Dbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 14);
    interval.setNotes("C4", "Db2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 14);
    interval.setNotes("C4", "D2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 14);
    interval.setNotes("C4", "D#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 14);
    interval.setNotes("C4", "Dx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 14);

    // C chromatic pitchClass block (octave below 2 times)
    interval.setNotes("C4", "Cbb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 15);
    interval.setNotes("C4", "Cb2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 15);
    interval.setNotes("C4", "C2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 15);
    interval.setNotes("C4", "C#2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 15);
    interval.setNotes("C4", "Cx2");
    EXPECT_EQ(interval.getDiatonicInterval(false, true), 15);
}

TEST(getDiatonicInterval, useSingleOctave_True_absoluteValue_True_desc) {
    Interval interval;

    // B chromatic pitchClass block
    interval.setNotes("C4", "Bbb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);
    interval.setNotes("C4", "Bb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);
    interval.setNotes("C4", "B3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);
    interval.setNotes("C4", "B#3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);
    interval.setNotes("C4", "Bx3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);

    // A chromatic pitchClass block
    interval.setNotes("C4", "Abb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);
    interval.setNotes("C4", "Ab3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);
    interval.setNotes("C4", "A3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);
    interval.setNotes("C4", "A#3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);
    interval.setNotes("C4", "Ax3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);

    // G chromatic pitchClass block
    interval.setNotes("C4", "Gbb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);
    interval.setNotes("C4", "Gb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);
    interval.setNotes("C4", "G3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);
    interval.setNotes("C4", "G#3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);
    interval.setNotes("C4", "Gx3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);

    // F chromatic pitchClass block
    interval.setNotes("C4", "Fbb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);
    interval.setNotes("C4", "Fb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);
    interval.setNotes("C4", "F3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);
    interval.setNotes("C4", "F#3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);
    interval.setNotes("C4", "Fx3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);

    // E chromatic pitchClass block
    interval.setNotes("C4", "Ebb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);
    interval.setNotes("C4", "Eb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);
    interval.setNotes("C4", "E3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);
    interval.setNotes("C4", "E#3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);
    interval.setNotes("C4", "Ex3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);

    // D chromatic pitchClass block
    interval.setNotes("C4", "Dbb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);
    interval.setNotes("C4", "Db3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);
    interval.setNotes("C4", "D3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);
    interval.setNotes("C4", "D#3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);
    interval.setNotes("C4", "Dx3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);

    // C chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Cbb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
    interval.setNotes("C4", "Cb3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
    interval.setNotes("C4", "C3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
    interval.setNotes("C4", "C#3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
    interval.setNotes("C4", "Cx3");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);

    // B chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Bbb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);
    interval.setNotes("C4", "Bb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);
    interval.setNotes("C4", "B2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);
    interval.setNotes("C4", "B#2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);
    interval.setNotes("C4", "Bx2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 2);

    // A chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Abb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);
    interval.setNotes("C4", "Ab2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);
    interval.setNotes("C4", "A2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);
    interval.setNotes("C4", "A#2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);
    interval.setNotes("C4", "Ax2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 3);

    // G chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Gbb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);
    interval.setNotes("C4", "Gb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);
    interval.setNotes("C4", "G2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);
    interval.setNotes("C4", "G#2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);
    interval.setNotes("C4", "Gx2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 4);

    // F chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Fbb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);
    interval.setNotes("C4", "Fb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);
    interval.setNotes("C4", "F2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);
    interval.setNotes("C4", "F#2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);
    interval.setNotes("C4", "Fx2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 5);

    // E chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Ebb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);
    interval.setNotes("C4", "Eb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);
    interval.setNotes("C4", "E2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);
    interval.setNotes("C4", "E#2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);
    interval.setNotes("C4", "Ex2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 6);

    // D chromatic pitchClass block (octave below)
    interval.setNotes("C4", "Dbb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);
    interval.setNotes("C4", "Db2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);
    interval.setNotes("C4", "D2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);
    interval.setNotes("C4", "D#2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);
    interval.setNotes("C4", "Dx2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 7);

    // C chromatic pitchClass block (octave below 2 times)
    interval.setNotes("C4", "Cbb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
    interval.setNotes("C4", "Cb2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
    interval.setNotes("C4", "C2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
    interval.setNotes("C4", "C#2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
    interval.setNotes("C4", "Cx2");
    EXPECT_EQ(interval.getDiatonicInterval(true, true), 1);
}

TEST(PitchSpellingInterval, DirectionOutsidePianoRange) {
    Interval interval;

    interval.setNotes("C-1", "D-1");
    EXPECT_TRUE(interval.isAscendant());
    EXPECT_EQ(interval.getDirection(), "asc");
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 2);

    interval.setNotes("B10", "C11");
    EXPECT_TRUE(interval.isAscendant());
    EXPECT_EQ(interval.getDiatonicInterval(false, false), 2);

    interval.setNotes("C11", "C-1");
    EXPECT_TRUE(interval.isDescendant());
    EXPECT_EQ(interval.getDirection(), "desc");

    interval.setNotes("B9", "C10");
    EXPECT_TRUE(interval.isAscendant());
}

// =====================================================================================
// TASK 6 (T6): NOTE COMPOSES PITCH -- INTERVAL REST GUARDS STILL REJECT RESTS
// =====================================================================================

TEST(IntervalRestGuard, ConstructorRejectsRestOnEitherSide) {
    EXPECT_THROW(Interval("C4", "rest"), std::runtime_error);
    EXPECT_THROW(Interval("rest", "C4"), std::runtime_error);
}

TEST(IntervalRestGuard, SetNotesRejectsRestOnEitherSide) {
    Interval interval;
    EXPECT_THROW(interval.setNotes("C4", "rest"), std::runtime_error);
    EXPECT_THROW(interval.setNotes("rest", "C4"), std::runtime_error);
}

// =====================================================================================
// TASK 9: INTERVAL REJECTS QUARTER TONES AT CONSTRUCTION
//
// Every name, quality, classification and semitone count this class computes is defined over
// twelve-tone equal temperament, so a quarter tone would not make the analysis fail -- it would
// make it return a confident wrong answer. Rather than guarding 85 public methods, the two
// Note-taking entry points reject a quarter tone and the two string-taking ones delegate to
// them, so an Interval carrying a quarter tone can never be constructed in the first place.
// =====================================================================================

TEST(IntervalQuarterToneGuard, ConstructorRejectsQuarterToneOnEitherSide) {
    EXPECT_THROW(Interval("C4", "E1b4"), std::runtime_error);
    EXPECT_THROW(Interval("E1b4", "C4"), std::runtime_error);
    EXPECT_THROW(Interval(Note("C4"), Note("D3b4")), std::runtime_error);
    EXPECT_THROW(Interval(Note("D3b4"), Note("C4")), std::runtime_error);
}

TEST(IntervalQuarterToneGuard, SetNotesRejectsQuarterToneOnEitherSide) {
    Interval interval;
    EXPECT_THROW(interval.setNotes("C4", "E1b4"), std::runtime_error);
    EXPECT_THROW(interval.setNotes("E1b4", "C4"), std::runtime_error);
    EXPECT_THROW(interval.setNotes(Note("C4"), Note("C1x4")), std::runtime_error);
    EXPECT_THROW(interval.setNotes(Note("C1x4"), Note("C4")), std::runtime_error);
}

TEST(IntervalQuarterToneGuard, ErrorMessageNamesTheNoteAndTheRemedy) {
    // A caller hitting this must be able to learn what to do from the message alone. Interval
    // has no escape hatch of its own, so the message names Note::roundToSemitone(), the per-note
    // equivalent of Chord::roundQuarterTones().
    try {
        Interval("C4", "E1b4");
        FAIL() << "Interval did not throw on a quarter-tone pitch";
    } catch (const std::runtime_error& error) {
        const std::string message(error.what());
        EXPECT_NE(message.find("E1b4"), std::string::npos) << message;
        EXPECT_NE(message.find("roundToSemitone"), std::string::npos) << message;
    }
}

TEST(IntervalQuarterToneGuard, WholeToneIntervalsAreUnaffectedAndTheRemedyWorks) {
    EXPECT_NO_THROW(Interval("C4", "E4"));
    EXPECT_EQ(Interval("C4", "E4").getNumSemitones(false), 4);

    // The remedy the error message names actually makes the interval constructible.
    Note rounded("E1b4");
    rounded.roundToSemitone();
    EXPECT_NO_THROW(Interval(Note("C4"), rounded));
    EXPECT_EQ(Interval(Note("C4"), rounded).getNumSemitones(false), 4);
}

// ===== Notes of transposing instruments ===== //

namespace {
// Every answer an Interval gives about its two notes, as one string.
std::string describe(const Interval& interval) {
    return interval.getName() + " " + interval.getDirection() + " | semitones " +
           std::to_string(interval.getNumSemitones()) + " | octaves " +
           std::to_string(interval.getNumOctaves()) + " | diatonic interval " +
           std::to_string(interval.getDiatonicInterval(false, false)) + " | diatonic steps " +
           std::to_string(interval.getDiatonicSteps(false, false)) + " | pitch-step interval " +
           std::to_string(interval.getPitchStepInterval());
}
}  // namespace

// A B-flat clarinet's written D4 sounds C4: against a violin's C4 it is a unison, not the major
// second the two parts show.
TEST(IntervalOfTransposingInstruments, aBFlatClarinetsWrittenD4AgainstAViolinsC4IsAUnison) {
    const Interval interval(bFlatClarinet("D4"), Note("C4"));
    EXPECT_EQ(interval.getName(), "P1");
    EXPECT_EQ(interval.getDirection(), "");
    EXPECT_EQ(interval.getNumSemitones(), 0);
    EXPECT_EQ(interval.getNumOctaves(), 0);
    EXPECT_EQ(interval.getPitchStepInterval(), 1);
}

// A transposed note is related by the pitch it sounds, spelled with its written letter moved by
// the diatonic transposing interval: every answer is the one the untransposed notes at those
// pitches give.
TEST(IntervalOfTransposingInstruments, isTheIntervalOfThePitchesTheNotesSound) {
    // The two notes, the pitches they sound, and the interval's name and direction.
    const std::vector<std::tuple<Note, Note, std::string, std::string, std::string>> cases = {
        {Note("C4"), bFlatClarinet("F#4"), "C4", "E4", "M3 asc"},
        {Note("C4"), hornInF("B4"), "C4", "E4", "M3 asc"},
        {Note("F#4"), bFlatClarinet("G#4"), "F#4", "F#4", "P1 "},
        {hornInF("C#5"), Note("E4"), "F#4", "E4", "M2 desc"},
        {Note("Bb5"), piccolo("Bb4"), "Bb5", "Bb5", "P1 "},
        {bFlatClarinet("Db4"), Note("Cb5"), "Cb4", "Cb5", "P8 asc"},
        {bFlatClarinet("F#4"), hornInF("B4"), "E4", "E4", "P1 "},
        {Note("C4"), transposingNote("F#4", 0, -2), "C4", "E4", "M3 asc"},
        {transposingNote("Db4", 0, -2), Note("Cb5"), "Cb4", "Cb5", "P8 asc"},
    };
    for (const auto& [noteA, noteB, soundsA, soundsB, name] : cases) {
        const std::string where = noteA.getWrittenPitch() + " (" +
                                  std::to_string(noteA.getTransposeChromatic()) + ") to " +
                                  noteB.getWrittenPitch() + " (" +
                                  std::to_string(noteB.getTransposeChromatic()) + ")";
        const Interval interval(noteA, noteB);
        EXPECT_EQ(interval.getName() + " " + interval.getDirection(), name) << where;
        EXPECT_EQ(describe(interval), describe(Interval(soundsA, soundsB))) << where;
    }
}

// A transposing interval given only in semitones is read with the diatonic interval
// conventionally written for them: two semitones down are a major second, so a written F#4 on a
// (0, -2) instrument is an E4, a unison with E4 rather than a diminished second, and two notes of
// that part are named by their written interval: its written C4 and F#4, sounding Bb3 and E4, are
// an augmented fourth.
TEST(IntervalOfTransposingInstruments, aChromaticIntervalAloneIsReadWithItsDiatonicInterval) {
    const Note written = transposingNote("F#4", 0, -2);
    const Interval unison(Note("E4"), written);
    EXPECT_EQ(unison.getName(), "P1");
    EXPECT_EQ(unison.getDirection(), "");
    EXPECT_EQ(unison.getNumSemitones(), 0);
    EXPECT_EQ(describe(unison), describe(Interval("E4", "E4")));

    const Interval withinThePart(transposingNote("C4", 0, -2), written);
    EXPECT_EQ(withinThePart.getName() + " " + withinThePart.getDirection(), "A4 asc");
    EXPECT_EQ(describe(withinThePart), describe(Interval("Bb3", "E4")));
}

// The interval keeps the notes it was given, with their written pitches and transposing
// intervals: only what it computes from them is taken at the pitches they sound.
TEST(IntervalOfTransposingInstruments, keepsTheNotesItWasGiven) {
    const std::vector<Note> notes = Interval(bFlatClarinet("D4"), Note("C4")).getNotes();
    ASSERT_EQ(notes.size(), 2u);
    EXPECT_EQ(notes[0].getWrittenPitch(), "D4");
    EXPECT_EQ(notes[0].getTransposeDiatonic(), -1);
    EXPECT_EQ(notes[0].getTransposeChromatic(), -2);
    EXPECT_EQ(notes[1].getWrittenPitch(), "C4");
}

// An interval this class has no name for is reported with the pitches it related: a B-flat
// clarinet's written A#4 sounds G#4, and Gb4 to G#4 has no name.
TEST(IntervalOfTransposingInstruments, anIntervalWithoutANameIsReportedWithThePitchesItRelated) {
    const std::string message =
        thrownFirstLine([] { Interval(Note("Gb4"), bFlatClarinet("A#4")).getName(); });
    EXPECT_EQ(message, "[maiacore] Unable to compute the interval [Gb4, G#4]");
}

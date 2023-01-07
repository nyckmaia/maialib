#include "maiacore/interval.h"

#include <gtest/gtest.h>

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

  // C chromatic pitchClass block (octave above)
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

  // C chromatic pitchClass block (octave above)
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

  // C chromatic pitchClass block (octave above)
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

  // C chromatic pitchClass block (octave above)
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

// ===== getDiatonicInterval ===== //

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

  // C chromatic pitchClass block (octave above)
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

  // C chromatic pitchClass block (octave above)
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
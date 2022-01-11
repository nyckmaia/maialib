#include <gtest/gtest.h>
#include "note.h"
#include "chord.h"

using namespace testing;

TEST(getName, majorTriad01)
{
    Note a("C4");
    Note b("E4");
    Note c("G4");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);

    EXPECT_EQ(myChord.getName(), "C");
}

TEST(getName, majorTriad02)
{
    Note a("C4");
    Note b("D7");
    Note c("E4");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);

    EXPECT_EQ(myChord.getName(), "C9");
}

TEST(getName, majorWithBass)
{
    Note a("C4");
    Note b("E4");
    Note c("G3");
    Note d("B4");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);
    myChord.addNote(d);

    EXPECT_EQ(myChord.getName(), "C7M/G");
}

TEST(getName, majorMinorSeventh)
{
    Note a("E5");
    Note b("C5");
    Note c("G3");
    Note d("Bb3");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);
    myChord.addNote(d);

    EXPECT_EQ(myChord.getName(), "C7/G");
}

TEST(getName, majorMajorSeventh)
{
    Note a("C");
    Note b("E");
    Note c("G");
    Note d("B");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);
    myChord.addNote(d);

    EXPECT_EQ(myChord.getName(), "C7M");
}

TEST(getName, minorTriad01)
{
    Note a("A4");
    Note b("C5");
    Note c("E7");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);

    EXPECT_EQ(myChord.getName(), "Am");
}

TEST(getName, minorTriad02)
{
    Note a("C5");
    Note b("Eb5");
    Note c("G3");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);

    EXPECT_EQ(myChord.getName(), "Cm/G");
}

TEST(getName, minorSeventhWithBass)
{
    Note a("A5");
    Note b("C5");
    Note c("E7");
    Note d("G3");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);
    myChord.addNote(d);

    EXPECT_EQ(myChord.getName(), "Am7/G");
}

TEST(isTonal, tonalMajorChord01)
{
    Note a("C");
    Note b("E");
    Note c("G");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);

    EXPECT_EQ(myChord.isTonal(), true);
}

TEST(isTonal, tonalMinorChord01)
{
    Note a("D");
    Note b("F");
    Note c("A");

    Chord myChord;

    myChord.addNote(a);
    myChord.addNote(b);
    myChord.addNote(c);

    EXPECT_EQ(myChord.isTonal(), true);
}

TEST(isTonal, notTonalMinorChord01)
{
    Chord myChord({Note("D"), Note("F"), Note("A#")});


    EXPECT_EQ(myChord.isTonal(), false);
}

TEST(chordOperator, plus)
{
    Note a("C");
    Note b("E");

    Chord myChord1;
    myChord1.addNote(a);
    myChord1.addNote(b);

    Note c("G");
    Note d("B");

    Chord myChord2;
    myChord2.addNote(c);
    myChord2.addNote(d);

    Chord myChord3 = myChord1 + myChord2;

    EXPECT_EQ(myChord3.size(), myChord1.size() + myChord2.size());
}

TEST(chordOperator, StreamLeft)
{
    Note a("C");
    Note b("E");

    Chord myChord1;
    myChord1 << a;
    myChord1 << b;

    Chord myChord2;
    myChord2.addNote(a);
    myChord2.addNote(b);

    EXPECT_EQ(myChord1, myChord2);
}

TEST(chordOperator, StreamRight)
{
    Note a("C");
    Note b("E");
    Note c("G");

    Chord myChord1;
    myChord1.addNote(a);
    myChord1.addNote(b);
    myChord1.addNote(c);

    Note x;
    myChord1 >> x;

    EXPECT_EQ(myChord1.size(), 2);
    EXPECT_EQ(x.getPitch(), c.getPitch());
}

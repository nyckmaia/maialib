#include <gtest/gtest.h>
#include "score.h"

using namespace testing;

void myFunc(Note& note)
{
    std::cout << note.getPitch() << std::endl;
}

TEST(nyckSuite01, myTestExample01)
{
    Score score = Score("./tests/xml_examples/unit_test/test_chord.xml");

    //score.forEachNote(myFunc, 0, -1, {"Flute"});

    EXPECT_EQ(score.isValid(), true);
}
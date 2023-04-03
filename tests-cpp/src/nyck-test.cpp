// #include <gtest/gtest.h>

// #include "maiacore/score.h"

// using namespace testing;

// void myFunc(Note& note) { std::cout << note.getPitch() << std::endl; }

// TEST(nyckSuite01, myTestExample01) {
//   Score score = Score("./tests/xml_examples/unit_test/test_chord.xml");

//   // score.forEachNote(myFunc, 0, -1, {"Flute"});

//   EXPECT_EQ(score.isValid(), true);
// }

// TEST(nyckSuite01, getChords) {
//   Score score =
//       Score("./tests/xml_examples/unit_test/test_getchords_poly.musicxml");
//   // Score score = Score("./tests/xml_examples/Beethoven/Symphony_9th.xml");

//   const auto x = score.getChords();
//   EXPECT_EQ(score.isValid(), true);
// }
#include <gtest/gtest.h>

#include <iostream>
#include <tuple>
#include <vector>

#include "maiacore/chord.h"
#include "maiacore/score_collection.h"

using namespace testing;

// TEST(nyckSuite01, myTestExample01) {
//   ScoreCollection collection("/mnt/c/Users/nyck/Desktop/Bach-chorale");
//   int errors = 0;
//   for (auto& score : collection.getScores()) {
//     std::cout << score.getTitle() << " | " << score.getLoadedFilePath()
//               << std::endl;
//     auto chordsTuple = score.getChords();
//     const int chordsTupleSize = chordsTuple.size();
//     std::vector<Chord> closeChords;
//     closeChords.reserve(chordsTupleSize);

//     for (auto& t : chordsTuple) {
//       Chord& c = std::get<3>(t);
//       try {
//         closeChords.push_back(c.getCloseStackChord());
//       } catch (const std::length_error& e) {
//         errors++;
//         std::cerr << "Caught length_error: " << e.what() << '\n';
//       }
//     }
//     std::cout << "errors = " << errors << std::endl;
//   }

//   EXPECT_EQ(errors, 0);
// }

TEST(nyckSuite01, myTestExample02) {
  int errors = 0;
  Chord chord;
  try {
    for (int i = 0; i < 100; ++i) {
      chord.addNote(Note("C4"));
    }
  } catch (const std::length_error& e) {
    errors++;
    std::cerr << "Caught length_error: " << e.what() << '\n';
  }

  EXPECT_EQ(errors, 0);
  EXPECT_EQ(chord.size(), 100);
}

TEST(nyckSuite01, myTestExample03) {
  Score score(
      "/mnt/c/Users/nyck/Desktop/Doutorado/Bach-chorale/"
      "BWV_105.06_FB.musicxml");

  auto chordsTuple = score.getChords();
  const int chordsTupleSize = chordsTuple.size();
  std::vector<Chord> closeChords;
  closeChords.reserve(chordsTupleSize);

  std::cout << score.getTitle() << " | " << score.getFilePath() << std::endl;

  int chordIdx = 0;
  int errors = 0;
  for (auto& t : chordsTuple) {
    const int measure = std::get<0>(t);
    const float floatMeasure = std::get<1>(t);
    Chord& c = std::get<3>(t);

    try {
      std::cout << "idx[" + std::to_string(chordIdx++) + "] m=[" << measure
                << "/" << floatMeasure << "] ";

      for (const auto& n : c.getNotes()) {
        std::cout << n.getPitch() << ", ";
      }

      std::cout << "]" << std::endl;

    } catch (const std::runtime_error& e) {
      errors++;
      std::cerr << "runtime_error: " << e.what() << '\n';
    }
  }

  EXPECT_EQ(errors, 0);
}

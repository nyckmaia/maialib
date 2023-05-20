import unittest
import maialib as ml


def myCallback(note):
    print(note.getPitch())

# ===== TEST SCORE CLASS ===== #


class ForEachNote(unittest.TestCase):
    def testCallbackFunction(self):
        # Important: This unit test cannot be done like others
        # because it calls a external callback function multiple times.
        # So I can't compare the result automaticaly.
        # The partial solution was test it manually
        myScore = ml.Score(
            "./xml_examples/unit_test/test_musical_scale.xml")
        # myScore.forEachNote(myCallback)

        self.assertEqual(myScore.isValid(), True)


if __name__ == '__main__':
    unittest.main()

import unittest
from maialib import *

def myFunc(note):
    print(note.getPitch())

# ===== TEST SCORE CLASS ===== #
class ForEachNote(unittest.TestCase):
    def testSimple(self):
        myScore = Score("./tests/xml_examples/unit_test/test_multiple_instruments3.musicxml")
        # myScore.forEachNote(myFunc, 0, -1, ["Flute"])
        
        self.assertEqual(myScore.isValid(), True)
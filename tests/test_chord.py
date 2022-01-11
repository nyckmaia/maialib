import unittest
from maialib import *

# ===== TEST CHORD CLASS ===== #
class GetName(unittest.TestCase):
    def testMajorTriad01(self):
        a = Note("C4")
        b = Note("E4")
        c = Note("G4")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)

        self.assertEqual(myChord.getName(), "C")
    
    def testMajorTriad02(self):
        a = Note("C4")
        b = Note("D7")
        c = Note("E4")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)

        self.assertEqual(myChord.getName(), "C9")
    
    def testMajorWithBass(self):
        a = Note("C4")
        b = Note("E4")
        c = Note("G3")
        d = Note("B4")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)
        myChord.addNote(d)

        self.assertEqual(myChord.getName(), "C7M/G")
    
    def testMajorMinorSeventh(self):
        a = Note("E5")
        b = Note("C5")
        c = Note("G3")
        d = Note("Bb3")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)
        myChord.addNote(d)

        self.assertEqual(myChord.getName(), "C7/G")
    
    def testMajorMajorSeventh(self):
        a = Note("C")
        b = Note("E")
        c = Note("G")
        d = Note("B")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)
        myChord.addNote(d)

        self.assertEqual(myChord.getName(), "C7M")
    
    def testMinorTriad01(self):
        a = Note("A4")
        b = Note("C5")
        c = Note("E7")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)

        self.assertEqual(myChord.getName(), "Am")
    
    def testMinorTriad02(self):
        a = Note("C5")
        b = Note("Eb5")
        c = Note("G3")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)

        self.assertEqual(myChord.getName(), "Cm/G")
    
    def testMinorSeventhWithBass(self):
        a = Note("A5")
        b = Note("C5")
        c = Note("E7")
        d = Note("G3")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)
        myChord.addNote(d)

        self.assertEqual(myChord.getName(), "Am7/G")

class IsTonal(unittest.TestCase):
    def testTonalMajorChord01(self):
        a = Note("D")
        b = Note("F")
        c = Note("A")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)

        self.assertEqual(myChord.isTonal(), True)
    
    def testNotTonalMajorChord01(self):
        a = Note("D")
        b = Note("F")
        c = Note("A#")

        myChord = Chord()

        myChord.addNote(a)
        myChord.addNote(b)
        myChord.addNote(c)

        self.assertEqual(myChord.isTonal(), False)
    
class chordOperator(unittest.TestCase):
    def testPlus(self):
        a = Note("C")
        b = Note("E")

        myChord1 = Chord()
        myChord1.addNote(a)
        myChord1.addNote(b)
        
        c = Note("G")
        d = Note("B")
        
        myChord2 = Chord()
        myChord2.addNote(c)
        myChord2.addNote(d)
        
        myChord3 = myChord1 + myChord2
        
        self.assertEqual(myChord3.size(), myChord1.size() + myChord2.size())
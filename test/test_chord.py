import unittest
import maialib as ml

# ===== TEST CHORD CLASS ===== #


class GetName(unittest.TestCase):
    def testMajorTriad01(self):
        myChord = ml.Chord(["C4", "E4", "G4"])
        self.assertEqual(myChord.getName(), "C")

    def testMajorTriad02(self):
        myChord = ml.Chord(["C4", "D7", "G6", "E4"])
        self.assertEqual(myChord.getName(), "C9")

    def testMajorWithBass(self):
        myChord = ml.Chord(["C4", "E4", "G3", "B4"])
        self.assertEqual(myChord.getName(), "C7M/G")

    def testMajorMinorSeventh(self):
        myChord = ml.Chord(["E5", "C5", "G3", "Bb3"])
        self.assertEqual(myChord.getName(), "C7/G")

    def testMajorMajorSeventh(self):
        myChord = ml.Chord(["C", "E", "G", "B"])
        self.assertEqual(myChord.getName(), "C7M")

    def testMinorTriad01(self):
        myChord = ml.Chord(["A4", "C5", "E7"])
        self.assertEqual(myChord.getName(), "Am")

    def testMinorTriad02(self):
        myChord = ml.Chord(["C5", "Eb5", "G3"])
        self.assertEqual(myChord.getName(), "Cm/G")

    def testMinorSeventhWithBass(self):
        myChord = ml.Chord(["A5", "C5", "E7", "G3"])
        self.assertEqual(myChord.getName(), "Am7/G")


class IsTonal(unittest.TestCase):
    def testTonalMajorChord01(self):
        myChord = ml.Chord(["C", "E", "G"])
        self.assertEqual(myChord.isTonal(), True)

    def testTonalMinorChord01(self):
        myChord = ml.Chord(["D", "F", "A"])
        self.assertEqual(myChord.isTonal(), True)

    def testNotTonalMajorChord01(self):
        myChord = ml.Chord(["D", "F", "A#"])
        self.assertEqual(myChord.isTonal(), False)


class GetCloseStackChord(unittest.TestCase):
    def testMajorAndMinorChords(self):
        # Exact the same chords
        chord01_test = ml.Chord(["C4", "E4", "G4", "B4"])
        chord01_answ = ml.Chord(["C4", "E4", "G4", "B4"])
        self.assertEqual(chord01_test.getCloseStackChord(
        ).getNotes(), chord01_answ.getNotes())

        # Different note sequency
        chord02_test = ml.Chord(["Eb4", "C4", "B4", "G4"])
        chord02_answ = ml.Chord(["C4", "Eb4", "G4", "B4"])
        self.assertEqual(chord02_test.getCloseStackChord(
        ).getNotes(), chord02_answ.getNotes())

        # Both different note sequency and octave
        chord03_test = ml.Chord(["Eb3", "C5", "B2", "G9"])
        chord03_answ = ml.Chord(["C4", "Eb4", "G4", "B4"])
        self.assertEqual(chord03_test.getCloseStackChord(
        ).getNotes(), chord03_answ.getNotes())

    def testComplexChords(self):
        # # Inverted major-seven-nine (5th ommited) chord
        # chord01_test = ml.Chord(["D2", "C4", "E5", "B4"])
        # chord01_answ = ml.Chord(["C4", "E4", "B4", "D5"])
        # self.assertEqual(chord01_test.getCloseStackChord().getNotes(), chord01_answ.getNotes())

        # # Inverted minor-seven-nine (5th ommited) chord
        # chord02_test = ml.Chord(["F2", "E5", "D3", "C4"])
        # chord02_answ = ml.Chord(["D4", "F4", "C5", "E5"])
        # self.assertEqual(chord02_test.getCloseStackChord().getNotes(), chord02_answ.getNotes())

        # # Inverted minor-seven-eleven (5th ommited) chord
        # chord03_test = ml.Chord(["F2", "G5", "D3", "C4"])
        # chord03_answ = ml.Chord(["D4", "F4", "C5", "G5"])
        # self.assertEqual(chord03_test.getCloseStackChord().getNotes(), chord03_answ.getNotes())

        # Four notes chromatic cluster
        chord04_test = ml.Chord(["C4", "C#4", "D4", "D#4"])
        # [C4, Eb4, <G4>, Bx4, D5] => Cm(7aug)9
        chord04_answ = ml.Chord(["C4", "Eb4", "Bx4", "D5"])
        self.assertEqual(chord04_test.getCloseStackChord(
        ).getNotes(), chord04_answ.getNotes())

        # Five notes diatonic cluster
        chord05a_test = ml.Chord(["C4", "E4", "G4", "B4", "D5"])
        chord05a_answ = ml.Chord(["C4", "E4", "G4", "B4", "D5"])
        self.assertEqual(chord05a_test.getCloseStackChord(
        ).getNotes(), chord05a_answ.getNotes())

        # Five notes diatonic cluster
        chord05b_test = ml.Chord(["C4", "Ebb7", "Fx5", "Ax3", "E4"])
        chord05b_answ = ml.Chord(["Ax4", "C5", "E5", "G5", "D6"])
        self.assertEqual(chord05b_test.getCloseStackChord(
        ).getNotes(), chord05b_answ.getNotes())

        # Five notes diatonic cluster
        chord05c_test = ml.Chord(["C4", "G4", "A4", "E4", "B5"])
        chord05c_answ = ml.Chord(["A4", "C5", "E5", "G5", "B5"])
        self.assertEqual(chord05c_test.getCloseStackChord(
        ).getNotes(), chord05c_answ.getNotes())

        # Five notes diatonic cluster
        chord05d_test = ml.Chord(["D3", "C4", "E4", "G7", "B5"])
        chord05d_answ = ml.Chord(["C4", "E4", "G4", "B4", "D5"])
        self.assertEqual(chord05d_test.getCloseStackChord(
        ).getNotes(), chord05d_answ.getNotes())

        # Five notes diatonic cluster
        chord05e_test = ml.Chord(["C4", "Ab8", "Eb3", "G4", "B5"])
        chord05e_answ = ml.Chord(["Ab4", "C5", "Eb5", "G5", "B5"])
        self.assertEqual(chord05e_test.getCloseStackChord(
        ).getNotes(), chord05e_answ.getNotes())

        # Six notes diatonic cluster
        chord06_test = ml.Chord(["C4", "E4", "G4", "B4", "D5", "F5"])
        chord06_answ = ml.Chord(["C4", "E4", "G4", "B4", "D5", "F5"])
        self.assertEqual(chord06_test.getCloseStackChord(
        ).getNotes(), chord06_answ.getNotes())

        # Six notes diatonic cluster
        chord06b_test = ml.Chord(["C4", "D4", "E4", "F4", "G4", "B4"])
        chord06b_answ = ml.Chord(["C4", "E4", "G4", "B4", "D5", "F5"])
        self.assertEqual(chord06b_test.getCloseStackChord(
        ).getNotes(), chord06b_answ.getNotes())


class ChordOperator(unittest.TestCase):
    def testPlus(self):
        myChord1 = ml.Chord(["C", "E", "G"])
        myChord2 = ml.Chord(["B", "D", "F"])

        myChord3 = myChord1 + myChord2

        self.assertEqual(myChord3.size(), myChord1.size() + myChord2.size())


if __name__ == '__main__':
    unittest.main()

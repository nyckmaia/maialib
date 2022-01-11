import unittest
import io
from contextlib import redirect_stderr
from maialib import *

def isFloatEq(x, y, epslon = 1E-3):
    return abs(x - y) < epslon

# ===== TEST MAIALIB FUNCTIONS ===== #
class midiNote2freq(unittest.TestCase):
    def testMidiValuesTable(self):

        # Special Cases
        self.assertEqual(isFloatEq(Helper.midiNote2freq(-1), 0.00), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(0), 8.175), True)

        # Piano First Octave: From A0 to A1
        self.assertEqual(isFloatEq(Helper.midiNote2freq(21), 27.500), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(22), 29.135), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(23), 30.868), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(24), 32.703), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(25), 34.648), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(26), 36.708), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(27), 38.891), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(28), 41.203), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(29), 43.654), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(30), 46.249), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(31), 48.999), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(32), 51.913), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(33), 55.000), True)

        # Piano Middle Octave: From C4 to C5 
        self.assertEqual(isFloatEq(Helper.midiNote2freq(60), 261.626), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(61), 277.183), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(62), 293.665), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(63), 311.127), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(64), 329.628), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(65), 349.228), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(66), 369.994), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(67), 391.995), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(68), 415.305), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(69), 440.000), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(70), 466.164), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(71), 493.883), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(72), 523.251), True)

        # Piano Higher Octave: From C7 to C8 
        self.assertEqual(isFloatEq(Helper.midiNote2freq(96), 2093.005), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(97), 2217.461), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(98), 2349.318), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(99), 2489.016), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(100), 2637.020), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(101), 2793.826), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(102), 2959.955), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(103), 3135.963), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(104), 3322.438), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(105), 3520.000), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(106), 3729.310), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(107), 3951.066), True)
        self.assertEqual(isFloatEq(Helper.midiNote2freq(108), 4186.009), True)

class Freq2midiNote(unittest.TestCase):
    def OctavesValues(self):
        indexes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
        for i in enumerate(indexes):
            result = Helper.freq2midiNote((pow(2,i)) * 110.0)
            self.assertEqual(result.first, 45 + (12 * i))
            self.assertEqual(result.second, 0)

    def centsValues(self):
        result01 = Helper.freq2midiNote(30.1) # base frequency = 29.135, base midiNote = 22
        self.assertEqual(result01.first, 23)
        self.assertEqual(result01.second, -44)
    
        result02 = Helper.freq2midiNote(202.0) # base frequency = 195.998, base midiNote = 55
        self.assertEqual(result02.first, 56)
        self.assertEqual(result02.second, -48)
    
        result03 = Helper.freq2midiNote(481.0) # base frequency = 466.164, base midiNote = 70
        self.assertEqual(result03.first, 71)
        self.assertEqual(result03.second, -46)
    
        result04 = Helper.freq2midiNote(1712.0) # base frequency = 1661.219, base midiNote = 92
        self.assertEqual(result04.first, 93)
        self.assertEqual(result04.second, -48)
    
        result05 = Helper.freq2midiNote(3270.0) # base frequency = 3135.963, base midiNote = 103
        self.assertEqual(result05.first, 104)
        self.assertEqual(result05.second, -28)
    
        result06 = Helper.freq2midiNote(4080.0) # base frequency =3951.056, base midiNote = 107
        self.assertEqual(result06.first, 108)
        self.assertEqual(result06.second, -44)
    
        # b) Testing for frequencies with values slighter smaller than the base frequency.
        # the output note must be the base MidiNote -1. So cents are positive and smaller than 50.
    
        result07 = Helper.freq2midiNote(28.0) # base frequency = 29.135, base midiNote = 22
        self.assertEqual(result07.first, 21)
        self.assertEqual(result07.second, 31)
    
        result08 = Helper.freq2midiNote(189.0) # base frequency = 195.998, base midiNote = 55
        self.assertEqual(result08.first, 54)
        self.assertEqual(result08.second, 37)
    
        result09 = Helper.freq2midiNote(450.0) # base frequency = 466.164, base midiNote = 70
        self.assertEqual(result09.first, 69)
        self.assertEqual(result09.second, 39)
    
        result10 = Helper.freq2midiNote(1605.0) # base frequency = 1661.219, base midiNote = 92
        self.assertEqual(result10.first, 91)
        self.assertEqual(result10.second, 40)
    
        result11 = Helper.freq2midiNote(3035.0) # base frequency = 3135.963, base midiNote = 103
        self.assertEqual(result11.first, 102)
        self.assertEqual(result11.second, 43)
    
        result12 = Helper.freq2midiNote(3828.0) # base frequency =3951.056, base midiNote = 107
        self.assertEqual(result12.first, 106)
        self.assertEqual(result12.second, 45)

class midiNote2pitch(unittest.TestCase):
    def testNegativeValue_restCase(self):
        # Any negative number - rest
        self.assertEqual(Helper.midiNote2pitch(-1, "bb"), "rest")
        self.assertEqual(Helper.midiNote2pitch(-1, "b"),  "rest")
        self.assertEqual(Helper.midiNote2pitch(-1),       "rest")
        self.assertEqual(Helper.midiNote2pitch(-1, "#"),  "rest")
        self.assertEqual(Helper.midiNote2pitch(-1, "x"),  "rest")
    
    def testTwelveTonesOctave4(self):
        # MIDI Note 60 - C4
        self.assertEqual(Helper.midiNote2pitch(60, "bb"), "Dbb4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(60, "b"),  "")
        self.assertIn("[ERROR] The MIDI Note '60' cannot be wrote using 'b' accident type\n", buf.getvalue())
        
        self.assertEqual(Helper.midiNote2pitch(60),       "C4")
        self.assertEqual(Helper.midiNote2pitch(60, "#"),  "B#3")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(60, "x"),  "")
        self.assertIn("[ERROR] The MIDI Note '60' cannot be wrote using 'x' accident type\n", buf.getvalue())

        # MIDI Note 61 - C#4
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(61, "bb"), "")
        self.assertIn("[ERROR] The MIDI Note '61' cannot be wrote using 'bb' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(61, "b"),  "Db4")
        self.assertEqual(Helper.midiNote2pitch(61),       "C#4")
        self.assertEqual(Helper.midiNote2pitch(61, "#"),  "C#4")
        self.assertEqual(Helper.midiNote2pitch(61, "x"),  "Bx3")

        # MIDI Note 62 - D4
        self.assertEqual(Helper.midiNote2pitch(62, "bb"), "Ebb4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(62, "b"),  "")
        self.assertIn("[ERROR] The MIDI Note '62' cannot be wrote using 'b' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(62),       "D4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(62, "#"),  "")
        self.assertIn("[ERROR] The MIDI Note '62' cannot be wrote using '#' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(62, "x"),  "Cx4")

        # MIDI Note 63 - D#4
        self.assertEqual(Helper.midiNote2pitch(63, "bb"), "Fbb4")
        self.assertEqual(Helper.midiNote2pitch(63, "b"),  "Eb4")
        self.assertEqual(Helper.midiNote2pitch(63),       "D#4")
        self.assertEqual(Helper.midiNote2pitch(63, "#"),  "D#4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(63, "x"),  "")
        self.assertIn("[ERROR] The MIDI Note '63' cannot be wrote using 'x' accident type\n", buf.getvalue())

        # MIDI Note 64 - E4
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(64, "bb"), "")
        self.assertIn("[ERROR] The MIDI Note '64' cannot be wrote using 'bb' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(64, "b"),  "Fb4")
        self.assertEqual(Helper.midiNote2pitch(64),       "E4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(64, "#"),  "")
        self.assertIn("[ERROR] The MIDI Note '64' cannot be wrote using '#' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(64, "x"),  "Dx4")

        # MIDI Note 65 - F4
        self.assertEqual(Helper.midiNote2pitch(65, "bb"), "Gbb4")

        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(65, "b"),  "")
        self.assertIn("[ERROR] The MIDI Note '65' cannot be wrote using 'b' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(65),       "F4")
        self.assertEqual(Helper.midiNote2pitch(65, "#"),  "E#4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(65, "x"),  "")
        self.assertIn("[ERROR] The MIDI Note '65' cannot be wrote using 'x' accident type\n", buf.getvalue())

        # MIDI Note 66 - F#4
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(66, "bb"), "")
        self.assertIn("[ERROR] The MIDI Note '66' cannot be wrote using 'bb' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(66, "b"),  "Gb4")
        self.assertEqual(Helper.midiNote2pitch(66),       "F#4")
        self.assertEqual(Helper.midiNote2pitch(66, "#"),  "F#4")
        self.assertEqual(Helper.midiNote2pitch(66, "x"),  "Ex4")

        # MIDI Note 67 - G4
        self.assertEqual(Helper.midiNote2pitch(67, "bb"), "Abb4")

        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(67, "b"),  "")
        self.assertIn("[ERROR] The MIDI Note '67' cannot be wrote using 'b' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(67),       "G4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(67, "#"),  "")
        self.assertIn("[ERROR] The MIDI Note '67' cannot be wrote using '#' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(67, "x"),  "Fx4")

        # MIDI Note 68 - G#4
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(68, "bb"), "")
        self.assertIn("[ERROR] The MIDI Note '68' cannot be wrote using 'bb' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(68, "b"),  "Ab4")
        self.assertEqual(Helper.midiNote2pitch(68),       "G#4")
        self.assertEqual(Helper.midiNote2pitch(68, "#"),  "G#4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(68, "x"),  "")
        self.assertIn("[ERROR] The MIDI Note '68' cannot be wrote using 'x' accident type\n", buf.getvalue())

        # MIDI Note 69 - A4
        self.assertEqual(Helper.midiNote2pitch(69, "bb"), "Bbb4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(69, "b"),  "")
        self.assertIn("[ERROR] The MIDI Note '69' cannot be wrote using 'b' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(69),       "A4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(69, "#"),  "")
        self.assertIn("[ERROR] The MIDI Note '69' cannot be wrote using '#' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(69, "x"),  "Gx4")

        # MIDI Note 70 - A#4
        self.assertEqual(Helper.midiNote2pitch(70, "bb"), "Cbb5")
        self.assertEqual(Helper.midiNote2pitch(70, "b"),  "Bb4")
        self.assertEqual(Helper.midiNote2pitch(70),       "A#4")
        self.assertEqual(Helper.midiNote2pitch(70, "#"),  "A#4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(70, "x"),  "")
        self.assertIn("[ERROR] The MIDI Note '70' cannot be wrote using 'x' accident type\n", buf.getvalue())

        # MIDI Note 71 - B4
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(71, "bb"), "")
        self.assertIn("[ERROR] The MIDI Note '71' cannot be wrote using 'bb' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(71, "b"),  "Cb5")
        self.assertEqual(Helper.midiNote2pitch(71),       "B4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(71, "#"),  "")
        self.assertIn("[ERROR] The MIDI Note '71' cannot be wrote using '#' accident type\n", buf.getvalue())

        self.assertEqual(Helper.midiNote2pitch(71, "x"),  "Ax4")

        # MIDI Note 72 - C5
        self.assertEqual(Helper.midiNote2pitch(72, "bb"), "Dbb5")
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(72, "b"),  "")
        self.assertIn("[ERROR] The MIDI Note '72' cannot be wrote using 'b' accident type\n", buf.getvalue())
        self.assertEqual(Helper.midiNote2pitch(72),       "C5")
        self.assertEqual(Helper.midiNote2pitch(72, "#"),  "B#4")
        
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.assertEqual(Helper.midiNote2pitch(72, "x"),  "")
        self.assertIn("[ERROR] The MIDI Note '72' cannot be wrote using 'x' accident type\n", buf.getvalue())
        
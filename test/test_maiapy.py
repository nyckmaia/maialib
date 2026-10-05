import unittest

import maialib as ml


def transposingScore():
    """One measure of two transposing parts sounding together: a B-flat clarinet's written D4,
    which sounds C4, and a horn in F's written D5, which sounds G4. A label that showed the
    written pitch would read D4 and D5."""
    score = ml.Score(["Clarinet in Bb", "Horn in F"], 1)
    for index, (written, diatonic, chromatic) in enumerate((("D4", -1, -2), ("D5", -4, -7))):
        note = ml.Note(written, transposeDiatonic=diatonic, transposeChromatic=chromatic)
        score.getPart(index).getMeasure(0).addNote(note)
    return score


class SoundingPitchLabels(unittest.TestCase):
    """maiapy's plots label a note with getSoundingPitch(), what it sounds, never with the pitch
    its part writes. Each test reads the plot's data, which the figure displays; nothing is
    rendered."""

    def testThePianoRollLabelsEachNoteWithWhatItSounds(self):
        _, data = ml.plotPianoRoll(transposingScore())
        self.assertEqual(list(data["notePitch"]), ["C4", "G4"])

    def testThePitchEnvelopeLabelsItsLowestAndHighestNotesWithWhatTheySound(self):
        _, data = ml.plotScorePitchEnvelope(transposingScore())
        self.assertEqual(list(data["lowPitch"]), ["C4"])
        self.assertEqual(list(data["highPitch"]), ["G4"])

    def testTheDissonancePlotListsAChordsNotesAsTheySound(self):
        _, data = ml.plotScoreSetharesDissonance(transposingScore())
        self.assertEqual(list(data["chordNotes"]), ["C4, G4"])


class OctaveDoublingInThePianoRoll(unittest.TestCase):
    """The piano roll draws a doubled note twice: at what it sounds, and one octave below or
    above."""

    def testThePianoRollDrawsTheDoubledOctave(self):
        score = ml.Score(["Violoncello and Contrabass", "Flute and Piccolo"], 1)
        cello = ml.Note("C3")
        cello.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        score.getPart(0).getMeasure(0).addNote(cello)
        flute = ml.Note("G4")
        flute.setOctaveDoubling(ml.OctaveDoubling.ABOVE)
        score.getPart(1).getMeasure(0).addNote(flute)
        _, data = ml.plotPianoRoll(score)
        self.assertEqual(list(data["notePitch"]), ["C3", "C2", "G4", "G5"])
        self.assertEqual(list(data["midiValue"]), [48, 36, 67, 79])

    def testThePartsActivityDrawsEachNoteOnce(self):
        score = ml.Score(["Violoncello and Contrabass"], 1)
        cello = ml.Note("C3")
        cello.setOctaveDoubling(ml.OctaveDoubling.BELOW)
        score.getPart(0).getMeasure(0).addNote(cello)
        _, data = ml.plotPartsActivity(score)
        self.assertEqual(1, len(data))


if __name__ == "__main__":
    unittest.main()

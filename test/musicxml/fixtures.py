"""Small MusicXML documents shared by the tests of the MusicXML tools."""

# The MusicXML "Hello World": one whole-note C4 in 4/4, valid against the 4.0 schema. Each piece
# the tests replace appears exactly once.
MINIMAL_SCORE = (
    b'<?xml version="1.0" encoding="UTF-8"?>\n'
    b'<score-partwise version="4.0">'
    b'<part-list><score-part id="P1"><part-name>Music</part-name></score-part></part-list>'
    b'<part id="P1"><measure number="1">'
    b"<attributes><divisions>1</divisions><key><fifths>0</fifths></key>"
    b"<time><beats>4</beats><beat-type>4</beat-type></time>"
    b"<clef><sign>G</sign><line>2</line></clef></attributes>"
    b"<note><pitch><step>C</step><octave>4</octave></pitch>"
    b"<duration>4</duration><type>whole</type></note>"
    b"</measure></part>"
    b"</score-partwise>\n"
)

# The META-INF/container.xml of an .mxl archive whose score is "score.musicxml".
CONTAINER = (
    b'<?xml version="1.0" encoding="UTF-8"?>\n'
    b"<container><rootfiles>"
    b'<rootfile full-path="score.musicxml" media-type="application/vnd.recordare.musicxml+xml"/>'
    b"</rootfiles></container>\n"
)

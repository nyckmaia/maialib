# W3C MusicXML test suite

The MusicXML files here, with `LICENSE` and `README.md`, are the unmodified contents of
https://github.com/w3c-cg/musicxmlTestSuite at commit `77c19f7e819154c70ca1a1992e80dcda8ff82fea`
(retrieved 2026-10-01): the W3C Music Notation Community Group's copy of Reinhold Kainhofer's
LilyPond MusicXML test suite, released under the MIT License (see `LICENSE`).

Files whose name contains `.invalid` are deliberately invalid MusicXML. `MANIFEST.sha256` pins
every file and the tests check it, so the files must stay byte-identical; `.gitattributes` marks
the folder `-text`. maialib uses them only in its tests; they are not part of the maialib package.

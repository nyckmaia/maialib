# MusicXML 4.0 schema

These are the unmodified XML Schema files of **MusicXML 4.0**, the Final Community Group Report
of the W3C Music Notation Community Group (1 June 2021, https://www.w3.org/2021/06/musicxml40/).

- Source: https://github.com/w3c-cg/musicxml, tag `v4.0`
  (commit `799e2defb2ece0ae7bafe08dcbcac25b2c631d53`), folder `schema/`.
- `musicxml.xsd`, `xlink.xsd`, `container.xsd` and `catalog.xml`: Copyright © 2004-2021 the
  Contributors to the MusicXML Specification, published by the W3C Music Notation Community Group
  under the W3C Community Final Specification Agreement (FSA),
  https://www.w3.org/community/about/agreements/final/.
- `xml.xsd`: the W3C schema for the XML namespace, identical to
  http://www.w3.org/2007/08/xml.xsd; Copyright © World Wide Web Consortium,
  https://www.w3.org/copyright/document-license-2023/.

maialib uses these files only in its tests, to validate MusicXML documents
(`test/musicxml/musicxml_check.py`); they are not part of the maialib package. The tests pin each
file's SHA-256, so the files must stay byte-identical; `.gitattributes` marks the folder `-text`.

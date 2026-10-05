#include "melodic-lines.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "maiacore/log.h"
#include "maiacore/measure.h"
#include "maiacore/part.h"

namespace maiacore::detail {

namespace {
// Whether the note's tie list holds a tie stop: the note continues a note tied to it.
bool stopsATie(const Note& note) {
    const std::vector<std::string> ties = note.getTie();
    return std::find(ties.begin(), ties.end(), "stop") != ties.end();
}

// Whether 'next' extends the event that 'previous' stands for: both are notes at the same
// sounding exact position, and 'next' stops a tie.
bool continuesTheTie(const Note& previous, const Note& next) {
    return previous.isNoteOn() && next.isNoteOn() && stopsATie(next) &&
           previous.getQuarterToneSteps() == next.getQuarterToneSteps();
}

// Adds the duration of 'tied' to 'note', in ticks at the least common multiple of their
// divisions per quarter note and 1024, so the sum is exact. Duration names a tick count by a
// rhythm figure whose length is the divisions times a power of two down to 1/256, then adds half
// of that length per dot: with a multiple of 1024 divisions that half is never 0, so the sum of
// two tuplet notes, which no longer carries their tuplet ratio, still gets a figure.
void addDuration(Note& note, const Note& tied) {
    const int noteDivisions = note.getDivisionsPerQuarterNote();
    const int tiedDivisions = tied.getDivisionsPerQuarterNote();
    const int divisions = std::lcm(std::lcm(noteDivisions, tiedDivisions), 1024);
    const int ticks = note.getDurationTicks() * (divisions / noteDivisions) +
                      tied.getDurationTicks() * (divisions / tiedDivisions);
    note.setDuration(ticks, divisions);
}

// Lets a chord note stand for its event when it sounds higher than the note that does.
void joinChord(MelodicEvent& event, const Note& chordNote) {
    if (!chordNote.isNoteOn()) {
        return;
    }
    if (event.note.isNoteOff() ||
        chordNote.getQuarterToneSteps() > event.note.getQuarterToneSteps()) {
        event.note = chordNote;
    }
}

// The events of a line with every tied continuation folded into the event it continues.
std::vector<MelodicEvent> mergeTies(std::vector<MelodicEvent> events) {
    std::vector<MelodicEvent> merged;
    merged.reserve(events.size());
    for (MelodicEvent& event : events) {
        if (!merged.empty() && continuesTheTie(merged.back().note, event.note)) {
            addDuration(merged.back().note, event.note);
            continue;
        }
        merged.push_back(std::move(event));
    }
    return merged;
}
}  // namespace

std::vector<MelodicLine> melodicLines(const std::vector<Part>& parts) {
    std::vector<MelodicLine> lines;
    for (int p = 0; p < static_cast<int>(parts.size()); p++) {
        const Part& part = parts[p];
        // The part's lines keyed by (staff, voice): std::map lists them by staff, then by voice.
        std::map<std::pair<int, int>, MelodicLine> partLines;
        for (int m = 0; m < part.getNumMeasures(); m++) {
            const Measure& measure = part.getMeasure(m);
            for (int s = 0; s < measure.getNumStaves(); s++) {
                // The line of the last note written on this staff that started an event: the
                // chord notes written after it join that event.
                MelodicLine* open = nullptr;
                for (int n = 0; n < measure.getNumNotes(s); n++) {
                    const Note& note = measure.getNote(n, s);
                    if (note.isGraceNote()) {
                        continue;
                    }
                    if (note.inChord()) {
                        if (open != nullptr && open->voice == note.getVoice()) {
                            joinChord(open->events.back(), note);
                        }
                        continue;
                    }
                    MelodicLine& line = partLines[{s, note.getVoice()}];
                    line.partIdx = p;
                    line.staff = s;
                    line.voice = note.getVoice();
                    line.events.push_back({note, m});
                    open = &line;
                }
            }
        }
        for (auto& entry : partLines) {
            entry.second.events = mergeTies(std::move(entry.second.events));
            lines.push_back(std::move(entry.second));
        }
    }
    return lines;
}

void requireTwoNotes(const std::string& method, const size_t numNotes) {
    if (numNotes < 2) {
        LOG_ERROR(method + ": a melody pattern needs at least 2 notes, and this one has " +
                  std::to_string(numNotes));
    }
}

}  // namespace maiacore::detail

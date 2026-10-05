#pragma once

#include <string>
#include <vector>

#include "maiacore/note.h"

class Part;

// The melodic lines that the melody search reads. Internal to maiacore: this header lives next to
// the sources, is not among the public headers, and nothing here is bound to Python.
namespace maiacore::detail {

/**
 * @brief One event of a melodic line: a note, a chord or a rest, with the notes tied to it.
 */
struct MelodicEvent {
    /**
     * @brief The note that stands for the event: the note itself; for a chord, its highest note
     *        by sounding exact position (Note::getQuarterToneSteps()), the first written of equal
     *        ones. A note tied to it adds its duration: the event keeps this note's written pitch
     *        and the measure where it starts, with the summed duration.
     */
    Note note;
    int measureIdx = 0;  ///< 0-based index of the measure where the event starts.
};

/**
 * @brief The events of one voice on one staff of one part, in measure order.
 */
struct MelodicLine {
    int partIdx = 0;  ///< 0-based index of the part.
    int staff = 0;    ///< 0-based staff.
    int voice = 0;    ///< The voice as written (Note::getVoice()).
    std::vector<MelodicEvent> events;
};

/**
 * @brief The melodic lines of a score's parts: one per part, staff and voice that occurs on that
 *        staff, listed by part, by staff, then by voice in ascending order.
 * @details A note without `<chord/>` starts an event, and the chord notes written right after it
 *          on its staff, in its voice, join that event; a chord note that follows no note of its
 *          voice on its staff (a chord spread over two staves) joins no event. A grace note is
 *          not an event. A rest is an event. A note whose tie list holds "stop" and that sounds
 *          the exact position of the previous event of its line, itself a note, extends that
 *          event: its duration is added, exactly, and it starts no event of its own.
 * @param parts The parts, in score order.
 * @return The lines; a line has at least one event.
 */
std::vector<MelodicLine> melodicLines(const std::vector<Part>& parts);

/**
 * @brief Rejects a melody pattern of fewer than 2 notes: a melodic interval needs two. Every
 *        melody search makes this check, naming itself.
 * @param method The searching method, which the message names first
 *        ("Score::findMelodyPattern").
 * @param numNotes The pattern's number of notes.
 * @throws std::runtime_error If numNotes is less than 2: "<method>: a melody pattern needs at
 *         least 2 notes, and this one has <numNotes>".
 */
void requireTwoNotes(const std::string& method, size_t numNotes);

}  // namespace maiacore::detail

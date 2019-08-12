#ifndef QUANTIZE_H
#define QUANTIZE_H

#include <cstdint>
#include <vector>
#include "midi/MidiFile.h"
#include "midi/MidiEvent.h"

struct note
{

    // NoteOn / NoteOff

    bool on;

    // Note value

    uint8_t key;

    // Tick in which the event occurs (not delta)

    uint32_t tick;

};

class track_quantize
{

public:

    // Tick for last NoteOff message

    uint32_t last_off;

    // Function for creating NoteOff messages

    void create_notes();

public:

    // All notes for this track

    std::vector<note> notes;

    // Constructor

    track_quantize(const MidiEventList& track,uint32_t q);

};

class file_quantize
{

protected:

    // All tracks for this file

    std::vector<track_quantize> tracks;

public:

    // Constructor

    file_quantize(const MidiFile& file,uint32_t q);

    // Create a quantized midi file

    void create_midi(const std::string& filename,uint16_t tpqn,uint32_t tempo) const;

};

#endif

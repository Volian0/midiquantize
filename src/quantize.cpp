#include <stdexcept>
#include "quantize.h"
#include "midi.h"

uint32_t add_vlq(uint32_t a,uint32_t b)
{
    uint32_t c = a + b;
    if ((c|a|b)>0xFFFFFFF)
        throw std::overflow_error("Variable-length quantity overflow");
    return c;
}

uint32_t shift(uint32_t tick,uint32_t q)
{
    if ((tick % q ) * 2 < q)
    {
        while (tick%q!=0)
            --tick;
    }
    else
    {
        while (tick%q!=0)
            ++tick;
    }
    return add_vlq(tick,0);
}

track_quantize::track_quantize(const MidiEventList& track,uint32_t q)
    : last_off{0}
{
    uint32_t size = track.getSize();
    uint32_t tick = 0;
    for (uint32_t i = 0; i < size; ++i)
    {
        const auto& event = track[i];
        tick = add_vlq(tick,event.tick);
        if (event.isNoteOn())
            notes.push_back({true,static_cast<uint8_t>(event.getKeyNumber()),shift(tick,q)});
        else if (event.isNoteOff())
            last_off = shift(tick,q);
    }
    if (!notes.empty()&&notes.back().tick>=last_off)
        last_off = add_vlq(last_off,q);
    create_notes();
}

void track_quantize::create_notes()
{
    std::vector<note> new_notes;
    std::vector<note> temp_notes;
    for (const auto note : notes)
    {
        if (!temp_notes.empty()&&temp_notes.back().tick!=note.tick)
        {
            for (const auto temp_note : temp_notes)
                new_notes.push_back({false,temp_note.key,note.tick});
            temp_notes.clear();
        }
        temp_notes.push_back(note);
        new_notes.push_back(note);
    }
    if (!temp_notes.empty())
        for (const auto temp_note : temp_notes)
            new_notes.push_back({false,temp_note.key,last_off});
    notes = new_notes;
}

file_quantize::file_quantize(const MidiFile& file,uint32_t q)
{
    uint32_t size = file.getTrackCount();
    for (uint32_t i = 0; i < size; ++i)
        tracks.push_back(track_quantize(file[i],q));
}

void file_quantize::create_midi(const std::string& filename,uint16_t tpqn,uint32_t tempo) const
{
    midi::MIDIfile midi(tpqn);
    midi.SetTempo(tempo);
    for (uint32_t i = 0; i < tracks.size(); ++i)
    {
        uint32_t last_tick = 0;
        for (const auto event : tracks[i].notes)
        {
            midi[i].AddDelay(event.tick - last_tick);
            if (event.on)
                midi[i].KeyOn(0,event.key,100);
            else
                midi[i].KeyOff(0,event.key,64);
            last_tick = event.tick;
        }
    }
    midi.Create(filename);
}

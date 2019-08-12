#include <map>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "quantize.h"

const std::map<std::string,uint32_t> divisions =
{
    {"sixth-step",24},
    {"quarter-step",16},
    {"third-step",12},
    {"half-step",8},
    {"step",4},
    {"sixth-beat",6},
    {"quarter-beat",4},
    {"third-beat",3},
    {"half-beat",2},
    {"beat",1}
};

bool is_digits(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

uint32_t safe_div(uint32_t a,uint32_t b)
{
    uint32_t c = a / b;
    if (c*b!=a)
        throw std::runtime_error("This division doesn't work: "+std::to_string(a)+" / "+std::to_string(b)+" = NOT integer\n                        TPQN^");
    return c;
}

uint32_t get_tick(uint32_t tpqn,const std::string& tick)
{
    if (is_digits(tick))
        return std::stoull(tick);
    auto search = divisions.find(tick);
    if (search == divisions.end())
        throw std::runtime_error("Wrong division, enter the tick as number or use these:\nsixth-step  quarter-step  third-step  half-step  step\nsixth-beat  quarter-beat  third-beat  half-beat  beat");
    return safe_div(tpqn,search->second);
}

int main(int argc, char* argv[])
try
{
    if (argc<4||argc>6)
        throw std::runtime_error("Invalid number of arguments");
    uint32_t tempo = 500000;
    if (argc>=5)
    {
        long double des_bpm = std::stold(argv[4]);
        if (!isnormal(des_bpm))
            throw std::runtime_error("BPM is either infinity, NaN, zero or subnormal");
        if (des_bpm < 1 || des_bpm>60000000)
            throw std::runtime_error("Invalid BPM range, minimum is 3.5762787, maximum is 60000000");
        tempo = static_cast<long double>(60000000) / des_bpm;
        if (tempo > 0xFFFFFF)
            throw std::runtime_error("Minimum BPM is 3.5762787");
    }
    MidiFile midi;
    midi.read(argv[1]);
    if (!midi.status() || midi.getTrackCount() == 0)
        throw std::runtime_error("MIDI file is invalid");
    midi.deltaTicks();
    uint32_t tpqn = midi.getTicksPerQuarterNote();
    uint32_t tick = get_tick(tpqn,argv[3]);
    if (tick>0xFFFFFFF||tpqn>32767||tempo>16777215||tick==0||tpqn==0||tempo==0)
        throw std::runtime_error("Argument's value is out of range or zero");
    file_quantize file(midi,tick);
    file.create_midi(argv[2],tpqn,tempo);

}
catch (const std::exception& e)
{
    std::cout << "Exception:" << std::endl;
    std::cout << e.what() << std::endl;
    if (argc<6||std::string(argv[5])!="-f")
        std::cin.get();
    return -1;
}
catch (...)
{
    std::cout << "Unknown error" << std::endl;
    return -1;
}

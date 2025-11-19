#pragma once
#include "MidiFile.h"
#include "Options.h"
#include <vector>
#include <functional>

class MIDIPlayer {
public:
    bool load(const std::string& path);

    // dt_sec: 経過秒数 (timer から渡す)
    // left_note_on  / left_note_off  : トラック0 のノートコールバック
    // right_note_on / right_note_off : トラック1 のノートコールバック
    void update(
        double dt_sec,
        std::function<void(int midi_note)> left_note_on,
        std::function<void(int midi_note)> left_note_off,
        std::function<void(int midi_note)> right_note_on,
        std::function<void(int midi_note)> right_note_off
    );

private:
    struct Event {
        double time_sec;
        bool on;
        int note;
        int track; // 0 or 1
    };

    std::vector<Event> events;
    size_t index = 0;
    double curTime = 0;
};

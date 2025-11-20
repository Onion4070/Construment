#pragma once
#include "MidiFile.h"
#include "Options.h"
#include <vector>
#include <functional>

struct ActiveNotes {
    int left = -1;   // 鳴らすMIDIノート番号（-1 なら無音）
    int right = -1;
};

class MIDIPlayer {
public:
    bool load(const std::string& path);

    // dt_sec: 経過秒数 (timer から渡す)
    ActiveNotes update(double dt_sec);
    void reset();
    bool EndPlaying() { return end; };

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

	int left_current = -1;
	int right_current = -1;
    bool end = false;
};

#pragma once
#include "MidiFile.h"
#include "Options.h"
#include <vector>
#include <functional>
class DrawPanel;

struct ActiveNotes {
    int left = -1;   // 鳴らすMIDIノート番号（-1 なら無音）
    int right = -1;
};

class MIDIPlayer {
public:
    bool load(const std::string& path);

    void SetDrawPanel(DrawPanel* panel) { drawPanel = panel; }

    ActiveNotes update();
    void reset();
    bool EndPlaying() { return end; };
    void TempoUp();
    void TempoDown();

    double GetDt() const { return dt_sec; }

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
    double dt_sec = 0.008; // 8ms

    DrawPanel* drawPanel = nullptr;
};

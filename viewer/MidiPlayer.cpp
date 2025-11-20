#include "MIDIPlayer.h"
#include <algorithm>
using namespace smf;

bool MIDIPlayer::load(const std::string& path) {
    MidiFile file;
    if (!file.read(path)) return false;

    file.doTimeAnalysis();
    file.linkNotePairs();

    events.clear();
    left_current = -1;
    right_current = -1;

    int tracks = file.getTrackCount();
    if (tracks < 1) return false;

    for (int t = 0; t < tracks; t++) {
        if (t > 1) break;  // トラック0,1 以外無視

        for (int i = 0; i < file[t].size(); i++) {
            MidiEvent& ev = file[t][i];

            if (ev.isNoteOn()) {
                double t_on = ev.seconds;
                int note = ev.getKeyNumber();

                events.push_back({ t_on, true, note, t });

                // 対応する NoteOff
                double t_off = t_on + ev.getDurationInSeconds();
                events.push_back({ t_off, false, note, t });
            }
        }
    }

    // 時間順に並べ替え
    std::sort(events.begin(), events.end(),
        [](auto& a, auto& b) { return a.time_sec < b.time_sec; });

    index = 0;
    curTime = 0;
    return true;
}

ActiveNotes MIDIPlayer::update(double dt_sec) {
    curTime += dt_sec;

    while (index < events.size() && events[index].time_sec <= curTime) {
        Event& e = events[index];

        if (e.track == 0) {
            left_current = e.on ? e.note : -1;
        }
        else if (e.track == 1) {
            right_current = e.on ? e.note : -1;
        }

        index++;
    }
	if (index == events.size()) end = true;

    return { left_current, right_current };
}

void MIDIPlayer::reset() {
    index = 0;
    curTime = 0;
    left_current = -1;
    right_current = -1;
	end = false;
}
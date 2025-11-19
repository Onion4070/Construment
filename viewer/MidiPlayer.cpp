#include "MIDIPlayer.h"
#include <algorithm>
using namespace smf;

bool MIDIPlayer::load(const std::string& path) {
    MidiFile file;
    if (!file.read(path)) return false;

    file.doTimeAnalysis();
    file.linkNotePairs();

    events.clear();

    int tracks = file.getTrackCount();
    if (tracks < 1) return false;

    // トラック 0 → left
    // トラック 1 → right
    for (int t = 0; t < tracks; t++) {
        if (t > 1) break; // 0,1 以外無視

        for (int i = 0; i < file[t].size(); i++) {
            MidiEvent& ev = file[t][i];

            if (ev.isNoteOn()) {
                double t_on = ev.seconds;
                int note = ev.getKeyNumber();

                events.push_back({ t_on, true, note, t });

                // 対応する NoteOff
                double dur = ev.getDurationInSeconds();
                double t_off = t_on + dur;

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

void MIDIPlayer::update(
    double dt_sec,
    std::function<void(int)> left_on,
    std::function<void(int)> left_off,
    std::function<void(int)> right_on,
    std::function<void(int)> right_off
) {
    curTime += dt_sec;

    while (index < events.size() && events[index].time_sec <= curTime) {
        Event& e = events[index];

        if (e.track == 0) {
            if (e.on) left_on(e.note);
            else      left_off(e.note);
        }
        else if (e.track == 1) {
            if (e.on) right_on(e.note);
            else      right_off(e.note);
        }

        index++;
    }
}

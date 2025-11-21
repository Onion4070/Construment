#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <utility>
#include <mutex>
#include "Scale.h"

// グローバルなデフォルトノート（左, 右）
// アクセスは必ず defaultNotesMutex で保護してください。
inline std::pair<Scale::Note, Scale::Note> defaultNotes = { Scale::Silence, Scale::Silence };
inline std::mutex defaultNotesMutex;

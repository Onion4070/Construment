#include "DrawPanel.h"
#include "MIDIPlayer.h"
#include "Scale.h"
#include <cctype>

// イベントテーブル定義
wxBEGIN_EVENT_TABLE(DrawPanel, wxPanel)
	EVT_PAINT(DrawPanel::OnPaint)
	EVT_TIMER(wxID_ANY, DrawPanel::OnTimer)
	EVT_KEY_DOWN(DrawPanel::OnKeyDown)
	EVT_KEY_UP(DrawPanel::OnKeyUp)
wxEND_EVENT_TABLE()

// ペン定義
wxPen blackPen(*wxBLACK, 2, wxPENSTYLE_SOLID);
wxPen bluePen(*wxBLUE, 2, wxPENSTYLE_SOLID); // 青色ペン，太さ2, 実線
wxPen anyColorPen(wxColor(255, 100, 100), 5); // 任意の色(薄い赤)，太さ5

DrawPanel::DrawPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY), 
	refresh_timer(this, wxID_ANY)
{
	// 背景色を白に設定
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetBackgroundColour(*wxWHITE);

	// SVG読み込み
	wxString path_treble = wxT("assets/treble.svg");
	wxString path_bass   = wxT("assets/bass.svg");
	
	wxBitmapBundle bundle_treble = wxBitmapBundle::FromSVGFile(path_treble, wxSize(330, 330));
	wxBitmapBundle bundle_bass   = wxBitmapBundle::FromSVGFile(path_bass, wxSize(150, 150));
	svgBitmapTreble = bundle_treble.GetBitmap(wxSize(330, 330));
	svgBitmapBass   = bundle_bass.GetBitmap(wxSize(150, 150));

	refresh_timer.Start(8);

	// パネルがキー入力を受け取れるようにフォーカスを要求
	this->SetFocus();

	gamepad.SetDefaultNoteCallback([this](Scale::Note l, Scale::Note r) {
		this->CallAfter([this, l, r]() {
			current_left = l;
			current_right = r;
			this->Refresh();
		});
	});
}

// キー押下イベントハンドラ
void DrawPanel::OnKeyDown(wxKeyEvent& event) {
	int key = event.GetKeyCode();
	// normalize to upper-case for letters
	if (key >= 'a' && key <= 'z') key = std::toupper(key);

	// modifiers set on keydown
	if (key == 'H') { mod_h = true; event.Skip(); return; }
	if (key == 'J') { mod_j = true; event.Skip(); return; }
	if (key == 'K') { mod_k = true; event.Skip(); return; }
	if (key == 'L') { mod_l = true; event.Skip(); return; }
	if (key == 'N') { mod_n = true; event.Skip(); return; }
	if (key == 'M') { mod_m = true; event.Skip(); return; }

	// Silence: update left/right depending on modifiers
	if (key == 'S') {
		if (mod_n) current_left = Scale::Silence;
		if (mod_m) current_right = Scale::Silence;
		if (!mod_n && !mod_m) { current_left = Scale::Silence; current_right = Scale::Silence; }
		if (gamepad.IsConnected()) gamepad.SendDefaultNote(current_left, current_right);
		event.Skip();
		return;
	}

	// Note keys A-G
	Scale::Note base = Scale::Silence;
	bool isNote = true;
	switch (key) {
		case 'C': base = Scale::C4; break;
		case 'D': base = Scale::D4; break;
		case 'E': base = Scale::E4; break;
		case 'F': base = Scale::F4; break;
		case 'G': base = Scale::G4; break;
		case 'A': base = Scale::A4; break;
		case 'B': base = Scale::B4; break;
		default: isNote = false; break;
	}

	if (isNote && gamepad.IsConnected()) {
		int semitoneOffset = 0;
		if (mod_h) semitoneOffset += -12;
		if (mod_j) semitoneOffset += -1;
		if (mod_k) semitoneOffset += +1;
		if (mod_l) semitoneOffset += +12;

		Scale::Note target = Scale::transpose(base, semitoneOffset);
		if (mod_n) current_left = target;
		if (mod_m) current_right = target;
		if (!mod_n && !mod_m) { current_left = target; current_right = target; }

		gamepad.SendDefaultNote(current_left, current_right);
	}

	event.Skip();
}

// キー解放イベントハンドラ
void DrawPanel::OnKeyUp(wxKeyEvent& event) {
	int key = event.GetKeyCode();
	if (key >= 'a' && key <= 'z') key = std::toupper(key);
	if (key == 'H') { mod_h = false; }
	if (key == 'J') { mod_j = false; }
	if (key == 'K') { mod_k = false; }
	if (key == 'L') { mod_l = false; }
	if (key == 'N') { mod_n = false; }
	if (key == 'M') { mod_m = false; }
	event.Skip();
}

void DrawPanel::ClearBackground(wxGCDC& gdc) {
	gdc.SetBrush(wxBrush(GetBackgroundColour()));
	gdc.SetPen(*wxTRANSPARENT_PEN);
	gdc.DrawRectangle(GetClientRect());
}

void DrawPanel::OnTimer(wxTimerEvent& event) {
	const double dt = 0.008; // 8ms

	if (gamepad.IsConnected() && gamepad.midiPlaying) {
		auto notes = gamepad.midi.update(dt);
		if (gamepad.midi.EndPlaying()) gamepad.midiPlaying = false;
		gamepad.SendNotes(notes);
	}
	Refresh();
}

// 五線の描画(上下同時に描画)
void DrawPanel::DrawScoreLine(wxGCDC& gdc, int width = 50, int offset = 150) {
	gdc.SetPen(blackPen);
	for (int i = 0; i < 11; i++) {
		if (i == 5) continue; // 上下の五線の間の線は描画しない
		gdc.DrawLine(0, offset + width*i, this->GetSize().GetWidth(), offset + width*i);
	}
}

// notes: 描画するノートの配列 (NoteInfo)
void DrawPanel::Draw(wxGCDC* gdc, const std::vector<std::pair<NoteInfo, NoteInfo>>& notes) {
	wxPen buttonPen(*wxBLACK, 2, wxPENSTYLE_SOLID);
	gdc->SetPen(buttonPen);

	const int baseX = 300;  // left-to-right placement base
	const int baseY = 250;  // top-to-bottom placement base
	const int spacing = 80; // horizontal spacing between notes
	const int radius = 25;

	int drawCount = 12;
	int total = (int)notes.size();
	if (total == 0) return;
	int start = total - drawCount;
	if (start < 0) start = 0;

	const int staffOffset = 150;  // same as DrawScoreLine default offset
	const int staffSpacing = 50;  // same as DrawScoreLine default width (line spacing)

	// helper to determine if a note is sharp
	auto isSharp = [](Scale::Note n) -> bool {
		using Scale::Note;
		switch (n) {
															case Note::Gs2: case Note::As2:
			case Note::Cs3: case Note::Ds3: case Note::Fs3: case Note::Gs3: case Note::As3:
			case Note::Cs4: case Note::Ds4: case Note::Fs4: case Note::Gs4: case Note::As4:
			case Note::Cs5: case Note::Ds5: case Note::Fs5: case Note::Gs5: case Note::As5:
			case Note::Cs6: case Note::Ds6:
				return true;
			default:
				return false;
		}
	};

	auto drawSingle = [&](int cx, const NoteInfo& entry) {
		Scale::Note note = entry.note;
		if (note==Scale::Silence) return;

		int fontSize = 30;
		wxFont noteFont(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
		gdc->SetFont(noteFont);

		int cy = staffOffset + baseY + noteHeight[note] * radius;

		// draw ledger (補助線) behind the note (円の後ろに見えるように、円描画前に描画)
		wxPen oldPen = gdc->GetPen();
		wxPen ledgerPen(*wxBLACK, 3, wxPENSTYLE_SOLID);
		gdc->SetPen(ledgerPen);

		int halfLen = radius + 8; // 線の半分の長さ（必要なら調整可）
		if (note==Scale::C4 || note==Scale::Cs4) {
			gdc->DrawLine(cx - halfLen, cy, cx + halfLen, cy);
		} else if (note>=Scale::A5) {
			int a5y = staffOffset + baseY + noteHeight[(int)Scale::A5] * radius;
			gdc->DrawLine(cx - halfLen, a5y, cx + halfLen, a5y);
			if (note>=Scale::C6) {
				int c6y = staffOffset + baseY + noteHeight[(int)Scale::C6] * radius;
				gdc->DrawLine(cx - halfLen, c6y, cx + halfLen, c6y);
			}
		}

		gdc->SetPen(oldPen);
		gdc->DrawCircle(cx, cy, radius);

		// label
		int tw, th;
		gdc->GetTextExtent(entry.label, &tw, &th);
		int tx = cx - tw / 2;
		int ty = cy - th / 2;
		gdc->DrawText(entry.label, tx, ty);

		// sharp mark
		if (isSharp(note)) {
			std::string sharp = "#";
			int ssw, ssh;
			gdc->GetTextExtent(sharp, &ssw, &ssh);
			int shx = cx - radius - ssw - 6;
			int shy = cy - ssh / 2;
			gdc->DrawText(sharp, shx, shy);
		}

		// index string (subscript)
		if (!entry.indexStr.empty()) {
			int idxFontSize = 14;
			wxFont idxFont(idxFontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
			gdc->SetFont(idxFont);
			int itw, ith;
			gdc->GetTextExtent(entry.indexStr, &itw, &ith);
			int itx = cx - itw / 2;
			int ity = cy + radius + 4; // small padding below the circle
			gdc->DrawText(entry.indexStr, itx, ity);
			gdc->SetFont(noteFont);
		}
	};

	for (int i = start; i < total; i++) {
		const auto& pairEntry = notes[i];
		const NoteInfo& left = pairEntry.first;
		const NoteInfo& right = pairEntry.second;

		int cx = baseX + spacing * (i - start);

		if (left.note == right.note) {
			drawSingle(cx, left);
		} else {
			drawSingle(cx, left);
			drawSingle(cx, right);
		}
	}
}

void DrawPanel::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	wxGCDC gdc(dc);
	ClearBackground(gdc);
	DrawScoreLine(gdc);

	gdc.DrawBitmap(svgBitmapTreble, 50, 98, true); // ト音記号
	gdc.DrawBitmap(svgBitmapBass, 50, 450, true);  // ヘ音記号

	// テキスト描画
	wxFont font(75, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
	gdc.SetFont(font);
	wxString info = wxString::Format("Vol. %d", GetSize().GetWidth()); // 仮
	gdc.DrawText(info, GetSize().GetHeight()/2, 650); // テキスト描画

	auto gamepad_state = gamepad.GetGamePad();
	if (!gamepad.IsConnected()) return;
	gamepad.Update();
	auto buf = gamepad.GetInputStream(current_left, current_right);
	Draw(&gdc, buf);
}

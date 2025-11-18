#include "DrawPanel.h"
#include <iostream>
using std::cout;
using std::endl;
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
	
	wxBitmapBundle bundle_treble = wxBitmapBundle::FromSVGFile(path_treble, wxSize(300, 300));
	wxBitmapBundle bundle_bass   = wxBitmapBundle::FromSVGFile(path_bass, wxSize(150, 150));
	svgBitmapTreble = bundle_treble.GetBitmap(wxSize(300, 300));
	svgBitmapBass   = bundle_bass.GetBitmap(wxSize(150, 150));

	refresh_timer.Start(8);

	// パネルがキー入力を受け取れるようにフォーカスを要求
	this->SetFocus();
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
	Refresh();
}

// 五線の描画(上下同時に描画)
void DrawPanel::DrawScoreLine(wxGCDC& gdc, int width = 50, int offset = 50) {
	for (int i = 0; i < 11; i++) {
		if (i == 5) continue; // 上下の五線の間の線は描画しない
		gdc.DrawLine(0, offset + width*i, this->GetSize().GetWidth(), offset + width*i);
	}
}

void DrawPanel::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	wxGCDC gdc(dc);
	ClearBackground(gdc);

	gdc.SetPen(blackPen);
	DrawScoreLine(gdc);

	gdc.DrawCircle(300, 100, 50); // 中心(300, 100), 半径50の円を描画

	// 正八角形を描画
	gdc.SetPen(anyColorPen);
	wxPoint points[8];
	for (int i = 0; i < 8; i++) {
		double angle = i * (2 * 3.14159 / 8); // 角度を計算
		points[i] = wxPoint(500 + 50 * cos(angle), 100 + 50 * sin(angle)); // 中心(500, 100), 半径50
	}
	gdc.DrawPolygon(8, points);

	gdc.DrawBitmap(svgBitmapTreble, 50, 20, true); // ト音記号
	gdc.DrawBitmap(svgBitmapBass, 50, 350, true); // ヘ音記号

	// テキスト描画
	wxFont font(75, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
	gdc.SetFont(font);
	wxString info = wxString::Format("Vol. %d", GetSize().GetWidth()); // 仮
	gdc.DrawText(info, GetSize().GetHeight()/2, 550); // テキスト描画

	auto gamepad_state = gamepad.GetGamePad();
	if (gamepad_state.empty() || !gamepad.IsConnected()) return;
	// cout << (int)gamepad_state[3] << endl;
}

#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include "GamePad.h"
#include "Scale.h"
#include "globals.h"

// 描画用パネル
class DrawPanel : public wxPanel
{
public:
	DrawPanel(wxWindow* parent);
	GamePad gamepad;

	// KeyDown/KeyUp を使って modifier の押下状態を管理
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);

private:
	// 描画イベントハンドラ
	void OnPaint(wxPaintEvent& event);
	void ClearBackground(wxGCDC& gdc);
	void DrawScoreLine(wxGCDC& gdc, int width, int offset);
	void Draw(wxGCDC* gdc, const std::vector<std::pair<NoteInfo, NoteInfo>>& notes);
	void OnTimer(wxTimerEvent& event);

	wxBitmap svgBitmapTreble; // SVG画像を保持するビットマップ
	wxBitmap svgBitmapBass;
	wxTimer refresh_timer; // タイマー

	// modifier 状態
	bool mod_h = false; // octave down
	bool mod_j = false; // semitone down
	bool mod_k = false; // semitone up
	bool mod_l = false; // octave up

	// which side to update
	bool mod_n = false; // update left when true
	bool mod_m = false; // update right when true

	// current default notes (maintained on viewer side so we can update one side)
	Scale::Note current_left = Scale::Silence;
	Scale::Note current_right = Scale::Silence;

	// per-note vertical positions (pixels) used for drawing; initialized in constructor
	const int noteHeight[Scale::COUNT] = {
												 10,   9,   9,   8,
		  7,   7,   6,   6,   5,   4,   4,   3,   3,   2,   2,   1,
		  0,   0,  -1,  -1,  -2,  -3,  -3,  -4,  -4,  -5,  -5,  -6,
		 -7,  -7,  -8,  -8,  -9, -10, -10, -11, -11, -12, -12, -13,
		-14, -14, -15, -15,
		-100
	};

	// イベントテーブル宣言
	wxDECLARE_EVENT_TABLE();
};

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
	void DrawNote(wxGCDC* gdc, std::vector<std::pair<uint8_t, std::string>>& notes);
	void Draw(wxGCDC* gdc, std::vector<std::pair<Scale::Note, std::pair<std::string, std::string>>>& notes);
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
	std::vector<int> noteHeight = {
														 250,  225,  225,  200,
		 175,  175,  150,  150,  125,  100,  100,   75,   75,   50,   50,   25,
		   0,    0,  -25,  -25,  -50,  -75,  -75, -100, -100, -125, -125, -150,
		-175, -175, -200, -200, -225, -250, -250, -275, -275, -300, -300, -325,
		-350, -350, -375, -375,
		-500,
		-500
	};

	// イベントテーブル宣言
	wxDECLARE_EVENT_TABLE();
};

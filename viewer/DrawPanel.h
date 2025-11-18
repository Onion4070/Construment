#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include "GamePad.h"

// 描画用パネル
class DrawPanel : public wxPanel
{
public:
	DrawPanel(wxWindow* parent);
	GamePad gamepad;

	// キー入力ハンドラ（A-G を拾って RP 側へ送信する）
	void OnCharHook(wxKeyEvent& event);
	// KeyDown/KeyUp を使って modifier の押下状態を管理
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);

private:
	// 描画イベントハンドラ
	void OnPaint(wxPaintEvent& event);
	void ClearBackground(wxGCDC& gdc);
	void DrawScoreLine(wxGCDC& gdc, int width, int offset);
	void OnTimer(wxTimerEvent& event);

	wxBitmap svgBitmapTreble; // SVG画像を保持するビットマップ
	wxBitmap svgBitmapBass;
	wxTimer refresh_timer; // タイマー

	// modifier 状態
	bool mod_h = false; // octave down
	bool mod_j = false; // semitone down
	bool mod_k = false; // semitone up
	bool mod_l = false; // octave up

	// イベントテーブル宣言
	wxDECLARE_EVENT_TABLE();
};

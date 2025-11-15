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

private:
	// 描画イベントハンドラ
	void OnPaint(wxPaintEvent& event);
	void ClearBackground(wxGCDC& gdc);
	void DrawScoreLine(wxGCDC& gdc, int width, int offset);
	void OnTimer(wxTimerEvent& event);

	wxBitmap svgBitmapTreble; // SVG画像を保持するビットマップ
	wxBitmap svgBitmapBass;
	wxTimer refresh_timer; // タイマー

	// イベントテーブル宣言
	wxDECLARE_EVENT_TABLE();
};

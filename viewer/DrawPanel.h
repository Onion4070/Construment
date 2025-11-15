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
	void ConnectGamePad(const std::string& portName);

private:
	// 描画イベントハンドラ
	void OnPaint(wxPaintEvent& event);
	void ClearBackground(wxGCDC& gdc);
	void DrawScoreLine(wxGCDC& gdc, int width, int offset);

	wxBitmap svgBitmapTreble; // SVG画像を保持するビットマップ
	wxBitmap svgBitmapBass;

	GamePad gamepad;

	// イベントテーブル宣言
	wxDECLARE_EVENT_TABLE();
};

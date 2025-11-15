#include "DrawPanel.h"
#include "SerialUtils.h"
#include <iostream>
using std::cout;
using std::endl;

// イベントテーブル定義
wxBEGIN_EVENT_TABLE(DrawPanel, wxPanel)
	EVT_PAINT(DrawPanel::OnPaint)
wxEND_EVENT_TABLE()

// ペン定義
wxPen blackPen(*wxBLACK, 2, wxPENSTYLE_SOLID);
wxPen bluePen(*wxBLUE, 2, wxPENSTYLE_SOLID); // 青色ペン，太さ2, 実線
wxPen anyColorPen(wxColor(255, 100, 100), 5); // 任意の色(薄い赤)，太さ5


DrawPanel::DrawPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
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

	auto ports = SerialUtils::AvailablePorts();
	for (const auto& port : ports) {
		cout << "Port: " << port.port << ", Description: " << port.description << endl;
	}
}

void DrawPanel::ClearBackground(wxGCDC& gdc) {
	gdc.SetBrush(wxBrush(GetBackgroundColour()));
	gdc.SetPen(*wxTRANSPARENT_PEN);
	gdc.DrawRectangle(GetClientRect());
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
}

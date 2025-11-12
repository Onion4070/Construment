#include "MainFrame.h"
#include <wx/wx.h>

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {
	//// フレーム上にパネルを作成(thisはMainFrameを指す)
	//wxPanel* panel = new wxPanel(this);

	//// ボタン
	//// 注意: 第一引数にthis=>MainFrameを指定すると，ボタンがフレーム全体に広がってしまう
	////// (150, 150)はボタンの位置(左上), (100, 50)はボタンのサイズ(幅, 高さ)
	//wxButton* button = new wxButton(panel, wxID_ANY, "Button", wxPoint(50, 150), wxSize(100, 50));

	//// チェックボックス
	////// 第五引数(wxSize)は指定しなくてもOK, 指定すれば領域の中央に配置される
	//wxCheckBox* checkBox = new wxCheckBox(panel, wxID_ANY, "CheckBox", wxPoint(200, 150)); /*, wxSize(100, 50));*/

	//// 静的テキスト
	//wxStaticText* staticText = new wxStaticText(panel, wxID_ANY, "StaticText - Not editable", wxPoint(300, 150));

	//// テキストボックス
	////// wxSizeの第二引数を-1 -> 高さを自動的に文字幅に合わせて設定する
	//wxTextCtrl* textCtrl = new wxTextCtrl(panel, wxID_ANY, "TextCtrl - editable", wxPoint(450, 150), wxSize(200, -1));

	//// スライダー
	////// 第3～第5引数は default, min, max value
	//wxSlider* slider = new wxSlider(panel, wxID_ANY, 25, 0, 100, wxPoint(50, 250), wxSize(200, -1));
	//
	//// ゲージ(プログレスバー)
	////// 第3引数はrange
	//wxGauge* gauge = new wxGauge(panel, wxID_ANY, 100, wxPoint(250, 250), wxSize(200, -1));
	//gauge->SetValue(50);

	// 描画用パネル
	m_drawPanel = new DrawPanel(this);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_drawPanel, 1, wxEXPAND);
	this->SetSizer(sizer);
}
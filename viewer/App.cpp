#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>

// インスタンス生成
wxIMPLEMENT_APP(App);

bool App::OnInit() {
	// タイトル設定
	MainFrame* mainFrame = new MainFrame("Construment App");

	// フレームサイズ設定
	mainFrame->SetClientSize(wxSize(1280, 720));
	mainFrame->SetMinClientSize(wxSize(1280, 720));
	mainFrame->SetMaxClientSize(wxSize(1280, 720));

	// フレーム初期位置を中央に
	mainFrame->Center();

	// フレームを表示
	mainFrame->Show(true);
	return true;
}

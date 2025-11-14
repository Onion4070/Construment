#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>

#define DEBUG

// インスタンス生成
wxIMPLEMENT_APP(App);

bool App::OnInit() {

	// コンソール画面を強制的に出す
	#ifdef DEBUG
		if (AllocConsole()) {
			FILE* fp;
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONOUT$", "w", stderr);
			std::cout << "Debug console started" << std::endl;
		}
	#endif

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

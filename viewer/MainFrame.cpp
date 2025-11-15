#include "MainFrame.h"
#include "SerialUtils.h"
#include <wx/wx.h>

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {
	//// フレーム上にパネルを作成(thisはMainFrameを指す)
	//wxPanel* panel = new wxPanel(this);

	// 描画用パネル
	drawPanel = new DrawPanel(this);

	wxPanel* topPanel = new wxPanel(this);
	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	comChoice = new wxChoice(topPanel, wxID_ANY);
	connectButton = new wxButton(topPanel, wxID_ANY, "Connect");

	auto ports = SerialUtils::AvailablePorts();
	for (const auto& port : ports) {
		wxString choiceLabel = wxString::Format("%s (%s)", port.port, port.description);
		comChoice->Append(choiceLabel);
	}

	topSizer->Add(comChoice, 1, wxEXPAND | wxRIGHT);
	topSizer->Add(connectButton, 0, wxEXPAND);

	topPanel->SetSizer(topSizer);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(topPanel, 0, wxEXPAND | wxALL);
	mainSizer->Add(drawPanel, 1, wxEXPAND);
	this->SetSizer(mainSizer);

	connectButton->Bind(wxEVT_BUTTON, &MainFrame::OnConnect, this);
}

void MainFrame::OnConnect(wxCommandEvent& event) {
	int sel = comChoice->GetSelection();
	if (sel == wxNOT_FOUND) {
		wxMessageBox("Please select a COM port.", "Error", wxOK | wxICON_ERROR);
		return;
	}

	wxString portName = comChoice->GetString(sel);
	portName = portName.BeforeFirst(' '); // ポート名だけ抽出
}
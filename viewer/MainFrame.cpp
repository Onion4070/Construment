#include "MainFrame.h"
#include "SerialUtils.h"
#include "DrawPanel.h"
#include <wx/wx.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_BUTTON(wxID_ANY, MainFrame::OnConnect)
	EVT_BUTTON(wxID_ANY, MainFrame::OnRefresh)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {
	//// フレーム上にパネルを作成(thisはMainFrameを指す)
	//wxPanel* panel = new wxPanel(this);

	// 描画用パネル
	drawPanel = new DrawPanel(this);

	wxPanel* topPanel = new wxPanel(this);
	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	comChoice = new wxChoice(topPanel, wxID_ANY);
	connectButton = new wxButton(topPanel, wxID_ANY, "Connect");
	refreshButton = new wxButton(topPanel, wxID_ANY, "Refresh");
	loadButton = new wxButton(topPanel, wxID_ANY, "Load");
	playButton = new wxButton(topPanel, wxID_ANY, "Play");
	stopButton = new wxButton(topPanel, wxID_ANY, "Stop");

	RefreshComPorts();

	topSizer->Add(comChoice, 1, wxEXPAND | wxRIGHT);
	topSizer->Add(connectButton, 0, wxEXPAND);
	topSizer->Add(refreshButton, 0, wxEXPAND | wxLEFT);
	topSizer->Add(loadButton, 0, wxEXPAND | wxLEFT);
	topSizer->Add(playButton, 0, wxEXPAND | wxLEFT);
	topSizer->Add(stopButton, 0, wxEXPAND | wxLEFT);

	topPanel->SetSizer(topSizer);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(topPanel, 0, wxEXPAND | wxALL);
	mainSizer->Add(drawPanel, 1, wxEXPAND);
	this->SetSizer(mainSizer);

	connectButton->Bind(wxEVT_BUTTON, &MainFrame::OnConnect, this);
	refreshButton->Bind(wxEVT_BUTTON, &MainFrame::OnRefresh, this);
	loadButton->Bind(wxEVT_BUTTON, &MainFrame::OnLoad, this);
	playButton->Bind(wxEVT_BUTTON, &MainFrame::OnPlay, this);
	stopButton->Bind(wxEVT_BUTTON, &MainFrame::OnStop, this);
}

void MainFrame::RefreshComPorts() {
	comChoice->Clear();
	auto ports = SerialUtils::AvailablePorts();
	for (const auto& port : ports) {
		wxString choiceLabel = wxString::Format("%s (%s)", port.port, port.description);
		comChoice->Append(choiceLabel);
	}
	comChoice->SetSelection(0);
}

void MainFrame::OnConnect(wxCommandEvent& event) {
	int sel = comChoice->GetSelection();
	if (sel == wxNOT_FOUND) {
		wxMessageBox("Please select a COM port.", "Error", wxOK | wxICON_ERROR);
		return;
	}

	wxString portName = comChoice->GetString(sel);
	portName = portName.BeforeFirst(' '); // ポート名だけ抽出

	if (drawPanel->gamepad.IsConnected()) {
		drawPanel->gamepad.Disconnect();
		connectButton->SetLabel("Connect");
		return;
	}
	drawPanel->gamepad.Connect(portName.ToStdString());
	connectButton->SetLabel("Disconnect");
}

void MainFrame::OnRefresh(wxCommandEvent& event) {
	RefreshComPorts();
}

void MainFrame::OnLoad(wxCommandEvent& event) {
	wxFileDialog* openFileDialog = new wxFileDialog(this);
	openFileDialog->SetMessage("Select MIDI file");
	openFileDialog->SetWildcard("MIDI files (*.mid;*.midi)|*.mid;*.midi");

	if (openFileDialog->ShowModal() == wxID_CANCEL) {
		return;
	}
	std::cout << "Full path: " << openFileDialog->GetPath() << std::endl;
	drawPanel->gamepad.LoadMidi(openFileDialog->GetPath().ToStdString());
}

void MainFrame::OnPlay(wxCommandEvent& event) {
	if (!drawPanel->gamepad.IsConnected()) {
		wxMessageBox("Please connect to a COM port first.", "Error", wxOK | wxICON_ERROR);
		return;
	}
	drawPanel->gamepad.PlayMidi();
}

void MainFrame::OnStop(wxCommandEvent& event) {
	if (drawPanel->gamepad.IsConnected()) {
		drawPanel->gamepad.StopMidi();
	}
}
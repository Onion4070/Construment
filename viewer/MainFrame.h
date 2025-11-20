#pragma once
// See https://www.youtube.com/watch?v=cQalRGqRRp4&list=PLFk1_lkqT8MbVOcwEppCPfjGOGhLvcf9G&index=3

#include <wx/wx.h>
#include "DrawPanel.h"
#include "globals.h"

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);

private:
	void OnConnect(wxCommandEvent& event);
	void OnRefresh(wxCommandEvent& event);
	void OnLoad(wxCommandEvent& event);
	void RefreshComPorts();

	DrawPanel* drawPanel;
	wxChoice* comChoice;
	wxButton* connectButton;
	wxButton* loadButton;
	wxButton* refreshButton;

	wxDECLARE_EVENT_TABLE();
};


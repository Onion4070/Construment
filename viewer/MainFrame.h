#pragma once
// See https://www.youtube.com/watch?v=cQalRGqRRp4&list=PLFk1_lkqT8MbVOcwEppCPfjGOGhLvcf9G&index=3

#include <wx/wx.h>
#include "DrawPanel.h"

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);

private:
	DrawPanel* m_drawPanel;
};


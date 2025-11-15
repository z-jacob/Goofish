
#include "DashboardPage.h"

void CDashboardPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("“«±Ì≈Ã“≥√Ê"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1100);
	else
		m_stLabel.MoveWindow(&rc);
}
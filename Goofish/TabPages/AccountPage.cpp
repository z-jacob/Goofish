#include "AccountPage.h"

void CAccountPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("’À∫≈π‹¿Ì“≥√Ê"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1101);
	else
		m_stLabel.MoveWindow(&rc);
}
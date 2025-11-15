#include "AutoDeliverPage.h"

void CAutoDeliverPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("自动发货页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1107);
	else
		m_stLabel.MoveWindow(&rc);
}
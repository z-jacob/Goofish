
#include "AutoReplyPage.h"

void CAutoReplyPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("自动回复页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1104);
	else
		m_stLabel.MoveWindow(&rc);
}
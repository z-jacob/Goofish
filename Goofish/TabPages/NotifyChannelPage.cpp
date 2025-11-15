
#include "NotifyChannelPage.h"

void CNotifyChannelPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("通知渠道页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1108);
	else
		m_stLabel.MoveWindow(&rc);
}
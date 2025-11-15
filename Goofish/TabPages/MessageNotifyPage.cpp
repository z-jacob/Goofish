
#include "MessageNotifyPage.h"

void CMessageNotifyPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("消息通知页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1109);
	else
		m_stLabel.MoveWindow(&rc);
}
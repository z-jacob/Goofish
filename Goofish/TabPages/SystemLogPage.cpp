
#include "SystemLogPage.h"

void CSystemLogPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("系统日志页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1112);
	else
		m_stLabel.MoveWindow(&rc);
}
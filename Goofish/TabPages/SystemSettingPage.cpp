
#include "SystemSettingPage.h"

void CSystemSettingPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("系统设置页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1111);
	else
		m_stLabel.MoveWindow(&rc);
}
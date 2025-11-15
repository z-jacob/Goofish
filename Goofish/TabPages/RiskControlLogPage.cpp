#include "RiskControlLogPage.h"

void CRiskControlLogPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("风控日志页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1113);
	else
		m_stLabel.MoveWindow(&rc);
}
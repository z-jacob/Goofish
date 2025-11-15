
#include "OrderPage.h"

void COrderPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("订单管理页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1103);
	else
		m_stLabel.MoveWindow(&rc);
}
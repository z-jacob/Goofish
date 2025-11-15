
#include "ProductPage.h"

void CProductPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("商品管理页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1102);
	else
		m_stLabel.MoveWindow(&rc);
}
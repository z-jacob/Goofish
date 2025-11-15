
#include "SpecificProductReplyPage.h"

void CSpecificProductReplyPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("指定商品回复页面"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1105);
	else
		m_stLabel.MoveWindow(&rc);
}
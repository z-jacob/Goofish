
#include "CouponPage.h"

void CCouponPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("ø®»Øπ‹¿Ì“≥√Ê"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1106);
	else
		m_stLabel.MoveWindow(&rc);
}
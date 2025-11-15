
#include "ProductSearchPage.h"

void CProductSearchPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);
	if (!m_stLabel.GetSafeHwnd())
		m_stLabel.Create(_T("ÉÌÆ·ËÑË÷Ò³Ãæ"), WS_CHILD | WS_VISIBLE | SS_LEFT, rc, this, 1110);
	else
		m_stLabel.MoveWindow(&rc);
}
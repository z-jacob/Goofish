#pragma once
#include <afxwin.h>

class CTabPageBase : public CWnd
{
public:
	CTabPageBase() = default;
	virtual ~CTabPageBase() = default;

	// 创建页面窗口，parent 是对话框，rc 是页面在对话框客户区的绝对坐标，nID 唯一 ID
	virtual BOOL CreatePage(CWnd* pParent, const CRect& rc, UINT nID)
	{
		LPCTSTR className = AfxRegisterWndClass(0);
		if (!CWnd::Create(className, _T(""), WS_CHILD | WS_VISIBLE, rc, pParent, nID))
			return FALSE;
		CreateContent();
		return TRUE;
	}

	// 子类覆盖以创建具体控件
	virtual void CreateContent()
	{
		CRect rc;
		GetClientRect(&rc);
		if (!m_stLabel.GetSafeHwnd())
		{
			m_stLabel.Create(_T("Tab Page"), WS_CHILD | WS_VISIBLE | SS_CENTER, rc, this, 1000);
		}
		else
		{
			m_stLabel.MoveWindow(&rc);
		}
	}

	// 调整大小
	virtual void Resize(const CRect& rc)
	{
		MoveWindow(&rc);
		CRect rcClient;
		GetClientRect(&rcClient);
		if (m_stLabel.GetSafeHwnd())
			m_stLabel.MoveWindow(&rcClient);
	}

protected:
	CStatic m_stLabel;
};
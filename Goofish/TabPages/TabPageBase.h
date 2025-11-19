#pragma once
#include <afxwin.h>
#include "../Helper/JFramework.h"
#include "../Model/FontModel.h"
#include "../Model/UIModel.h"

class CTabPageBase : public CWnd, public JFramework::AbstractController
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

		m_uiModel = this->GetModel<UIModel>();
		m_font = this->GetModel<FontModel>()->GetFont();

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


	std::weak_ptr<JFramework::IArchitecture> GetArchitecture() const override;

protected:
	CStatic m_stLabel;

	CFont* m_font;

	std::shared_ptr<UIModel> m_uiModel;
	void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;

};
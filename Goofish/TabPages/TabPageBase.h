#pragma once
#include <afxwin.h>
#include "../Helper/JFramework.h"

class CTabPageBase : public CWnd,public JFramework::AbstractController
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


		// 在 OnInitDialog 里创建字体并应用到按钮
		m_font.CreateFont(
			13,                // 字体高度
			0,                 // 字体宽度（0为自适应）
			0,                 // 文字倾斜角度
			0,                 // 基线倾斜角度
			FW_NORMAL,           // 字体粗细
			FALSE,             // 是否斜体
			FALSE,             // 是否下划线
			0,                 // 字体字符集
			ANSI_CHARSET,      // 字符集
			OUT_DEFAULT_PRECIS,// 输出精度
			CLIP_DEFAULT_PRECIS,// 裁剪精度
			DEFAULT_QUALITY,   // 输出质量
			DEFAULT_PITCH | FF_SWISS, // 字体类型
			_T("宋体")      // 字体名称
		);


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

	CFont m_font;

	void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;

};
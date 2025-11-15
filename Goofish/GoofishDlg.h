
// GoofishDlg.h: 头文件
//

#pragma once
#include "Helper/JFramework.h"
#include <memory>
#include <vector>
#include "TabPages/TabPageBase.h"

// CGoofishDlg 对话框
class CGoofishDlg : public CDialog,public JFramework::AbstractController
{
// 构造
public:
	CGoofishDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GOOFISH_DIALOG };
#endif


	std::weak_ptr<JFramework::IArchitecture> GetArchitecture() const override;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



	void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;

	// 实现
protected:
	HICON m_hIcon;
	CTabCtrl m_tabCtrl;
	CFont m_tabFont; // 新增：保持字体对象生命周期，避免临时对象被销毁
	std::vector<std::unique_ptr<CTabPageBase>> m_pages;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
};

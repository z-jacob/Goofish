
// GoofishDlg.h: 头文件
//

#pragma once
#include "Helper/JFramework.h"


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

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};

// GoofishDlg.h: 头文件
//

#pragma once
#include "Helper/JFramework.h"
#include "TabPages/TabControlEx.h"
#include <memory>

#include "System/WebsocketClientSystem.h"
#include "Model/UIModel.h"

enum EnAppState
{
	ST_STARTING, ST_STARTED, ST_CONNECTING, ST_CONNECTED, ST_STOPPING, ST_STOPPED
};


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
	CTabManager m_tabManager;   // 新：封装后的 Tab 管理器


	CButton m_btnStop;
	CButton m_btnStart;
	CButton m_btnRestart;
	CButton m_btnSend;

	JFramework::BindableProperty<EnAppState> m_state;


    std::shared_ptr<WebsocketClientSystem> m_websocketClientSystem;

    std::shared_ptr<UIModel> m_uiModel;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	// 新增：按钮点击事件
	afx_msg void OnBtnStop();
	afx_msg void OnBtnStart();
	afx_msg void OnBtnRestart();
	afx_msg void OnBtnSend();

	void SetAppState(EnAppState state);
};

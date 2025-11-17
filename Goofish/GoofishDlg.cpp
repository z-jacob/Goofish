// GoofishDlg.cpp: 实现文件
//

#include "framework.h"
#include "Goofish.h"
#include "GoofishDlg.h"
#include "afxdialogex.h"
#include "GoofishArchitecture.h"
#include "System/WebsocketClientSystem.h"
#include "Helper/WebsocketEvents.h"
#include "Helper/ProcessHelper.h"
#include "Helper/Logger.h"

#include "TabPages/DashboardPage.h"
#include "TabPages/AccountPage.h"
#include "TabPages/ProductPage.h"
#include "TabPages/OrderPage.h"
#include "TabPages/AutoReplyPage.h"
#include "TabPages/SpecificProductReplyPage.h"
#include "TabPages/CouponPage.h"
#include "TabPages/AutoDeliverPage.h"
#include "TabPages/NotifyChannelPage.h"
#include "TabPages/MessageNotifyPage.h"
#include "TabPages/ProductSearchPage.h"
#include "TabPages/SystemSettingPage.h"
#include "TabPages/SystemLogPage.h"
#include "TabPages/RiskControlLogPage.h"
#include "Helper/Utils.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGoofishDlg 对话框



CGoofishDlg::CGoofishDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_GOOFISH_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

std::weak_ptr<JFramework::IArchitecture> CGoofishDlg::GetArchitecture() const
{
	return GoofishArchitecture::Instance();
}

void CGoofishDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGoofishDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(1001, &CGoofishDlg::OnBtnStop)
	ON_BN_CLICKED(1002, &CGoofishDlg::OnBtnStart)
	ON_BN_CLICKED(1003, &CGoofishDlg::OnBtnRestart)
	ON_BN_CLICKED(1004, &CGoofishDlg::OnBtnSend)
END_MESSAGE_MAP()


// CGoofishDlg 消息处理程序


BOOL CGoofishDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置窗口图标
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// ---- 获取系统 ----
	{
		m_websocketClientSystem = this->GetSystem<WebsocketClientSystem>();
	}

	// ---- 创建按钮 ----
	{

		// 按钮尺寸和位置
		const int btnWidth = 80;
		const int btnHeight = 30;
		const int btnTop = 10;
		const int btnSpacing = 10;
		const int btnLeftStart = 20;

		// 创建 Stop 按钮
		m_btnStop.Create(_T("Stop"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CRect(btnLeftStart, btnTop, btnLeftStart + btnWidth, btnTop + btnHeight),
			this, 1001);

		// 创建 Start 按钮
		m_btnStart.Create(_T("Start"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CRect(btnLeftStart + btnWidth + btnSpacing, btnTop,
				btnLeftStart + 2 * btnWidth + btnSpacing, btnTop + btnHeight),
			this, 1002);

		// 创建 Restart 按钮
		m_btnRestart.Create(_T("Restart"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CRect(btnLeftStart + 2 * (btnWidth + btnSpacing), btnTop,
				btnLeftStart + 3 * btnWidth + 2 * btnSpacing, btnTop + btnHeight),
			this, 1003);

		m_btnSend.Create(_T("Send"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CRect(btnLeftStart + 3 * (btnWidth + btnSpacing), btnTop,
				btnLeftStart + 4 * btnWidth + 2 * btnSpacing, btnTop + btnHeight),
			this, 1004);
	}

	this->RegisterEvent<WebsocketConnectionEvent>(this);
	this->RegisterEvent<WebsocketErrorEvent>(this);
	this->RegisterEvent<WebsocketReceiveEvent>(this);
	this->RegisterEvent<WebsocketDisconnectionEvent>(this);



	// ---- 使用封装的 Tab 管理器 ----
	{
		CRect rc;
		GetClientRect(&rc);
		rc.DeflateRect(8, 8); // 边距
		rc.top += 40;

		// 创建封装的 tab 控件（内部创建 CTabCtrlEx）
		m_tabManager.Create(this, IDC_TAB_CTRL, rc);

		// 添加页面（使用你已有的页面类）
		m_tabManager.AddPage(std::make_unique<CDashboardPage>(), _T("仪表盘"));
		m_tabManager.AddPage(std::make_unique<CAccountPage>(), _T("账号管理"));
		m_tabManager.AddPage(std::make_unique<CProductPage>(), _T("商品管理"));
		m_tabManager.AddPage(std::make_unique<COrderPage>(), _T("订单管理"));
		m_tabManager.AddPage(std::make_unique<CAutoReplyPage>(), _T("自动回复"));
		m_tabManager.AddPage(std::make_unique<CSpecificProductReplyPage>(), _T("指定商品回复"));
		m_tabManager.AddPage(std::make_unique<CCouponPage>(), _T("卡券管理"));
		m_tabManager.AddPage(std::make_unique<CAutoDeliverPage>(), _T("自动发货"));
		m_tabManager.AddPage(std::make_unique<CNotifyChannelPage>(), _T("通知渠道"));
		m_tabManager.AddPage(std::make_unique<CMessageNotifyPage>(), _T("消息通知"));
		m_tabManager.AddPage(std::make_unique<CProductSearchPage>(), _T("商品搜索"));
		m_tabManager.AddPage(std::make_unique<CSystemSettingPage>(), _T("系统设置"));
		m_tabManager.AddPage(std::make_unique<CSystemLogPage>(), _T("系统日志"));
		m_tabManager.AddPage(std::make_unique<CRiskControlLogPage>(), _T("风控日志"));
	}



	m_state = ControllerState::Stopped;

	m_state.RegisterWithInitValue([this](ControllerState state)
		{
			UpdateButtonStates(state);
		});

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGoofishDlg::OnClose()
{

	KillSelfProcess();

	__super::OnClose();
}


void CGoofishDlg::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{
	if (auto e = std::dynamic_pointer_cast<WebsocketConnectionEvent>(event))
	{
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketErrorEvent>(event))
	{
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketReceiveEvent>(event))
	{
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketDisconnectionEvent>(event))
	{
		m_state = ControllerState::Stopped;
	}
}

// 修改 OnSize：调整 Tab 与页面大小
void CGoofishDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	CRect rc;
	GetClientRect(&rc);
	rc.DeflateRect(8, 8);
	rc.top += 40;

	// 将大小调整委托给封装类
	m_tabManager.OnParentSize(rc);
}

void CGoofishDlg::OnBtnStop()
{
	m_websocketClientSystem->Close();
	m_state = ControllerState::Stopped;
}

void CGoofishDlg::OnBtnStart()
{
	//wss://echo.websocket.org
	//if (m_websocketClientSystem->Connect("wss://wss-goofish.dingtalk.com"))
	if (m_websocketClientSystem->Connect("wss://echo.websocket.org"))
	{
		m_state = ControllerState::Running;
	}
}

void CGoofishDlg::OnBtnRestart()
{
	m_state = ControllerState::Running;
}

void CGoofishDlg::OnBtnSend()
{
	m_websocketClientSystem->Send("hello world");
}

void CGoofishDlg::UpdateButtonStates(ControllerState state)
{
	if (state == ControllerState::Stopped)
	{
		m_btnStop.EnableWindow(FALSE);
		m_btnStart.EnableWindow(TRUE);
		m_btnRestart.EnableWindow(TRUE);
	}
	else // Running
	{
		m_btnStop.EnableWindow(TRUE);
		m_btnStart.EnableWindow(FALSE);
		m_btnRestart.EnableWindow(FALSE);
	}
}

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
END_MESSAGE_MAP()


// CGoofishDlg 消息处理程序


BOOL CGoofishDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	this->RegisterEvent<WebsocketConnectionEvent>(this);
	this->RegisterEvent<WebsocketErrorEvent>(this);
	this->RegisterEvent<WebsocketReceiveEvent>(this);
	this->RegisterEvent<WebsocketDisconnectionEvent>(this);



	// ---- 在代码中创建 Tab 控件 ----
	{
		CRect rc;
		GetClientRect(&rc);
		rc.DeflateRect(8, 8); // 边距，可根据需要调整

		// 样式：可见、子窗口、选项卡、热跟踪、防止重绘冲突等
		DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_TABS | TCS_HOTTRACK;
		m_tabCtrl.Create(style, rc, this, IDC_TAB_CTRL);

		// 添加示例页签
		m_tabCtrl.InsertItem(0, _T("仪表盘"));
		m_tabCtrl.InsertItem(1, _T("账号管理"));
		m_tabCtrl.InsertItem(2, _T("商品管理"));
		m_tabCtrl.InsertItem(3, _T("订单管理"));
		m_tabCtrl.InsertItem(4, _T("自动回复"));
		m_tabCtrl.InsertItem(5, _T("指定商品回复"));
		m_tabCtrl.InsertItem(6, _T("卡券管理"));
		m_tabCtrl.InsertItem(7, _T("自动发货"));
		m_tabCtrl.InsertItem(8, _T("通知渠道"));
		m_tabCtrl.InsertItem(9, _T("消息通知"));
		m_tabCtrl.InsertItem(10, _T("商品搜索"));
		m_tabCtrl.InsertItem(11, _T("系统设置"));
		m_tabCtrl.InsertItem(12, _T("系统日志"));
		m_tabCtrl.InsertItem(13, _T("风控日志"));

		if (!m_tabFont.GetSafeHandle()) {
			m_tabFont.CreatePointFont(100, _T("Microsoft YaHei"));
		}
		m_tabCtrl.SetFont(&m_tabFont);
	}


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
		LOG_INFO("Websocket Connect.");
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketErrorEvent>(event))
	{
		auto message = "Websocket Error: " + e->m_errorMessage;
		LOG_ERROR(message);
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketReceiveEvent>(event))
	{
		auto message = "Websocket Receive: " + e->m_message;
		LOG_INFO(message);
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketDisconnectionEvent>(event))
	{
		//m_listLog.AddString("Websocket Disconnect.");
		LOG_WARNING("Websocket Disconnect.");
	}
}

void CGoofishDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// 窗口大小改变时调整 Tab 大小（保留边距）
	if (m_tabCtrl.GetSafeHwnd())
	{
		CRect rc;
		GetClientRect(&rc);
		rc.DeflateRect(8, 8);
		m_tabCtrl.MoveWindow(&rc);
	}
}

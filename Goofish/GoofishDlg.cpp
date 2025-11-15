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
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CTRL, &CGoofishDlg::OnTcnSelchangeTab)
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

		// 设置字体
		if (!m_tabFont.GetSafeHandle()) {
			m_tabFont.CreatePointFont(100, _T("Microsoft YaHei"));
		}
		m_tabCtrl.SetFont(&m_tabFont);

		// 创建每个 tab 对应的页面实例并放入 m_pages
		// 先计算页内容区域（相对于对话框客户区）
		CRect rcTabWnd; m_tabCtrl.GetWindowRect(&rcTabWnd);
		ScreenToClient(&rcTabWnd); // 变为对话框坐标
		CRect rcClient; m_tabCtrl.GetClientRect(&rcClient);
		m_tabCtrl.AdjustRect(FALSE, &rcClient); // 得到内容区（相对于 tab 客户区）
		rcClient.OffsetRect(rcTabWnd.TopLeft()); // 转为对话框坐标

		// 清理旧的（若有）
		m_pages.clear();

		// 依次创建具体页面，ID 分配确保唯一（使用 IDC_TAB_CTRL + offset）
		m_pages.push_back(std::make_unique<CDashboardPage>());
		m_pages.push_back(std::make_unique<CAccountPage>());
		m_pages.push_back(std::make_unique<CProductPage>());
		m_pages.push_back(std::make_unique<COrderPage>());
		m_pages.push_back(std::make_unique<CAutoReplyPage>());
		m_pages.push_back(std::make_unique<CSpecificProductReplyPage>());
		m_pages.push_back(std::make_unique<CCouponPage>());
		m_pages.push_back(std::make_unique<CAutoDeliverPage>());
		m_pages.push_back(std::make_unique<CNotifyChannelPage>());
		m_pages.push_back(std::make_unique<CMessageNotifyPage>());
		m_pages.push_back(std::make_unique<CProductSearchPage>());
		m_pages.push_back(std::make_unique<CSystemSettingPage>());
		m_pages.push_back(std::make_unique<CSystemLogPage>());
		m_pages.push_back(std::make_unique<CRiskControlLogPage>());

		for (size_t i = 0; i < m_pages.size(); ++i)
		{
			UINT id = IDC_TAB_CTRL + 100 + static_cast<UINT>(i);
			m_pages[i]->CreatePage(this, rcClient, id);
			// 默认隐藏所有页面，之后只显示第一个
			m_pages[i]->ShowWindow(SW_HIDE);
		}

		// 显示第一个页
		if (!m_pages.empty())
			m_pages[0]->ShowWindow(SW_SHOW);

		// 注册选中变化通知处理（需在消息映射中添加 ON_NOTIFY）
		// 你需要在消息映射中添加： ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CTRL, &CGoofishDlg::OnTcnSelchangeTab)
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

// 新增：选中页切换处理（在头文件声明 afx_msg void OnTcnSelchangeTab(NMHDR*, LRESULT*);）
void CGoofishDlg::OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	int idx = m_tabCtrl.GetCurSel();
	if (idx < 0 || idx >= (int)m_pages.size()) return;

	// 隐藏其它页，显示当前页
	for (int i = 0; i < (int)m_pages.size(); ++i)
	{
		if (i == idx) m_pages[i]->ShowWindow(SW_SHOW);
		else m_pages[i]->ShowWindow(SW_HIDE);
	}

	// 确保当前页尺寸正确
	CRect rcTabWnd; m_tabCtrl.GetWindowRect(&rcTabWnd);
	ScreenToClient(&rcTabWnd);
	CRect rcClient; m_tabCtrl.GetClientRect(&rcClient);
	m_tabCtrl.AdjustRect(FALSE, &rcClient);
	rcClient.OffsetRect(rcTabWnd.TopLeft());
	m_pages[idx]->Resize(rcClient);

	if (pResult) *pResult = 0;
}

// 修改 OnSize：调整 Tab 与页面大小
void CGoofishDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if (m_tabCtrl.GetSafeHwnd())
	{
		CRect rc; GetClientRect(&rc);
		rc.DeflateRect(8,8);
		m_tabCtrl.MoveWindow(&rc);

		// 更新页内容区并调整每个页面
		CRect rcTabWnd; m_tabCtrl.GetWindowRect(&rcTabWnd);
		ScreenToClient(&rcTabWnd);
		CRect rcClient; m_tabCtrl.GetClientRect(&rcClient);
		m_tabCtrl.AdjustRect(FALSE, &rcClient);
		rcClient.OffsetRect(rcTabWnd.TopLeft());

		for (auto& p : m_pages)
		{
			if (p) p->Resize(rcClient);
		}
	}
}

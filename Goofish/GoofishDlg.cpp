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
#include "Model/FontModel.h"

#include <thread>
#include <atomic>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "System/GoofishHttpSystem.h"
#include "Helper/Logger.h"
#include "JSON/CJsonObject.hpp"


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

	auto font = this->GetModel<FontModel>()->GetFont();

	// ---- 获取模型 ----
	{
		m_uiModel = this->GetModel<UIModel>();
		m_configModel = this->GetModel<ConfigModel>();
	}

	// ---- 获取系统 ----
	{
		m_goofishHttpSystem = this->GetSystem<GoofishHttpSystem>();
		m_websocketClientSystem = this->GetSystem<WebsocketClientSystem>();
	}

	// ---- 创建按钮 ----
	{

		// 按钮尺寸和位置
		const int btnWidth = m_uiModel->GetButtonWidth();
		const int btnHeight = m_uiModel->GetButtonHeight();
		const int btnTop = (int)(10 * Utils::GetDpi());
		const int btnSpacing = (int)(10 * Utils::GetDpi());
		const int btnLeftStart = (int)(20 * Utils::GetDpi());

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


		// 在 OnInitDialog 里创建字体并应用到按钮


		// 应用到按钮
		m_btnStop.SetFont(font);
		m_btnStart.SetFont(font);
		m_btnRestart.SetFont(font);
		m_btnSend.SetFont(font);
	}

	// ---- 注册事件 ----
	{
		this->RegisterEvent<WebsocketConnectionEvent>(this);
		this->RegisterEvent<WebsocketErrorEvent>(this);
		this->RegisterEvent<WebsocketMessageBodyEvent>(this);
		this->RegisterEvent<WebsocketCloseEvent>(this);
		this->RegisterEvent<WebsocketHandShakeEvent>(this);
	}


	// ---- 使用封装的 Tab 管理器 ----
	{
		CRect rc;
		GetClientRect(&rc);
		rc.DeflateRect(m_uiModel->GetControlSafeDistance(), m_uiModel->GetControlSafeDistance()); // 边距
		rc.top += m_uiModel->GetTabControlTop();

		// 创建封装的 tab 控件（内部创建 CTabCtrlEx）
		m_tabManager.Create(this, IDC_TAB_CTRL, rc);
		m_tabManager.SetFont(font);

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

	// ---- 初始化状态绑定 ----
	{

		m_state = EnAppState::ST_STOPPED;

		m_state.RegisterWithInitValue([this](EnAppState state)
			{
				SetAppState(state);
			});
	}

	// ---- 调整窗口大小 ----
	{
		CRect rc;
		GetWindowRect(&rc);

		// 设置新宽高
		int newWidth = m_uiModel->GetWindowWidth();
		int newHeight = m_uiModel->GetWindowHeight();

		// 移动窗口到原来的位置，并改变大小
		SetWindowPos(nullptr, rc.left, rc.top, newWidth, newHeight, SWP_NOZORDER | SWP_NOACTIVATE);
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
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketErrorEvent>(event))
	{
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketMessageBodyEvent>(event))
	{
		auto message = e->m_message;
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketCloseEvent>(event))
	{
		m_state = EnAppState::ST_STOPPED;
	}
	else if (auto e = std::dynamic_pointer_cast<WebsocketHandShakeEvent>(event))
	{
		m_state = EnAppState::ST_STARTED;
	}
}

// 修改 OnSize：调整 Tab 与页面大小
void CGoofishDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	CRect rc;
	GetClientRect(&rc);
	rc.DeflateRect(m_uiModel->GetControlSafeDistance(), m_uiModel->GetControlSafeDistance());
	rc.top += m_uiModel->GetTabControlTop();

	// 将大小调整委托给封装类
	m_tabManager.OnParentSize(rc);
}

void CGoofishDlg::OnBtnStop()
{
	m_state = EnAppState::ST_STOPPING;
	m_websocketClientSystem->Close();
	m_workerRunning = false;
	if (m_workerThread.joinable()) m_workerThread.join();
}

void CGoofishDlg::OnBtnStart()
{
	m_state = EnAppState::ST_STARTING;

	// 启动一个循环线程
	if (!m_workerRunning) {
		m_workerRunning = true;
		m_workerThread = std::thread([this]()
			{
				while (m_workerRunning) {

					// 取出Cookie
					auto cookie = "t=5e247d2ff89af2e56b4aeebfdb3ef501; cna=9/CGIWtgdRkCAQAAAAAO+iAb; tracknick=yllove1989; havana_lgc2_77=eyJoaWQiOjI3MDM5MjM0NTAsInNnIjoiZDY0OTFiNzUzZTliNjNjZmZiMjI0MzE1NmQ3ZmZkZDciLCJzaXRlIjo3NywidG9rZW4iOiIxNEZpQlFBcVJhNGpOU2tSZ0hjY0V6QSJ9; _hvn_lgc_=77; havana_lgc_exp=1764606397378; cookie2=10d4943562d7d9d07a2f1aa15ef2ee45; xlly_s=1; _samesite_flag_=true; sgcookie=E100Vfm6R5sj9ZSJJGsuFO4JzXjtZoK4nA60hqz3Dci0klrN7pm%2BK9ck12doRC2Q7of7r8ty%2FX3aROzVhJR4Gj5UokOqz5bc5isdiWT68Wmb8K0%3D; csg=dcfb326c; _tb_token_=315a1e433b86b; unb=2703923450; sdkSilent=1763816242049; mtop_partitioned_detect=1; _m_h5_tk=88971805fec81f1683946c2d60d75a2b_1763745012004; _m_h5_tk_enc=b7f6c451654e1c2a056e6d885547dfb8; tfstk=geZEa66QnMIUFdR1A5ny352UIknKq05XUuGSE82odXcHAHOu7SVtOzaIOLzaZ53ItywBpPFbnHZCJ0trv0nlGssfcJwK20Y1bP1o9R2tEOMhGkfvd0nlG1T6q2Iq2W5zufouIOkSFeDuZvDGQxkoqH0kxFAi6Ymoq2xnsdDIFYDoE3XaIfHoqbVoqOoi6YmoZ7moNJ3-RFGn-OTtZ5PllvgEiJcw0REZKVLLKf-kqlzZ8jAj_3xubvyy-A5D0aG0kXgji5j6Y0yiUWG_iixZjqzYFYPFYHmQSPEKAl12H2PrOqqs7BYn3DlEok093nyErPrZAkfJkRwZtqo_5NCIPDPUkj3hW1FzQXNuYVAF1babCk0ai1taw4zYFYPFYHVc4pR-Igb72yRkz2DtQj6NQe_gE9p0AkckyU3MsAlf3jTJy2DtQj6NQUL-SNDZGtlf.";
					std::string refreshToken, accessToken;
					if (!m_goofishHttpSystem->Login(cookie, m_configModel->deviceId, refreshToken, accessToken))
					{
						LOG_ERROR(MODULE_INFO, "Login fail.");
						std::this_thread::sleep_for(std::chrono::seconds(5));
						continue;
					}

					LOG_INFO(MODULE_INFO, "refreshToken:" + refreshToken);
					LOG_INFO(MODULE_INFO, "accessToken:" + accessToken);

					if (!m_websocketClientSystem->Connect("wss://wss-goofish.dingtalk.com"))
					{
						LOG_ERROR(MODULE_INFO, "Connect wss-goofish.dingtalk.com fail.");
						std::this_thread::sleep_for(std::chrono::seconds(5));
						continue;
					}


					std::this_thread::sleep_for(std::chrono::seconds(5));


					neb::CJsonObject rootObj;
					rootObj.Add("lwp", "/reg");

					// 构造 headers 子对象
					neb::CJsonObject headersObj;
					headersObj.Add("cache-header", "app-key token ua wv");
					headersObj.Add("app-key", m_configModel->appKeyStr);
					headersObj.Add("token", accessToken);
					headersObj.Add("ua", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36 DingTalk(2.2.0) OS(Windows/10) Browser(Chrome/142.0.0.0) DingWeb/2.2.0 IMPaaS DingWeb/2.2.0");
					headersObj.Add("dt", "j");
					headersObj.Add("wv", "im:3,au:3,sy:6");
					headersObj.Add("sync", "0,0;0;0;");
					headersObj.Add("did", m_configModel->deviceId);
					headersObj.Add("mid", "4001763738942195 0");

					// 添加 headers 到根对象
					rootObj.Add("headers", headersObj);

					// 输出 JSON 字符串
					std::string jsonStr = rootObj.ToString();
					// jsonStr 即为组装好的 JSON 数据
					m_websocketClientSystem->Send(jsonStr);

					break;
				}
			});
	}

}

void CGoofishDlg::OnBtnRestart()
{

}

void CGoofishDlg::OnBtnSend()
{
	m_websocketClientSystem->Send("hello world");
}

void CGoofishDlg::SetAppState(EnAppState state)
{
	if (this->GetSafeHwnd() == nullptr)
		return;

	m_btnStop.EnableWindow(state == ST_STARTED);
	m_btnStart.EnableWindow(state == ST_STOPPED);
	m_btnRestart.EnableWindow(state == ST_STOPPED);
	m_btnSend.EnableWindow(state == ST_STARTED);
}

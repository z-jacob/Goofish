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
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
}

BEGIN_MESSAGE_MAP(CGoofishDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CGoofishDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CGoofishDlg::OnBnClickedButtonSend)
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

	// 初始化 Logger（可选写入文件）
	//Logger::Init("goofish.log");

	// 注册回调，将格式化日志显示到列表中
	// 注意：如果事件来自后台线程，请改为使用 PostMessage 将字符串发送到 UI 线程再调用 AddString
	Logger::SetCallback([this](const std::string& formatted) {
		m_listLog.AddString(formatted.c_str());
		});

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGoofishDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGoofishDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGoofishDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
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

void CGoofishDlg::OnBnClickedButtonConnect()
{
	try {
		auto wsSystem = GetSystem<WebsocketClientSystem>();

		wsSystem->Connect("wss://wss-goofish.dingtalk.com/");
	}
	catch (const std::exception& /*e*/) {
	}
}

void CGoofishDlg::OnBnClickedButtonSend()
{
	try {
		auto wsSystem = GetSystem<WebsocketClientSystem>();
		wsSystem->Send("Hello Goofish!");
	}
	catch (const std::exception& /*e*/) {
	}
}

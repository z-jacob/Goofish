#include "AccountPage.h"

void CAccountPage::CreateContent()
{
	CRect rc; GetClientRect(&rc);

	// 按钮参数
	const int btnWidth = 120;
	const int btnHeight = 40;
	const int btnSpacing = 30;
	const int btnCount = 3;
	const int totalWidth = btnWidth * btnCount + btnSpacing * (btnCount - 1);
	const int startX = (rc.Width() - totalWidth) / 2;
	const int topMargin = 30;

	// 创建扫码登录按钮
	if (!m_btnScanLogin.GetSafeHwnd())
		m_btnScanLogin.Create(_T("扫码登录"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CRect(startX, topMargin, startX + btnWidth, topMargin + btnHeight), this, 1201);
	else
		m_btnScanLogin.MoveWindow(CRect(startX, topMargin, startX + btnWidth, topMargin + btnHeight));

	// 创建账号密码登录按钮
	int btn2X = startX + btnWidth + btnSpacing;
	if (!m_btnPwdLogin.GetSafeHwnd())
		m_btnPwdLogin.Create(_T("账号密码登录"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CRect(btn2X, topMargin, btn2X + btnWidth, topMargin + btnHeight), this, 1202);
	else
		m_btnPwdLogin.MoveWindow(CRect(btn2X, topMargin, btn2X + btnWidth, topMargin + btnHeight));

	// 创建手动输入按钮
	int btn3X = btn2X + btnWidth + btnSpacing;
	if (!m_btnManualInput.GetSafeHwnd())
		m_btnManualInput.Create(_T("手动输入"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CRect(btn3X, topMargin, btn3X + btnWidth, topMargin + btnHeight), this, 1203);
	else
		m_btnManualInput.MoveWindow(CRect(btn3X, topMargin, btn3X + btnWidth, topMargin + btnHeight));

	m_btnScanLogin.SetFont(m_font);
	m_btnPwdLogin.SetFont(m_font);
	m_btnManualInput.SetFont(m_font);
}

void CAccountPage::Resize(const CRect& rc)
{
	CTabPageBase::Resize(rc);

	// 按钮参数
	const int btnWidth = 120;
	const int btnHeight = 40;
	const int btnSpacing = 30;
	const int btnCount = 3;
	const int totalWidth = btnWidth * btnCount + btnSpacing * (btnCount - 1);
	const int startX = (rc.Width() - totalWidth) / 2;
	const int topMargin = 30;

	// 移动扫码登录按钮
	m_btnScanLogin.MoveWindow(CRect(startX, topMargin, startX + btnWidth, topMargin + btnHeight));

	// 移动账号密码登录按钮
	int btn2X = startX + btnWidth + btnSpacing;
	m_btnPwdLogin.MoveWindow(CRect(btn2X, topMargin, btn2X + btnWidth, topMargin + btnHeight));

	// 移动手动输入按钮
	int btn3X = btn2X + btnWidth + btnSpacing;
	m_btnManualInput.MoveWindow(CRect(btn3X, topMargin, btn3X + btnWidth, topMargin + btnHeight));
}

#include "SystemLogPage.h"



void CSystemLogPage::CreateContent()
{
	CRect rc;
	GetClientRect(&rc);

	//STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU
	// 初始创建，填满整个页面
	m_listLog.Create(
		WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
		rc,
		this,
		1001
	);

	m_listLog.SetFont(m_font);

	JFramework::Debug::SetCallback([this](const JFramework::Debug::Level level, const std::string& formatted)
		{
			size_t start = 0, end;
			while ((end = formatted.find('\n', start)) != std::string::npos) {
				std::string line = formatted.substr(start, end - start);
				if (!line.empty() && m_listLog.GetSafeHwnd()) {
					m_listLog.AddString(CA2T(line.c_str()));
					m_listLog.SetTopIndex(m_listLog.GetCount() - 1);
				}
				start = end + 1;
			}
			if (start < formatted.size() && m_listLog.GetSafeHwnd()) {
				std::string line = formatted.substr(start);
				if (!line.empty()) {
					m_listLog.AddString(CA2T(line.c_str()));
					m_listLog.SetTopIndex(m_listLog.GetCount() - 1);
				}
			}
		});


	Log(MODULE_INFO , "系统日志初始化完成");
	Log(MODULE_INFO , "正在监听系统事件...");

}

void CSystemLogPage::Resize(const CRect& rc)
{
	CTabPageBase::Resize(rc);

	CRect rcList;
	GetClientRect(&rcList);

	rcList.DeflateRect(m_uiModel->GetControlSafeDistance(), m_uiModel->GetControlSafeDistance());


	if (m_listLog.GetSafeHwnd())
		m_listLog.MoveWindow(rcList);
}
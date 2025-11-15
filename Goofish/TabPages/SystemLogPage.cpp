#include "SystemLogPage.h"
#include "../Helper/Logger.h"

BEGIN_MESSAGE_MAP(CSystemLogPage, CTabPageBase)
	ON_MESSAGE(WM_ADD_LOG_LINE, &CSystemLogPage::OnAddLogLine)
END_MESSAGE_MAP()

void CSystemLogPage::CreateContent()
{
	CRect rc;
	GetClientRect(&rc);

	// 初始创建，填满整个页面
	m_listLog.Create(
		WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
		rc,
		this,
		1001
	);

	Logger::SetCallback([this](const std::string& formatted)
		{
			size_t start = 0, end;
			while ((end = formatted.find('\n', start)) != std::string::npos) {
				std::string line = formatted.substr(start, end - start);
				if (!line.empty() && m_listLog.GetSafeHwnd()) {
					CString* pStr = new CString(CA2T(line.c_str()));
					this->PostMessage(WM_ADD_LOG_LINE, 0, reinterpret_cast<LPARAM>(pStr));
				}
				start = end + 1;
			}
			if (start < formatted.size() && m_listLog.GetSafeHwnd()) {
				std::string line = formatted.substr(start);
				if (!line.empty()) {
					CString* pStr = new CString(CA2T(line.c_str()));
					this->PostMessage(WM_ADD_LOG_LINE, 0, reinterpret_cast<LPARAM>(pStr));
				}
			}
		});


	LOG_INFO("系统日志初始化完成");
	LOG_INFO("正在监听系统事件...");

}

void CSystemLogPage::Resize(const CRect& rc)
{
	CTabPageBase::Resize(rc);

	CRect rcList;
	GetClientRect(&rcList);

	rcList.DeflateRect(10, 10);


	if (m_listLog.GetSafeHwnd())
		m_listLog.MoveWindow(rcList);
}

LRESULT CSystemLogPage::OnAddLogLine(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = reinterpret_cast<CString*>(lParam);
	if (pStr && m_listLog.GetSafeHwnd())
	{
		m_listLog.AddString(*pStr);
		delete pStr; // 释放内存
	}
	return 0;
}
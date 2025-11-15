#include "SystemLogPage.h"
#include "../Helper/Logger.h"



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
					m_listLog.AddString(CA2T(line.c_str()));
				}
				start = end + 1;
			}
			if (start < formatted.size() && m_listLog.GetSafeHwnd()) {
				std::string line = formatted.substr(start);
				if (!line.empty()) {
					m_listLog.AddString(CA2T(line.c_str()));
				}
			}
		});


	LOG_INFO(MODULE_INFO + "系统日志初始化完成");
	LOG_INFO(MODULE_INFO + "正在监听系统事件...");

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
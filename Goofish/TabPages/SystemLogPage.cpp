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
			// 根据换行符分割formatted，然后添加到列表中
			size_t start = 0, end;
			while ((end = formatted.find('\n', start)) != std::string::npos) {
				std::string line = formatted.substr(start, end - start);
				if (!line.empty()) {
					m_listLog.AddString(CA2T(line.c_str()));
				}
				start = end + 1;
			}
			// 处理最后一行（如果没有以\n结尾）
			if (start < formatted.size()) {
				std::string line = formatted.substr(start);
				if (!line.empty()) {
					m_listLog.AddString(CA2T(line.c_str()));
				}
			}
		});


	LOG_INFO("系统日志初始化完成");
	LOG_INFO("正在监听系统事件...");

}

void CSystemLogPage::Resize(const CRect& rc)
{
	CTabPageBase::Resize(rc);

	CRect rcList = rc;
	rcList.DeflateRect(8, 8); // 四周留 8 像素边距
	if (m_listLog.GetSafeHwnd())
		m_listLog.MoveWindow(rcList);
}
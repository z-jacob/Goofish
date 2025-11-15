#include "TabControlEx.h"
#include <cassert>

BEGIN_MESSAGE_MAP(CTabCtrlEx, CTabCtrl)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, &CTabCtrlEx::OnSelChange)
END_MESSAGE_MAP()

CTabCtrlEx::CTabCtrlEx()
	: m_manager(nullptr)
{
}

void CTabCtrlEx::OnSelChange(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	if (m_manager)
	{
		int idx = GetCurSel();
		m_manager->ShowPage(idx);
	}
	if (pResult) *pResult = 0;
}


// CTabManager
CTabManager::CTabManager()
	: m_font(nullptr), m_parent(nullptr)
{
}

CTabManager::~CTabManager()
{
	// 页面 unique_ptr 会自动释放
}

bool CTabManager::Create(CWnd* pParent, UINT nID, const CRect& rc, DWORD style)
{
	if (!pParent) return false;
	m_parent = pParent;

	// 创建 tab 控件
	if (!m_tab.Create(style, rc, pParent, nID))
		return false;

	m_tab.SetManager(this);

	// 保存 tab rect 与 page rect
	UpdatePageRect();
	return true;
}

void CTabManager::SetFont(CFont* pFont)
{
	m_font = pFont;
	if (m_font && m_tab.GetSafeHwnd())
	{
		m_tab.SetFont(m_font);
	}
}

void CTabManager::AddPage(std::unique_ptr<CTabPageBase> page, const CString& title)
{
	assert(page);
	int index = static_cast<int>(m_pages.size());
	m_pages.push_back(std::move(page));
	m_tab.InsertItem(index, title);

	// 如果 tab 已创建，则创建 page 窗口（使用 tab 的 dlg id + offset）
	createPageWindow(m_pages.back(), index);

	// 默认隐藏新页面
	if (m_pages.back()->GetSafeHwnd())
		m_pages.back()->ShowWindow(SW_HIDE);

	// 如果是第一个页面，显示之
	if (index == 0)
		ShowPage(0);
}

void CTabManager::createPageWindow(std::unique_ptr<CTabPageBase>& page, int index)
{
	if (!page) return;
	if (!m_parent) return;

	// Ensure page rect is up-to-date
	UpdatePageRect();

	// Create unique ID for child window
	UINT childId = m_tab.GetDlgCtrlID() + 100 + index;
	page->CreatePage(m_parent, m_pageRect, childId);
	// 设置页面默认字体与 tab 一致（可在页面内部自定义）
	if (m_font)
	{
		// page is a CWnd derived; set font for all children if desired.
		page->SetFont(m_font);
	}
}

void CTabManager::UpdatePageRect()
{
	if (!m_parent || !m_tab.GetSafeHwnd()) return;

	// tab 窗口相对于对话框的矩形
	CRect rcTabWnd;
	m_tab.GetWindowRect(&rcTabWnd);
	m_parent->ScreenToClient(&rcTabWnd);
	m_tabRect = rcTabWnd;

	// 客户区（相对于 tab 控件），然后 AdjustRect 得到 page 客户区（相对于 tab 客户区）
	CRect rcClient;
	m_tab.GetClientRect(&rcClient);
	m_tab.AdjustRect(FALSE, &rcClient);

	// 把 rcClient 转换为对话框坐标（page 区域）
	rcClient.OffsetRect(m_tabRect.TopLeft());
	m_pageRect = rcClient;
}

void CTabManager::OnParentSize(const CRect& rcParent)
{
	if (!m_tab.GetSafeHwnd()) return;

	// 这里传入的是对话框的客户区，我们让 tab 占据 rcParent（通常带边距）
	m_tab.MoveWindow(&rcParent);

	// 更新内容区并调整页面
	UpdatePageRect();
	for (auto& p : m_pages)
	{
		if (p && p->GetSafeHwnd())
			p->Resize(m_pageRect);
	}
}

void CTabManager::ShowPage(int idx)
{
	if (idx < 0 || idx >= (int)m_pages.size()) return;

	for (int i = 0; i < (int)m_pages.size(); ++i)
	{
		if (m_pages[i] && m_pages[i]->GetSafeHwnd())
		{
			if (i == idx) m_pages[i]->ShowWindow(SW_SHOW);
			else m_pages[i]->ShowWindow(SW_HIDE);
		}
	}
	// 确保当前页尺寸正确
	UpdatePageRect();
	if (m_pages[idx]) m_pages[idx]->Resize(m_pageRect);
}

int CTabManager::GetCurPage() const
{
	if (!m_tab.GetSafeHwnd()) return -1;
	return m_tab.GetCurSel();
}
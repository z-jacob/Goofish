#pragma once
#include "TabPageBase.h"
#include <memory>
#include <vector>
#include <afxcmn.h> // 补充此行，确保 CTabCtrl 可用

// 轻量封装：管理一个 CTabCtrl，以及其页面（CTabPageBase 派生）
// 目标：在对话框中只需与 CTabManager 交互，减少对话框文件的杂乱。
class CTabManager;

class CTabCtrlEx : public CTabCtrl
{
public:
	CTabCtrlEx();

	void SetManager(CTabManager* mgr) { m_manager = mgr; }

protected:
	afx_msg void OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	CTabManager* m_manager;
};

class CTabManager
{
public:
	CTabManager();
	~CTabManager();

	// 创建 tab 控件（在对话框 OnInitDialog 中调用）
	bool Create(CWnd* pParent, UINT nID, const CRect& rc, DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_TABS | TCS_HOTTRACK);

	// 设置字体（对 tab 标签生效）
	void SetFont(CFont* pFont);

	// 添加页面（会同时创建页面窗口并隐藏），title 会作为 tab 标签
	void AddPage(std::unique_ptr<CTabPageBase> page, const CString& title);

	// 在父窗口尺寸变化时调用（在对话框 OnSize 中）
	void OnParentSize(const CRect& rcParent);

	// 显示指定页（0-based）
	void ShowPage(int idx);

	int GetPageCount() const { return static_cast<int>(m_pages.size()); }
	int GetCurPage() const;

private:
	void UpdatePageRect(); // 计算 m_pageRect（相对于父窗口的坐标）
	void createPageWindow(std::unique_ptr<CTabPageBase>& page, int index);

private:
	CTabCtrlEx m_tab;
	CFont* m_font; // 不拥有字体，仅引用（对话框保存实际 CFont 对象）
	CWnd* m_parent;
	CRect m_tabRect;   // tab 控件在 parent 中的位置
	CRect m_pageRect;  // page 区域（在 parent 中坐标）
	std::vector<std::unique_ptr<CTabPageBase>> m_pages;
};
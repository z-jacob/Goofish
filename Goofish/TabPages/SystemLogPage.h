#pragma once
#include "TabPageBase.h"
#define WM_ADD_LOG_LINE (WM_USER + 100)
class CSystemLogPage : public CTabPageBase
{
public:
    virtual void CreateContent() override;
    virtual void Resize(const CRect& rc) override; // ÖØÐ´ Resize

private:
    CListBox m_listLog;
    afx_msg LRESULT OnAddLogLine(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};
#pragma once
#include "TabPageBase.h"
#include <afxwin.h>

class CSystemLogPage : public CTabPageBase
{
public:
    virtual void CreateContent() override;
    virtual void Resize(const CRect& rc) override; // ÖØÐ´ Resize

private:
    CListBox m_listLog;
};
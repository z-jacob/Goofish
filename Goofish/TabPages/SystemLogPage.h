#pragma once
#include "TabPageBase.h"


class CSystemLogPage : public CTabPageBase
{
public:
    virtual void CreateContent() override;
    virtual void Resize(const CRect& rc) override; // ÖØÐ´ Resize

private:
    CListBox m_listLog;
};
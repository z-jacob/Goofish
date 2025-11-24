#pragma once
#include "TabPageBase.h"
#include <afxwin.h>

class CAccountPage : public CTabPageBase
{
public:
    virtual void CreateContent() override;
    virtual void Resize(const CRect& rc) override; // ÖØÐ´ Resize
private:
    CButton m_btnScanLogin;
    CButton m_btnPwdLogin;
    CButton m_btnManualInput;
};
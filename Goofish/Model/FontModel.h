#pragma once
#include "../Helper/JFramework.h"
#include <afxwin.h>
class FontModel : public JFramework::AbstractModel
{
protected:
	// Í¨¹ý AbstractModel ¼Ì³Ð
	void OnInit() override;
	void OnDeinit() override;
public:
	CFont* GetFont() { return &m_font; }
private:
	CFont m_font;
};


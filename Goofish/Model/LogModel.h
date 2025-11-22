#pragma once
#include "../Helper/JFramework.h"
#include "../Helper/sqlite3.h"
class LogModel : public JFramework::AbstractModel
{
protected:
	// Í¨¹ý AbstractModel ¼Ì³Ð
	void OnInit() override;
	void OnDeinit() override;
	sqlite3* m_pDB;
};


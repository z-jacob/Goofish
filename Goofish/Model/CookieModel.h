#pragma once
#include "../Helper/JFramework.h"
#include <map>
#include <string>
#include "../Utility/SQLiteUtility.h"


class CookieModel : public JFramework::AbstractModel
{
protected:
	void OnInit() override;

	void OnDeinit() override;

    std::shared_ptr<SQLiteUtility> m_sqliteUtility;
public:
	bool AddCookie(std::string id, std::string cookie);
};


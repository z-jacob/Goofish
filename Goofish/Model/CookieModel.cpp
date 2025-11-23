#include "CookieModel.h"
#include "../Helper/Utils.h"


void CookieModel::OnInit()
{
	m_sqliteUtility = this->GetUtility<SQLiteUtility>();
	if (!m_sqliteUtility->OpenDB("goofish.db"))
	{
		LogError(MODULE_INFO, "打开数据库Goofish.db失败");
	}
}

void CookieModel::OnDeinit()
{

}

bool CookieModel::AddCookie(std::string cookie)
{
	auto unb = Utils::ExtractBetween(cookie, "unb=", ";");
	if (unb.length() <= 0)
	{
		LogError(MODULE_INFO, "添加Cookie失败，unb未找到");
		return false;
	}

	// 创建cookie表
	std::string sql = "CREATE TABLE IF NOT EXISTS cookies (\
		id TEXT PRIMARY KEY,\
		value TEXT NOT NULL,\
		user_id INTEGER NOT NULL,\
		auto_confirm INTEGER DEFAULT 1,\
		remark TEXT DEFAULT '',\
		pause_duration INTEGER DEFAULT 10,\
		username TEXT DEFAULT '',\
		password TEXT DEFAULT '',\
		show_browser INTEGER DEFAULT 0,\
		created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,\
		FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE\
		)";
	if (!m_sqliteUtility->ExecuteSQL(sql))
	{
		LogError(MODULE_INFO, "创建Cookie表失败: " + sql + "\n" + m_sqliteUtility->GetDBErrorMsg());
		return false;
	}

	//检查是否已存在，如果存在，则更新
	sql = "SELECT * FROM cookies WHERE id='" + unb + "'";
	std::vector<std::map<std::string, std::string>> resultList;
	if (m_sqliteUtility->GetTableData(sql, resultList))
	{
		if (resultList.size() > 0)
		{
			sql = "UPDATE cookies SET value='" + cookie + "' WHERE id='" + unb + "'";
			if (!m_sqliteUtility->ExecuteSQL(sql))
			{
				LogError(MODULE_INFO, "更新Cookie数据失败: " + sql + "\n" + m_sqliteUtility->GetDBErrorMsg());
				return false;
			}
			Log(MODULE_INFO, "更新Cookie成功: " + cookie);
			return true;
		}
	}
	else
	{
		sql = "INSERT INTO cookies(id, value) VALUES('" + unb + "', '" + cookie + "')";
		if (!m_sqliteUtility->ExecuteSQL(sql))
		{
			LogError(MODULE_INFO, "插入Cookie数据失败: " + sql + "\n" + m_sqliteUtility->GetDBErrorMsg());
			return false;
		}

		Log(MODULE_INFO, "添加Cookie成功: " + cookie);
	}

	return true;
}

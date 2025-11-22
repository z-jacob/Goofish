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
	std::string sql = "CREATE TABLE IF NOT EXISTS Cookie(unb VARCHAR(255) PRIMARY KEY, cookie VARCHAR(255))";
	if (!m_sqliteUtility->ExecuteSQL(sql))
	{
		LogError(MODULE_INFO, "创建Cookie表失败: " + sql + "\n" + m_sqliteUtility->GetDBErrorMsg());
		return false;
	}

	//检查是否已存在，如果存在，则更新
	sql = "SELECT * FROM Cookie WHERE unb='" + unb + "'";
	std::vector<std::map<std::string, std::string>> resultList;
	if (m_sqliteUtility->GetTableData(sql, resultList))
	{
		if (resultList.size() > 0)
		{
			sql = "UPDATE Cookie SET cookie='" + cookie + "' WHERE unb='" + unb + "'";
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
		sql = "INSERT INTO Cookie(unb, cookie) VALUES('" + unb + "', '" + cookie + "')";
		if (!m_sqliteUtility->ExecuteSQL(sql))
		{
			LogError(MODULE_INFO, "插入Cookie数据失败: " + sql + "\n" + m_sqliteUtility->GetDBErrorMsg());
			return false;
		}

		Log(MODULE_INFO, "添加Cookie成功: " + cookie);
	}

	return true;
}

#pragma once

#include <map>
#include <vector>
#include "../Helper/JFramework.h"
#include "../Helper/sqlite3.h"

using namespace JFramework;
class SQLiteUtility :public IUtility
{
public:
	SQLiteUtility() : m_pDB(nullptr) {}
	// Êý¾Ý¿â²Ù×÷
	bool OpenDB(const std::string& dbPath);
	void CloseDB();
	bool IsOpen() const;
	bool ExecuteSQL(std::string lpszSQL);
	bool IsChanged();
	bool GetTableData(std::string lpszSQL, std::vector<std::map<std::string, std::string>>& resultList);
	std::string GetDBErrorMsg();
private:
	sqlite3* m_pDB;
};

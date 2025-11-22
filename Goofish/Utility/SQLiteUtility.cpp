#include "SQLiteUtility.h"

// 打开数据库连接，如果已打开则直接返回true
bool SQLiteUtility::OpenDB(const std::string& dbPath)
{
	if (m_pDB != nullptr) {
		return true;
	}

	int nRet = sqlite3_open(dbPath.c_str(), &m_pDB);
	if (nRet != SQLITE_OK) {
		m_pDB = nullptr;
		return false;
	}

	// 启用外键约束，保证数据完整性
	ExecuteSQL("PRAGMA foreign_keys = ON;");

	return true;
}

// 关闭数据库连接，释放资源
void SQLiteUtility::CloseDB()
{
	if (m_pDB == nullptr) {
		return;
	}
	sqlite3_close(m_pDB);
	m_pDB = nullptr;
}

// 判断数据库是否已打开
bool SQLiteUtility::IsOpen() const
{
	return m_pDB != nullptr;
}

// 执行一条SQL语句（无结果集），返回是否成功
bool SQLiteUtility::ExecuteSQL(std::string lpszSQL)
{
	if (m_pDB == nullptr) return false;

	char* szError = nullptr;
	int nRet = sqlite3_exec(
		m_pDB,
		lpszSQL.c_str(),
		nullptr,
		nullptr,
		&szError
	);

	if (nRet != SQLITE_OK) {
		if (szError != nullptr) {
			sqlite3_free(szError);
		}
		return false;
	}

	return true;
}

bool SQLiteUtility::IsChanged()
{
	if (m_pDB == nullptr) return false;
	return sqlite3_changes(m_pDB) > 0;
}

// 执行查询SQL，获取表数据到resultList
// resultList: 每行为map<列名, 值>
bool SQLiteUtility::GetTableData(std::string lpszSQL, std::vector<std::map<std::string, std::string>>& resultList)
{
	resultList.clear();

	if (m_pDB == nullptr)
		return false;

	char** pResult = nullptr;
	int nRow = 0, nCol = 0;
	char* szError = nullptr;

	int nRet = sqlite3_get_table(
		m_pDB,
		lpszSQL.c_str(),
		&pResult,
		&nRow,
		&nCol,
		&szError
	);

	if (nRet != SQLITE_OK) {
		if (szError != nullptr) {
			sqlite3_free(szError);
		}
		return false;
	}

	if (nRow < 1 || nCol < 1) {
		// 没有数据，释放表并返回true
		sqlite3_free_table(pResult);
		return true;
	}

	// 第一行是列名
	std::vector<std::string> arrColNames;
	arrColNames.reserve(nCol);
	for (int i = 0; i < nCol; ++i) {
		arrColNames.push_back(pResult[i]);
	}

	// 处理每一行数据
	for (int i = 1; i <= nRow; ++i) {
		std::map<std::string, std::string> rowData;
		for (int j = 0; j < nCol; ++j) {
			// 处理空指针和空字符串
			const char* val = pResult[i * nCol + j];
			std::string strValue = val ? val : "";
			rowData[arrColNames[j]] = strValue;
		}
		resultList.push_back(std::move(rowData));
	}

	sqlite3_free_table(pResult);
	return true;
}

// 获取最近一次数据库错误信息
std::string SQLiteUtility::GetDBErrorMsg()
{
	if (m_pDB == nullptr)
		return "Database not opened";

	return sqlite3_errmsg(m_pDB);
}
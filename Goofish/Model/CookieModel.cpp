#include "CookieModel.h"
#include "../Helper/Utils.h"
#include "../Helper/Logger.h"

void CookieModel::OnInit()
{
}

void CookieModel::OnDeinit()
{

}

std::map<std::string, std::string> CookieModel::GetCookies()
{
	return m_cookieMap;
}

bool CookieModel::AddCookie(std::string cookie)
{
	auto unb = Utils::ExtractBetween(cookie, "unb=", ";");
	if (unb.length() <= 0)
	{
		LOG_ERROR(MODULE_INFO, "Ìí¼ÓCookieÊ§°Ü£¬unbÎ´ÕÒµ½");
		return false;
	}

	m_cookieMap[unb] = cookie;

	return true;
}

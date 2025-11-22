#include "CookieModel.h"
#include "../Helper/Utils.h"


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
		LogError(MODULE_INFO, "Ìí¼ÓCookieÊ§°Ü£¬unbÎ´ÕÒµ½");
		return false;
	}

	m_cookieMap[unb] = cookie;

	return true;
}

#pragma once
#include "../Helper/JFramework.h"
#include <map>
#include <string>
#include <iostream>

class CookieModel : public JFramework::AbstractModel
{
protected:
	void OnInit() override;


	void OnDeinit() override;

	std::map<std::string,std::string> m_cookieMap;

public:
	std::map<std::string, std::string> GetCookies();
	bool AddCookie(std::string cookie);
};


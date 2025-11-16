#pragma once
#include "../Helper/JFramework.h"
#include <vector>
#include <string>
#include <iostream>

class CookieModel : public JFramework::AbstractModel
{
protected:
	void OnInit() override;


	void OnDeinit() override;

public:
	std::vector<std::string> GetCookies();
};


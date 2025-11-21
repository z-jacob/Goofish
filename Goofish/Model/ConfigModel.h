#pragma once
#include "../Helper/JFramework.h"
class ConfigModel : public JFramework::AbstractModel
{
protected:
	// Í¨¹ý AbstractModel ¼Ì³Ð
	void OnInit() override;
	void OnDeinit() override;
public:
	std::string appKey;
	std::string appKeyStr;
	std::string deviceId;

};


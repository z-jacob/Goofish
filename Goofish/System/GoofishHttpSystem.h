#pragma once
#include "../Helper/JFramework.h"
#include "../Model/ConfigModel.h"
class GoofishHttpSystem :public JFramework::AbstractSystem
{
protected:
	// Í¨¹ý AbstractSystem ¼Ì³Ð
	void OnInit() override;
	void OnDeinit() override;
	void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;
	std::shared_ptr<ConfigModel> m_configModel;
public:
	bool Login(std::string cookie,std::string& response);
};


#pragma once
#include "System/WebsocketClientSystem.h"
#include "Model/CookieModel.h"
class CookieSystem : public JFramework::AbstractSystem
{
protected:
	void OnInit() override;


	void OnDeinit() override;


	void OnEvent(std::shared_ptr<JFramework::IEvent> event) override;

	std::shared_ptr<CookieModel> m_cookieModel;
};


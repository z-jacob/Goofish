#include "CookieSystem.h"

void CookieSystem::OnInit()
{
	m_cookieModel = this->GetModel<CookieModel>();
}

void CookieSystem::OnDeinit()
{

}

void CookieSystem::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{

}

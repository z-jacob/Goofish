#include "GoofishArchitecture.h"
#include "System/WebsocketClientSystem.h"
#include "Model/CookieModel.h"
#include "CookieSystem.h"

std::shared_ptr<GoofishArchitecture> GoofishArchitecture::instance = nullptr;
std::mutex GoofishArchitecture::mutex;

void GoofishArchitecture::Init()
{
	RegisterModel(std::make_shared<CookieModel>());

    RegisterSystem(std::make_shared<CookieSystem>());
	RegisterSystem(std::make_shared<WebsocketClientSystem>());
}

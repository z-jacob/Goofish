#include "pch.h"
#include "GoofishArchitecture.h"
#include "System/WebsocketClientSystem.h"

std::shared_ptr<GoofishArchitecture> GoofishArchitecture::instance = nullptr;
std::mutex GoofishArchitecture::mutex;

void GoofishArchitecture::Init()
{
	RegisterSystem(std::make_shared<WebsocketClientSystem>());
}

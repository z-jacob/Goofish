#include "GoofishArchitecture.h"
#include "System/WebsocketClientSystem.h"
#include "Model/CookieModel.h"
#include "System/CookieSystem.h"
#include "Model/FontModel.h"
#include "Model/UIModel.h"
#include "Model/ConfigModel.h"
#include "System/GoofishHttpSystem.h"
#include "Model/LogModel.h"
#include "Utility/SQLiteUtility.h"

std::shared_ptr<GoofishArchitecture> GoofishArchitecture::instance = nullptr;
std::mutex GoofishArchitecture::mutex;

void GoofishArchitecture::Init()
{
	RegisterUtility(std::make_shared<SQLiteUtility>());

	RegisterModel(std::make_shared<LogModel>());
	RegisterModel(std::make_shared<UIModel>());
	RegisterModel(std::make_shared<CookieModel>());
	RegisterModel(std::make_shared<FontModel>());
	RegisterModel(std::make_shared<ConfigModel>());

    RegisterSystem(std::make_shared<CookieSystem>());
	RegisterSystem(std::make_shared<WebsocketClientSystem>());
	RegisterSystem(std::make_shared<GoofishHttpSystem>());
}


#include "TabPageBase.h"
#include "../GoofishArchitecture.h"

std::weak_ptr<JFramework::IArchitecture> CTabPageBase::GetArchitecture() const
{
	return GoofishArchitecture::Instance();
}

// 空实现文件，为将来扩展保留位置

void CTabPageBase::OnEvent(std::shared_ptr<JFramework::IEvent> event)
{

}

#include "UIModel.h"
#include "../Helper/Utils.h"

void UIModel::OnInit()
{

}

void UIModel::OnDeinit()
{

}

int UIModel::GetWindowWidth() const
{
	return (int)(1024 * Utils::GetDpi());
}

int UIModel::GetWindowHeight() const
{
	return (int)(768 * Utils::GetDpi());
}

int UIModel::GetButtonWidth() const
{
	return (int)(60 * Utils::GetDpi());
}

int UIModel::GetButtonHeight() const
{
	return (int)(30 * Utils::GetDpi());
}

int UIModel::GetTabControlTop() const
{
	return (int)(40 * Utils::GetDpi());
}

int UIModel::GetControlSafeDistance() const
{
	return (int)(8 * Utils::GetDpi());
}

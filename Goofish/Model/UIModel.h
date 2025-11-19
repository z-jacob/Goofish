#pragma once
#include "../Helper/JFramework.h"
class UIModel : public JFramework::AbstractModel
{
protected:
	void OnInit() override;


	void OnDeinit() override;
public:
	int GetWindowWidth() const;
    int GetWindowHeight() const;
	int GetButtonWidth() const;
    int GetButtonHeight() const;
	int GetTabControlTop() const;
	int GetControlSafeDistance() const;
};


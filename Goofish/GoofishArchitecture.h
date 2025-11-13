#pragma once

#include <mutex>
#include <memory>
#include "Helper/JFramework.h"

class GoofishArchitecture : public JFramework::Architecture {
public:
    // ÉùÃ÷¾²Ì¬µÄ shared_ptr ÊµÀı
    static std::shared_ptr<GoofishArchitecture> Instance()
    {
        std::lock_guard<std::mutex> lock(mutex); // ¼ÓËø
        if (!instance)
        {
            instance = std::make_shared<GoofishArchitecture>();
            instance->InitArchitecture();
        }
        return instance;
    }

protected:
    void Init() override;

private:
    static std::shared_ptr<GoofishArchitecture> instance; // ¾²Ì¬ÊµÀı
    static std::mutex mutex; // ¾²Ì¬»¥³âËø
};


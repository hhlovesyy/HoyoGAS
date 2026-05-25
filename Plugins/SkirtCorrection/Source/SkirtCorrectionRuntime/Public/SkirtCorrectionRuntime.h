#pragma once

#include "Modules/ModuleManager.h"

class FSkirtCorrectionRuntimeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
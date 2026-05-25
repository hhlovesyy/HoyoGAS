#pragma once

#include "Modules/ModuleManager.h"

class FSkirtCorrectionEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
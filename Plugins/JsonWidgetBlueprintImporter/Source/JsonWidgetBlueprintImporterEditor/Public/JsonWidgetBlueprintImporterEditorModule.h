#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FJsonWidgetBlueprintImporterEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	static void ExportManifestForWidgetBlueprintEditor(TWeakPtr<class FWidgetBlueprintEditor> WidgetBlueprintEditor);
	static TSharedRef<class FExtender> ExtendWidgetBlueprintEditorToolbar(
		const TSharedRef<class FUICommandList> CommandList,
		TSharedRef<class FWidgetBlueprintEditor> WidgetBlueprintEditor);
	static void FillWidgetBlueprintEditorToolbar(
		class FToolBarBuilder& ToolBarBuilder,
		TWeakPtr<class FWidgetBlueprintEditor> WidgetBlueprintEditor);
	static void OpenSemanticsTab(TWeakPtr<class FWidgetBlueprintEditor> WidgetBlueprintEditor);

	void HandleRegisterLayoutExtensions(class FLayoutExtender& LayoutExtender);
	void HandleRegisterWidgetBlueprintTabs(
		const class FWidgetBlueprintApplicationMode& WidgetBlueprintApplicationMode,
		class FWorkflowAllowedTabSet& TabFactories);
	void RegisterMenus();
	void OpenImporterTab();
	TSharedRef<class SDockTab> SpawnImporterTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	FDelegateHandle RegisterLayoutExtensionsHandle;
	FDelegateHandle RegisterWidgetBlueprintTabsHandle;
	static const FName ImporterTabName;
};

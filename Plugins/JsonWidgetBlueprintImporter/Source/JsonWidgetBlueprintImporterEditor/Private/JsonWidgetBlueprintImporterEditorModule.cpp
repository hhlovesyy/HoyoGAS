#include "JsonWidgetBlueprintImporterEditorModule.h"

#include "JsonWidgetBlueprintSemantics.h"
#include "JsonWidgetBlueprintSemanticsTabSummoner.h"
#include "SJsonWidgetBlueprintImporterWindow.h"

#include "BlueprintModes/WidgetBlueprintApplicationMode.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "UMGEditorModule.h"
#include "WidgetBlueprintEditor.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FJsonWidgetBlueprintImporterEditorModule"

const FName FJsonWidgetBlueprintImporterEditorModule::ImporterTabName(TEXT("JsonWidgetBlueprintImporter"));

void FJsonWidgetBlueprintImporterEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		ImporterTabName,
		FOnSpawnTab::CreateRaw(this, &FJsonWidgetBlueprintImporterEditorModule::SpawnImporterTab))
		.SetDisplayName(LOCTEXT("ImporterTabTitle", "JSON Widget Blueprint Importer"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJsonWidgetBlueprintImporterEditorModule::RegisterMenus));

	IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
	UMGEditorModule.AddWidgetEditorToolbarExtender(
		IUMGEditorModule::FWidgetEditorToolbarExtender::CreateStatic(&FJsonWidgetBlueprintImporterEditorModule::ExtendWidgetBlueprintEditorToolbar));
	RegisterWidgetBlueprintTabsHandle = UMGEditorModule.OnRegisterTabsForEditor().AddRaw(
		this,
		&FJsonWidgetBlueprintImporterEditorModule::HandleRegisterWidgetBlueprintTabs);
	RegisterLayoutExtensionsHandle = UMGEditorModule.OnRegisterLayoutExtensions().AddRaw(
		this,
		&FJsonWidgetBlueprintImporterEditorModule::HandleRegisterLayoutExtensions);
}

void FJsonWidgetBlueprintImporterEditorModule::ShutdownModule()
{
	if (UToolMenus::TryGet() != nullptr)
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}

	if (FModuleManager::Get().IsModuleLoaded("UMGEditor"))
	{
		IUMGEditorModule& UMGEditorModule = FModuleManager::GetModuleChecked<IUMGEditorModule>("UMGEditor");
		if (RegisterWidgetBlueprintTabsHandle.IsValid())
		{
			UMGEditorModule.OnRegisterTabsForEditor().Remove(RegisterWidgetBlueprintTabsHandle);
		}

		if (RegisterLayoutExtensionsHandle.IsValid())
		{
			UMGEditorModule.OnRegisterLayoutExtensions().Remove(RegisterLayoutExtensionsHandle);
		}
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ImporterTabName);
}

void FJsonWidgetBlueprintImporterEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& Section = ToolsMenu->FindOrAddSection("JsonWidgetBlueprintImporter");
	Section.AddMenuEntry(
		"OpenJsonWidgetBlueprintImporter",
		LOCTEXT("OpenImporterLabel", "JSON Widget Blueprint Importer"),
		LOCTEXT("OpenImporterTooltip", "Open the JSON UI schema to Widget Blueprint importer."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FJsonWidgetBlueprintImporterEditorModule::OpenImporterTab)));
}

void FJsonWidgetBlueprintImporterEditorModule::HandleRegisterLayoutExtensions(FLayoutExtender& LayoutExtender)
{
	LayoutExtender.ExtendLayout(
		FTabId(TEXT("WidgetDetails")),
		ELayoutExtensionPosition::After,
		FTabManager::FTab(FJsonWidgetBlueprintSemanticsTabSummoner::TabID, ETabState::OpenedTab));
}

void FJsonWidgetBlueprintImporterEditorModule::HandleRegisterWidgetBlueprintTabs(
	const FWidgetBlueprintApplicationMode& WidgetBlueprintApplicationMode,
	FWorkflowAllowedTabSet& TabFactories)
{
	if (!WidgetBlueprintApplicationMode.GetBlueprintEditor().IsValid() ||
		TabFactories.GetFactory(FJsonWidgetBlueprintSemanticsTabSummoner::TabID).IsValid())
	{
		return;
	}

	TabFactories.RegisterFactory(MakeShared<FJsonWidgetBlueprintSemanticsTabSummoner>(
		WidgetBlueprintApplicationMode.GetBlueprintEditor()));
}

void FJsonWidgetBlueprintImporterEditorModule::OpenImporterTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ImporterTabName);
}

void FJsonWidgetBlueprintImporterEditorModule::OpenSemanticsTab(TWeakPtr<FWidgetBlueprintEditor> WidgetBlueprintEditor)
{
	if (TSharedPtr<FWidgetBlueprintEditor> WidgetBlueprintEditorPinned = WidgetBlueprintEditor.Pin())
	{
		WidgetBlueprintEditorPinned->InvokeTab(FTabId(FJsonWidgetBlueprintSemanticsTabSummoner::TabID));
	}
}

void FJsonWidgetBlueprintImporterEditorModule::ExportManifestForWidgetBlueprintEditor(TWeakPtr<FWidgetBlueprintEditor> WidgetBlueprintEditor)
{
	JsonWidgetBlueprintSemantics::ExportManifestWithDialog(WidgetBlueprintEditor.Pin());
}

TSharedRef<FExtender> FJsonWidgetBlueprintImporterEditorModule::ExtendWidgetBlueprintEditorToolbar(
	const TSharedRef<FUICommandList> CommandList,
	TSharedRef<FWidgetBlueprintEditor> WidgetBlueprintEditor)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddToolBarExtension(
		"Settings",
		EExtensionHook::After,
		CommandList,
		FToolBarExtensionDelegate::CreateStatic(
			&FJsonWidgetBlueprintImporterEditorModule::FillWidgetBlueprintEditorToolbar,
			TWeakPtr<FWidgetBlueprintEditor>(WidgetBlueprintEditor)));
	return Extender;
}

void FJsonWidgetBlueprintImporterEditorModule::FillWidgetBlueprintEditorToolbar(
	FToolBarBuilder& ToolBarBuilder,
	TWeakPtr<FWidgetBlueprintEditor> WidgetBlueprintEditor)
{
	ToolBarBuilder.AddSeparator();
	ToolBarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateStatic(
			&FJsonWidgetBlueprintImporterEditorModule::OpenSemanticsTab,
			WidgetBlueprintEditor)),
		NAME_None,
		LOCTEXT("OpenSemanticsTabLabel", "UI Semantics"),
		LOCTEXT("OpenSemanticsTabTooltip", "Open the UI semantics annotation panel for the current Widget Blueprint."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	ToolBarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateStatic(
			&FJsonWidgetBlueprintImporterEditorModule::ExportManifestForWidgetBlueprintEditor,
			WidgetBlueprintEditor)),
		NAME_None,
		LOCTEXT("ExportManifestLabel", "Export UI Manifest"),
		LOCTEXT("ExportManifestTooltip", "Export the current Widget Blueprint's BindWidget, callback, ViewModel field and semantic role data as JSON."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Save"));
}

TSharedRef<SDockTab> FJsonWidgetBlueprintImporterEditorModule::SpawnImporterTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SJsonWidgetBlueprintImporterWindow)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FJsonWidgetBlueprintImporterEditorModule, JsonWidgetBlueprintImporterEditor)

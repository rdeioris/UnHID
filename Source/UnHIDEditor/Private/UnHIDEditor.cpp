// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnHIDEditor.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructure.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "UnHIDBlueprintFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "FUnHIDEditorModule"

void FUnHIDEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner("UnHIDDashboard", FOnSpawnTab::CreateStatic(&FUnHIDEditorModule::CreateUnHIDDashboard))
		.SetDisplayName(LOCTEXT("UnHID Dashboard", "UnHID Dashboard"))
		.SetTooltipText(LOCTEXT("Open the UnHID Dashboard", "Open the UnHID Dashboard"))
		.SetIcon(FSlateIcon(
			FAppStyle::GetAppStyleSetName()
			, "DebugTools.TabIcon"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());
}

void FUnHIDEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

struct FUnHIDEditorDeviceInfo : public TSharedFromThis<FUnHIDEditorDeviceInfo>
{
	FUnHIDDeviceInfo DeviceInfo;
	FString ReportDescriptor;
	FUnHIDDeviceDescriptorReports Reports;
};

class SUnHIDDashboard : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SUnHIDDashboard)
		{
		}

	SLATE_END_ARGS()

	virtual ~SUnHIDDashboard() override
	{
		if (ConnectedUnHIDDevice.IsValid())
		{
			ConnectedUnHIDDevice->Terminate();
		}
		ConnectedUnHIDDevice = nullptr;
	}

	void SortHIDDeviceInfos(EColumnSortPriority::Type Priority, const FName& ColumnId, EColumnSortMode::Type Mode)
	{
		if (ColumnId == "Manufacturer")
		{
			HIDDeviceInfosManufacturerSortMode = Mode;

			HIDDeviceInfos.Sort([Mode](const TSharedRef<FUnHIDEditorDeviceInfo> A, const TSharedRef<FUnHIDEditorDeviceInfo> B)
				{
					return Mode == EColumnSortMode::Type::Descending ? A->DeviceInfo.Manufacturer > B->DeviceInfo.Manufacturer : A->DeviceInfo.Manufacturer < B->DeviceInfo.Manufacturer;
				});
		}
		else if (ColumnId == "Product")
		{
			HIDDeviceInfosProductSortMode = Mode;

			HIDDeviceInfos.Sort([Mode](const TSharedRef<FUnHIDEditorDeviceInfo> A, const TSharedRef<FUnHIDEditorDeviceInfo> B)
				{
					return Mode == EColumnSortMode::Type::Descending ? A->DeviceInfo.Product > B->DeviceInfo.Product : A->DeviceInfo.Product < B->DeviceInfo.Product;
				});
		}
		else if (ColumnId == "UsagePage")
		{
			HIDDeviceInfosUsagePageSortMode = Mode;

			HIDDeviceInfos.Sort([Mode](const TSharedRef<FUnHIDEditorDeviceInfo> A, const TSharedRef<FUnHIDEditorDeviceInfo> B)
				{
					return Mode == EColumnSortMode::Type::Descending ? A->DeviceInfo.UsagePage > B->DeviceInfo.UsagePage : A->DeviceInfo.UsagePage < B->DeviceInfo.UsagePage;
				});
		}
		else if (ColumnId == "Usage")
		{
			HIDDeviceInfosUsageSortMode = Mode;

			HIDDeviceInfos.Sort([Mode](const TSharedRef<FUnHIDEditorDeviceInfo> A, const TSharedRef<FUnHIDEditorDeviceInfo> B)
				{
					return Mode == EColumnSortMode::Type::Descending ? A->DeviceInfo.Usage > B->DeviceInfo.Usage : A->DeviceInfo.Usage < B->DeviceInfo.Usage;
				});
		}

		if (HIDDeviceInfosListView.IsValid())
		{
			HIDDeviceInfosListView->RequestListRefresh();
		}
	}

	void Construct(const FArguments& InArgs)
	{
		RefreshHIDDeviceInfos();

		ChildSlot[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(2)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SButton).Text(FText::FromString("Refresh")).OnClicked_Lambda([this] { RefreshHIDDeviceInfos();  HIDDeviceInfosListView->RequestListRefresh(); return FReply::Handled(); })
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(EVerticalAlignment::VAlign_Center).Padding(FMargin(2, 0))
						[
							SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("hidapi version: %s"), *UUnHIDBlueprintFunctionLibrary::UnHIDGetHidapiVersionString())))
						]
				]

			+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(0.5)
						[
							SNew(SScrollBox).AllowOverscroll(EAllowOverscroll::Yes)
								+ SScrollBox::Slot()
								[
									SAssignNew(HIDDeviceInfosListView, SListView<TSharedRef<FUnHIDEditorDeviceInfo>>).ListItemsSource(&HIDDeviceInfos)
										.OnGenerateRow(this, &SUnHIDDashboard::GenerateHIDDeviceInfoRow)
										.SelectionMode(ESelectionMode::Single).HeaderRow(
											SNew(SHeaderRow)
											+ SHeaderRow::Column("Manufacturer")
											.DefaultLabel(FText::FromString("Manufacturer"))
											.SortMode_Lambda([this] {return HIDDeviceInfosManufacturerSortMode; })
											.OnSort(this, &SUnHIDDashboard::SortHIDDeviceInfos)
											.FillWidth(0.3)
											+ SHeaderRow::Column("Product")
											.DefaultLabel(FText::FromString("Product"))
											.SortMode_Lambda([this] {return HIDDeviceInfosProductSortMode; })
											.OnSort(this, &SUnHIDDashboard::SortHIDDeviceInfos)
											.FillWidth(0.3)
											+ SHeaderRow::Column("UsagePage")
											.DefaultLabel(FText::FromString("Usage Page"))
											.SortMode_Lambda([this] {return HIDDeviceInfosUsagePageSortMode; })
											.OnSort(this, &SUnHIDDashboard::SortHIDDeviceInfos)
											.FillWidth(0.3)
											+ SHeaderRow::Column("Usage")
											.DefaultLabel(FText::FromString("Usage"))
											.SortMode_Lambda([this] {return HIDDeviceInfosUsageSortMode; })
											.OnSort(this, &SUnHIDDashboard::SortHIDDeviceInfos)
											.FillWidth(0.1)
										)
										.OnSelectionChanged(this, &SUnHIDDashboard::ChangeSelectedHIDDeviceInfo)
								]
						]
					+ SHorizontalBox::Slot().FillWidth(0.5)
						[
							SNew(SBorder).Visibility_Lambda([this] { return SelectedDeviceInfo.DeviceInfo.Path.IsEmpty() ? EVisibility::Hidden : EVisibility::Visible; }).Padding(8)
								[
									SNew(SScrollBox).AllowOverscroll(EAllowOverscroll::Yes)
										+ SScrollBox::Slot()
										[
											SNew(SVerticalBox)
												+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Path"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(SelectedDeviceInfo.DeviceInfo.Path); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Bus"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return UEnum::GetDisplayValueAsText((SelectedDeviceInfo.DeviceInfo.BusType)); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Interface Number"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(FString::Printf(TEXT("%d"), SelectedDeviceInfo.DeviceInfo.InterfaceNumber)); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Manufacturer"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(SelectedDeviceInfo.DeviceInfo.Manufacturer); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Vendor Id"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(FString::Printf(TEXT("0x%04X"), SelectedDeviceInfo.DeviceInfo.VendorId)); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Product"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(SelectedDeviceInfo.DeviceInfo.Product); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Product Id"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(FString::Printf(TEXT("0x%04X"), SelectedDeviceInfo.DeviceInfo.ProductId)); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Serial Number"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(SelectedDeviceInfo.DeviceInfo.SerialNumber); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Release Number"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(FString::Printf(TEXT("0x%04X"), SelectedDeviceInfo.DeviceInfo.ReleaseNumber)); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Usage Page"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(FString::Printf(TEXT("0x%02X (%s)"), SelectedDeviceInfo.DeviceInfo.UsagePage, *UUnHIDBlueprintFunctionLibrary::UnHIDUsagePageToString(SelectedDeviceInfo.DeviceInfo.UsagePage))); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Usage"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SEditableTextBox).Text_Lambda([this] { return FText::FromString(FString::Printf(TEXT("0x%02X"), SelectedDeviceInfo.DeviceInfo.Usage)); }).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Report Descriptor"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SMultiLineEditableTextBox).Text_Lambda([this] { return FText::FromString(SelectedDeviceInfo.ReportDescriptor); }).AutoWrapText(true).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Report Descriptor Inputs"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SMultiLineEditableTextBox).Text_Lambda([this] {return BuildDescriptorReport(SelectedDeviceInfo.Reports.Inputs); }).AutoWrapText(false).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Report Descriptor Outputs"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SMultiLineEditableTextBox).Text_Lambda([this] {return BuildDescriptorReport(SelectedDeviceInfo.Reports.Outputs); }).AutoWrapText(false).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.2)
														[
															SNew(STextBlock).Text(FText::FromString("Report Descriptor Features"))
														]
														+ SHorizontalBox::Slot().FillWidth(0.8)
														[
															SNew(SMultiLineEditableTextBox).Text_Lambda([this] {return BuildDescriptorReport(SelectedDeviceInfo.Reports.Features); }).AutoWrapText(false).IsReadOnly(true)
														]
												]
											+ SVerticalBox::Slot().AutoHeight().Padding(2)
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().AutoWidth()
														[
															SNew(SButton).Text_Lambda([this]() { return FText::FromString(ConnectedUnHIDDevice.IsValid() ? "Disconnect" : "Connect"); }).OnClicked(this, &SUnHIDDashboard::ConnectDisconnectUnHIDDevice).HAlign(EHorizontalAlignment::HAlign_Center)
														]
														+ SHorizontalBox::Slot().AutoWidth().VAlign(EVerticalAlignment::VAlign_Center).Padding(FMargin(2, 0))
														[
															SNew(STextBlock).Text(FText::FromString("Report Id: 0x")).IsEnabled_Lambda([this]() { return ConnectedUnHIDDevice.IsValid(); })
														]
														+ SHorizontalBox::Slot().AutoWidth()
														[
															SAssignNew(ConnectedUnHIDDeviceReportId, SEditableTextBox).IsEnabled_Lambda([this]() { return ConnectedUnHIDDevice.IsValid(); }).Text(FText::FromString("00"))
														]
														+ SHorizontalBox::Slot().AutoWidth().VAlign(EVerticalAlignment::VAlign_Center).Padding(FMargin(2, 0))
														[
															SNew(STextBlock).Text(FText::FromString("Force Feature Report Size (0 uses NumBytes from ReportDescriptor): ")).IsEnabled_Lambda([this]() { return ConnectedUnHIDDevice.IsValid(); })
														]
														+ SHorizontalBox::Slot().AutoWidth()
														[
															SAssignNew(ConnectedUnHIDDeviceFeatureReportSize, SEditableTextBox).IsEnabled_Lambda([this]() { return ConnectedUnHIDDevice.IsValid(); }).Text(FText::FromString("0"))
														]
														+ SHorizontalBox::Slot().AutoWidth()
														[
															SNew(SButton).Text(FText::FromString("GetFeatureReport")).IsEnabled_Lambda([this]() { return ConnectedUnHIDDevice.IsValid(); }).OnClicked(this, &SUnHIDDashboard::GetFeatureReportFromConnectedUnHIDDevice).HAlign(EHorizontalAlignment::HAlign_Center)
														]
												]
											+ SVerticalBox::Slot().MinHeight(384).MaxHeight(384)
												[
													SAssignNew(ConnectedUnHIDDeviceLog, SMultiLineEditableTextBox).IsReadOnly(true).Padding(8)
												]
												+ SVerticalBox::Slot().AutoHeight()
												[
													SNew(SHorizontalBox)
														+ SHorizontalBox::Slot().FillWidth(0.6)
														[
															SAssignNew(ConnectedUnHIDDeviceWrite, SEditableTextBox)
														]
														+ SHorizontalBox::Slot().FillWidth(0.1)
														[
															SNew(SButton).Text(FText::FromString("Write")).IsEnabled_Lambda([this]() { return ConnectedUnHIDDevice.IsValid(); }).OnClicked(this, &SUnHIDDashboard::WriteToConnectedUnHIDDevice).HAlign(EHorizontalAlignment::HAlign_Center)
														]
														+ SHorizontalBox::Slot().FillWidth(0.3)
														[
															SNew(SButton).Text(FText::FromString("SetFeatureReport")).IsEnabled_Lambda([this]() { return ConnectedUnHIDDevice.IsValid(); }).OnClicked(this, &SUnHIDDashboard::SetFeatureReportFromConnectedUnHIDDevice).HAlign(EHorizontalAlignment::HAlign_Center)
														]

												]
										]
								]
						]
				]
		];
	}

	static FText BuildDescriptorReport(const TArray<FUnHIDDeviceDescriptorReport>& DescriptorReports)
	{
		FString Reports;
		for (const FUnHIDDeviceDescriptorReport& DescriptorReport : DescriptorReports)
		{
			FString DescriptorReportItems;
			for (const FUnHIDDeviceDescriptorReportItem& DescriptorReportItem : DescriptorReport.Items)
			{
				FString Usages;
				for (const int64 Usage : DescriptorReportItem.Usage)
				{
					Usages += FString::Printf(TEXT("0x%02X, "), Usage);
				}
				DescriptorReportItems += FString::Printf(TEXT("\t\t\t\tBitOffset: %lld, BitSize: %lld, Count: %lld, UsagePage: 0x%02X, Usage: [%s], UsageMinimum: 0x%02X, UsageMaximum: 0x%02X, LogicalMinimum: %lld, LogicalMaximum: %lld\n"),
					DescriptorReportItem.BitOffset, DescriptorReportItem.BitSize, DescriptorReportItem.Count, DescriptorReportItem.UsagePage, *Usages.LeftChop(2),
					DescriptorReportItem.UsageMinimum, DescriptorReportItem.UsageMaximum,
					DescriptorReportItem.LogicalMinimum, DescriptorReportItem.LogicalMaximum);
			}
			Reports += FString::Printf(TEXT("ReportID: 0x%02X, NumBytes: %d\n%s\n"), DescriptorReport.ReportId, DescriptorReport.NumBytes, *DescriptorReportItems);
		}
		return FText::FromString(Reports.LeftChop(2));
	}

	TSharedRef<ITableRow> GenerateHIDDeviceInfoRow(TSharedRef<FUnHIDEditorDeviceInfo> Item, const TSharedRef<STableViewBase>& OwnerTable)
	{
		return SNew(STableRow<TSharedRef<FUnHIDEditorDeviceInfo>>, OwnerTable)
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.3)
					[
						SNew(STextBlock).Text(FText::FromString(Item->DeviceInfo.Manufacturer))
					]
					+ SHorizontalBox::Slot().FillWidth(0.3)
					[
						SNew(STextBlock).Text(FText::FromString(Item->DeviceInfo.Product))
					]
					+ SHorizontalBox::Slot().FillWidth(0.3)
					[
						SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%s (0x%02X)"), *UUnHIDBlueprintFunctionLibrary::UnHIDUsagePageToString(Item->DeviceInfo.UsagePage), Item->DeviceInfo.UsagePage)))
					]
					+ SHorizontalBox::Slot().FillWidth(0.1)
					[
						SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("0x%02X"), Item->DeviceInfo.Usage)))
					]
			];
	}

	void ChangeSelectedHIDDeviceInfo(TSharedPtr<FUnHIDEditorDeviceInfo> SelectedItem, ESelectInfo::Type SelectInfo)
	{
		if (ConnectedUnHIDDevice.IsValid())
		{
			ConnectedUnHIDDevice->Terminate();
		}
		ConnectedUnHIDDevice = nullptr;

		if (ConnectedUnHIDDeviceLog.IsValid())
		{
			ConnectedUnHIDDeviceLog->SetText(FText());
		}

		if (SelectedItem.IsValid())
		{
			SelectedDeviceInfo = *SelectedItem.Get();
		}
		else
		{
			SelectedDeviceInfo = FUnHIDEditorDeviceInfo();
		}
	}

	void RefreshHIDDeviceInfos()
	{
		if (ConnectedUnHIDDevice.IsValid())
		{
			ConnectedUnHIDDevice->Terminate();
		}
		ConnectedUnHIDDevice = nullptr;

		if (ConnectedUnHIDDeviceLog.IsValid())
		{
			ConnectedUnHIDDeviceLog->SetText(FText());
		}

		SelectedDeviceInfo = FUnHIDEditorDeviceInfo();

		HIDDeviceInfos.Empty();

		TArray<FUnHIDDeviceInfo> DeviceInfos = UUnHIDBlueprintFunctionLibrary::UnHIDEnumerate();
        
        // fix serial number, manufacturer and product (if required)
        for (FUnHIDDeviceInfo& DeviceInfo : DeviceInfos)
        {
            FString IgnoredErrorMessage;
            
            if (DeviceInfo.SerialNumber.IsEmpty())
            {
                DeviceInfo.SerialNumber = UUnHIDBlueprintFunctionLibrary::UnHIDGetSerialNumberString(DeviceInfo, IgnoredErrorMessage);
            }
            
            if (DeviceInfo.Manufacturer.IsEmpty())
            {
                DeviceInfo.Manufacturer = UUnHIDBlueprintFunctionLibrary::UnHIDGetManufacturerString(DeviceInfo, IgnoredErrorMessage);
            }
            
            if (DeviceInfo.Product.IsEmpty())
            {
                DeviceInfo.Product = UUnHIDBlueprintFunctionLibrary::UnHIDGetProductString(DeviceInfo, IgnoredErrorMessage);
            }
        }
        

		for (const FUnHIDDeviceInfo& DeviceInfo : DeviceInfos)
		{
			FString ErrorMessage;
			TArray<uint8> ReportDescriptor = UUnHIDBlueprintFunctionLibrary::UnHIDGetReportDescriptor(DeviceInfo, ErrorMessage);
			TSharedRef<FUnHIDEditorDeviceInfo> EditorDeviceInfoRef = MakeShared<FUnHIDEditorDeviceInfo>();
			EditorDeviceInfoRef->DeviceInfo = DeviceInfo;
            
			if (ReportDescriptor.IsEmpty())
			{
				EditorDeviceInfoRef->ReportDescriptor = FString::Printf(TEXT("Error: %s"), *ErrorMessage);
			}
			else
			{
				EditorDeviceInfoRef->ReportDescriptor = UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString(ReportDescriptor);
				EditorDeviceInfoRef->Reports = UUnHIDBlueprintFunctionLibrary::UnHIDGetReportsFromReportDescriptorBytes(ReportDescriptor, ErrorMessage);
			}
			HIDDeviceInfos.Add(EditorDeviceInfoRef);
		}

		HIDDeviceInfosManufacturerSortMode = EColumnSortMode::Type::None;
		HIDDeviceInfosProductSortMode = EColumnSortMode::Type::None;
		HIDDeviceInfosUsagePageSortMode = EColumnSortMode::Type::None;
		HIDDeviceInfosUsageSortMode = EColumnSortMode::Type::None;
	}

	FReply ConnectDisconnectUnHIDDevice()
	{
		if (ConnectedUnHIDDevice.IsValid())
		{
			ConnectedUnHIDDevice->Terminate();
			ConnectedUnHIDDevice = nullptr;
		}
		else
		{
			FUnHIDReadNativeDelegate UnHIDReadNativeDelegate;
			UnHIDReadNativeDelegate.BindLambda([this](UUnHIDDevice* UnHIDDevice, const TArray<uint8>& Data, const FString& ErrorMessage)
				{
					if (ConnectedUnHIDDeviceLog.IsValid())
					{
						FString NewLog = ConnectedUnHIDDeviceLog->GetText().ToString();
						if (!NewLog.IsEmpty())
						{
							NewLog += "\n";
						}
						if (Data.Num() > 0)
						{
							NewLog += "[" + FDateTime::Now().ToString() + "] Read: " + UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString(Data);
						}
						else
						{
							NewLog += "[" + FDateTime::Now().ToString() + "] Read Error: " + ErrorMessage;
						}
						ConnectedUnHIDDeviceLog->SetText(FText::FromString(NewLog));
						ConnectedUnHIDDeviceLog->ScrollTo(ETextLocation::EndOfDocument);
					}
				});
			FString ErrorMessage;
			UUnHIDDevice* UnHIDDevice = UUnHIDBlueprintFunctionLibrary::UnHIDOpenDevice(SelectedDeviceInfo.DeviceInfo, UnHIDReadNativeDelegate, ErrorMessage);
			if (UnHIDDevice)
			{
				ConnectedUnHIDDevice = TStrongObjectPtr<UUnHIDDevice>(UnHIDDevice);
			}
		}

		return FReply::Handled();
	}

	FReply WriteToConnectedUnHIDDevice()
	{
		if (!ConnectedUnHIDDevice.IsValid() || !ConnectedUnHIDDeviceWrite.IsValid() || !ConnectedUnHIDDeviceLog.IsValid())
		{
			return FReply::Handled();
		}

		FString ErrorMessage;
		const FString HexString = ConnectedUnHIDDeviceWrite->GetText().ToString();
		FString NewLog = ConnectedUnHIDDeviceLog->GetText().ToString();
		if (!NewLog.IsEmpty())
		{
			NewLog += "\n";
		}
		if (!ConnectedUnHIDDevice->WriteHexString(HexString, ErrorMessage))
		{
			NewLog += "[" + FDateTime::Now().ToString() + "] Write Error: " + ErrorMessage;

		}
		else
		{
			NewLog += "[" + FDateTime::Now().ToString() + "] Write: " + HexString;
		}

		ConnectedUnHIDDeviceLog->SetText(FText::FromString(NewLog));
		ConnectedUnHIDDeviceLog->ScrollTo(ETextLocation::EndOfDocument);

		return FReply::Handled();
	}

	FReply SetFeatureReportFromConnectedUnHIDDevice()
	{
		if (!ConnectedUnHIDDevice.IsValid() || !ConnectedUnHIDDeviceWrite.IsValid() || !ConnectedUnHIDDeviceLog.IsValid())
		{
			return FReply::Handled();
		}

		FString ErrorMessage;
		const FString HexString = ConnectedUnHIDDeviceWrite->GetText().ToString();
		FString NewLog = ConnectedUnHIDDeviceLog->GetText().ToString();
		if (!NewLog.IsEmpty())
		{
			NewLog += "\n";
		}
		if (!ConnectedUnHIDDevice->SetFeatureReportHexString(HexString, ErrorMessage))
		{
			NewLog += "[" + FDateTime::Now().ToString() + "] SetFeatureReport Error: " + ErrorMessage;
		}
		else
		{

			NewLog += "[" + FDateTime::Now().ToString() + "] SetFeatureReport: " + HexString;
		}

		ConnectedUnHIDDeviceLog->SetText(FText::FromString(NewLog));
		ConnectedUnHIDDeviceLog->ScrollTo(ETextLocation::EndOfDocument);

		return FReply::Handled();
	}

	FReply GetFeatureReportFromConnectedUnHIDDevice()
	{
		if (!ConnectedUnHIDDevice.IsValid() || !ConnectedUnHIDDeviceReportId.IsValid() || !ConnectedUnHIDDeviceFeatureReportSize.IsValid() || !ConnectedUnHIDDeviceLog.IsValid())
		{
			return FReply::Handled();
		}

		const TArray<uint8> ReportIdBytes = UUnHIDBlueprintFunctionLibrary::UnHIDHexStringToBytes(ConnectedUnHIDDeviceReportId->GetText().ToString());
		if (ReportIdBytes.Num() != 1)
		{
			return FReply::Handled();
		}

		FString ErrorMessage;
		FString HexString;

		FString NewLog = ConnectedUnHIDDeviceLog->GetText().ToString();
		if (!NewLog.IsEmpty())
		{
			NewLog += "\n";
		}

		int32 Size = FCString::Atoi(*ConnectedUnHIDDeviceFeatureReportSize->GetText().ToString());
		if (Size <= 0)
		{
			for (const FUnHIDDeviceDescriptorReport& DescriptorReportFeature : SelectedDeviceInfo.Reports.Features)
			{
				if (DescriptorReportFeature.ReportId == ReportIdBytes[0])
				{
					Size = DescriptorReportFeature.NumBytes;
					break;
				}
			}
		}

		if (!ConnectedUnHIDDevice->GetFeatureReportHexString(ReportIdBytes[0], Size, HexString, ErrorMessage))
		{
			NewLog += "[" + FDateTime::Now().ToString() + "] GetFeatureReport Error: " + ErrorMessage;
		}
		else
		{
			NewLog += "[" + FDateTime::Now().ToString() + "] GetFeatureReport: " + HexString;
		}

		ConnectedUnHIDDeviceLog->SetText(FText::FromString(NewLog));
		ConnectedUnHIDDeviceLog->ScrollTo(ETextLocation::EndOfDocument);

		return FReply::Handled();
	}

protected:
	TArray<TSharedRef<FUnHIDEditorDeviceInfo>> HIDDeviceInfos;
	TSharedPtr<SListView<TSharedRef<FUnHIDEditorDeviceInfo>>> HIDDeviceInfosListView;

	FUnHIDEditorDeviceInfo SelectedDeviceInfo;

	EColumnSortMode::Type HIDDeviceInfosManufacturerSortMode = EColumnSortMode::Type::None;
	EColumnSortMode::Type HIDDeviceInfosProductSortMode = EColumnSortMode::Type::None;
	EColumnSortMode::Type HIDDeviceInfosUsagePageSortMode = EColumnSortMode::Type::None;
	EColumnSortMode::Type HIDDeviceInfosUsageSortMode = EColumnSortMode::Type::None;

	TStrongObjectPtr<UUnHIDDevice> ConnectedUnHIDDevice = nullptr;
	TSharedPtr<SMultiLineEditableTextBox> ConnectedUnHIDDeviceLog = nullptr;
	TSharedPtr<SEditableTextBox> ConnectedUnHIDDeviceWrite = nullptr;
	TSharedPtr<SEditableTextBox> ConnectedUnHIDDeviceReportId = nullptr;
	TSharedPtr<SEditableTextBox> ConnectedUnHIDDeviceFeatureReportSize = nullptr;
};

TSharedRef<SDockTab> FUnHIDEditorModule::CreateUnHIDDashboard(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SUnHIDDashboard)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnHIDEditorModule, UnHIDEditor)

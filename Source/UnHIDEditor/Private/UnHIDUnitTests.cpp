// Copyright 2026 - Roberto De Ioris

#if WITH_DEV_AUTOMATION_TESTS
#include "UnHIDBlueprintFunctionLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnHIDUnitTests_SingleByteToHexString, "UnHID.UnitTests.SingleByteToHexString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnHIDUnitTests_SingleByteToHexString::RunTest(const FString& Parameters)
{
	TestEqual("{ 0 } == \"00\"", UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString({ 0 }), "00");

	TestEqual("{ 1 } == \"01\"", UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString({ 1 }), "01");

	TestEqual("{ 0xAB } == \"AB\"", UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString({ 0xAB }), "AB");

	TestEqual("{ 0xFF } == \"FF\"", UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString({ 0xFF }), "FF");

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnHIDUnitTests_BytesToHexString, "UnHID.UnitTests.BytesToHexString", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnHIDUnitTests_BytesToHexString::RunTest(const FString& Parameters)
{
	TestEqual("{ 0, 0, 0 } == \"00\"", UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString({ 0, 0, 0 }), "00 00 00");

	TestEqual("{ 1, 2, 0xA, 0xB } == \"01\"", UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString({ 1, 2, 0xA, 0xB }), "01 02 0A 0B");

	TestEqual("{ 0xAB, 0xCD, 0, 1, 0xFF } == \"AB CD 00 01 FF\"", UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString({ 0xAB, 0xCD, 0, 1, 0xFF }), "AB CD 00 01 FF");

	TestEqual("{ 0xFF, 0xFF } == \"FF FF\"", UUnHIDBlueprintFunctionLibrary::UnHIDBytesToHexString({ 0xFF, 0XFF }), "FF FF");

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnHIDUnitTests_ParseReportDescriptor, "UnHID.UnitTests.ParseReportDescriptor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnHIDUnitTests_ParseReportDescriptor::RunTest(const FString& Parameters)
{
	const FString ReportDescriptor = R"(
05 59 09 01 A1 01 09 02 A1 02 85 01 09 03 15 00 27 FF FF 00 00 75 10 95 01 B1 03 09 04 09 05 09
06 09 07 09 08 15 00 27 FF FF FF 7F 75 20 95 05 B1 03 C0 09 20 A1 02 85 02 09 21 15 00 27 FF FF
00 00 75 10 95 01 B1 02 C0 09 22 A1 02 85 03 09 21 15 00 27 FF FF 00 00 75 10 95 01 B1 02 09 23
09 24 09 25 09 27 09 26 15 00 27 FF FF FF 7F 75 20 95 05 B1 02 09 28 09 29 09 2A 09 2C 09 2D 15
00 26 FF 00 75 08 95 05 B1 02 C0 09 50 A1 02 85 04 09 03 09 55 15 00 25 08 75 08 95 02 B1 02 09
21 15 00 27 FF FF 00 00 75 10 95 08 B1 02 09 51 09 52 09 53 09 51 09 52 09 53 09 51 09 52 09 53
09 51 09 52 09 53 09 51 09 52 09 53 09 51 09 52 09 53 09 51 09 52 09 53 09 51 09 52 09 53 15 00
26 FF 00 75 08 95 18 B1 02 C0 09 60 A1 02 85 05 09 55 15 00 25 08 75 08 95 01 B1 02 09 61 09 62
15 00 27 FF FF 00 00 75 10 95 02 B1 02 09 51 09 52 09 53 15 00 26 FF 00 75 08 95 03 B1 02 C0 09
70 A1 02 85 06 09 71 15 00 25 01 75 08 95 01 B1 02 C0 85 07 06 00 FF 09 02 15 00 26 FF 00 75 08
95 3F B1 02 C0
	)";

	FString ErrorMessage;
	const FUnHIDDeviceDescriptorReports DescriptorReports = UUnHIDBlueprintFunctionLibrary::UnHIDGetReportsFromReportDescriptorBytes(UUnHIDBlueprintFunctionLibrary::UnHIDHexStringToBytes(ReportDescriptor), ErrorMessage);

	TestTrue("ErrorMessage.IsEmpty()", ErrorMessage.IsEmpty());

	TestTrue("DescriptorReports.bValid == true", DescriptorReports.bValid);

	TestEqual("DescriptorReports.Inputs.Num() == 0", DescriptorReports.Inputs.Num(), 0);

	TestEqual("DescriptorReports.Outputs.Num() == 0", DescriptorReports.Outputs.Num(), 0);

	TestEqual("DescriptorReports.Features.Num() == 0", DescriptorReports.Features.Num(), 7);

	TestEqual("DescriptorReports.Features[0].ReportId == 1", DescriptorReports.Features[0].ReportId, 1);
	TestEqual("DescriptorReports.Features[1].ReportId == 2", DescriptorReports.Features[1].ReportId, 2);
	TestEqual("DescriptorReports.Features[2].ReportId == 3", DescriptorReports.Features[2].ReportId, 3);
	TestEqual("DescriptorReports.Features[3].ReportId == 4", DescriptorReports.Features[3].ReportId, 4);
	TestEqual("DescriptorReports.Features[4].ReportId == 5", DescriptorReports.Features[4].ReportId, 5);
	TestEqual("DescriptorReports.Features[5].ReportId == 6", DescriptorReports.Features[5].ReportId, 6);
	TestEqual("DescriptorReports.Features[6].ReportId == 7", DescriptorReports.Features[6].ReportId, 7);

	TestEqual("DescriptorReports.Features[0].NumBytes == 22", DescriptorReports.Features[0].NumBytes, 22);
	TestEqual("DescriptorReports.Features[1].NumBytes == 2", DescriptorReports.Features[1].NumBytes, 2);
	TestEqual("DescriptorReports.Features[2].NumBytes == 22", DescriptorReports.Features[2].NumBytes, 27);
	TestEqual("DescriptorReports.Features[3].NumBytes == 42", DescriptorReports.Features[3].NumBytes, 42);
	TestEqual("DescriptorReports.Features[4].NumBytes == 8", DescriptorReports.Features[4].NumBytes, 8);
	TestEqual("DescriptorReports.Features[5].NumBytes == 8", DescriptorReports.Features[5].NumBytes, 1);
	TestEqual("DescriptorReports.Features[6].NumBytes == 63", DescriptorReports.Features[6].NumBytes, 63);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUnHIDUnitTests_ParseUnsignedInteger, "UnHID.UnitTests.ParseUnsignedInteger", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUnHIDUnitTests_ParseUnsignedInteger::RunTest(const FString& Parameters)
{
	TArray<uint8> Data = { 0x12, 0x34 };
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 8) == 0x12", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 8), 0x12);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 9) == 0x12", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 9), 0x12);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 10) == 0x12", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 10), 0x12);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 11) == 0x412", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 11), 0x412);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 12) == 0x412", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 12), 0x412);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 13) == 0x1412", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 13), 0x1412);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 14) == 0x3412", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 14), 0x3412);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 15) == 0x3412", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 15), 0x3412);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 0, 16) == 0x3412", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 16), 0x3412);

	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 8, 8) == 0x34", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 8, 8), 0x34);

	TestEqual("UnHIDParseUnsignedIntegerFromBytes({ 0x12, 0x34 }, 8, 4) == 0x4", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 8, 4), 0x4);

	Data = { 0x96, 0xB0, 0x84, 0x25, 0x00 };
	TestEqual("UnHIDParseUnsignedIntegerFromBytes( { 0x96, 0xB0, 0x84, 0x25, 0x00 }, 0, 11) == 0x96", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 0, 11), 0x96);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes( { 0x96, 0xB0, 0x84, 0x25, 0x00 }, 11, 11) == 0x96", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 11, 11), 0x96);
	TestEqual("UnHIDParseUnsignedIntegerFromBytes( { 0x96, 0xB0, 0x84, 0x25, 0x00 }, 22, 11) == 0x96", UUnHIDBlueprintFunctionLibrary::UnHIDParseUnsignedIntegerFromBytes(Data, 22, 11), 0x96);

	return true;
}

#endif
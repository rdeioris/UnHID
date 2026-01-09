// Copyright 2026 - Roberto De Ioris

using UnrealBuildTool;
using System.IO;

public class Hidapi : ModuleRules
{
    public Hidapi(ReadOnlyTargetRules Target) : base(Target)
    {
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("hidapi_winapi_EXPORTS=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDefinitions.Add("HID_API_EXPORT=__attribute__((visibility(\"default\")))");
            PublicFrameworks.AddRange(
                new string[] {
                    "IOKit",
                    "CoreFoundation"
                }
            );
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicDefinitions.Add("HID_API_EXPORT=__attribute__((visibility(\"default\")))");
            PublicSystemIncludePaths.Add("/usr/include");
            PublicSystemIncludePaths.Add("/usr/include/x86_64-linux-gnu");
            PublicSystemLibraryPaths.Add("/usr/lib/x86_64-linux-gnu");
            PublicAdditionalLibraries.Add("udev");
        }
    }
}

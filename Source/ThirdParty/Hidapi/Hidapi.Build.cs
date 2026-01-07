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
    }
}

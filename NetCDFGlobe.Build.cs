// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class NetCDFGlobe : ModuleRules
{
	public NetCDFGlobe(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        // Path to main folder, and thirdparty folder
        // https://wiki.unrealengine.com/Linking_Static_Libraries_Using_The_Build_System
        string projectPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../"));
        string thirdpartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Thirdparty/"));


        // Paths to the include, library (.lib) and dynamic (.dll)
        string incPath = Path.Combine(thirdpartyPath, "netCDF/include/");
        string libPath = Path.Combine(thirdpartyPath, "netCDF/lib/");
        string dllPath = Path.Combine(thirdpartyPath, "netCDF/bin/");

        // Include headers for our library
        PublicIncludePaths.Add(incPath);

        // Include the static library file
        // We include the library and copy the dll to the binary
        // http://stackoverflow.com/a/35920570
        PublicAdditionalLibraries.Add(libPath + "netcdf.lib");


        // Here we are copying over all the dll we need
        // These are just binaries for the netCDF
        CopyToBinaries(Target, projectPath, dllPath + "hdf.dll");
        CopyToBinaries(Target, projectPath, dllPath + "hdf5.dll");
        CopyToBinaries(Target, projectPath, dllPath + "hdf5_cpp.dll");
        CopyToBinaries(Target, projectPath, dllPath + "hdf5_hl.dll");
        CopyToBinaries(Target, projectPath, dllPath + "hdf5_hl_cpp.dll");
        CopyToBinaries(Target, projectPath, dllPath + "hdf5_tools.dll");
        CopyToBinaries(Target, projectPath, dllPath + "jpeg.dll");
        CopyToBinaries(Target, projectPath, dllPath + "mfhdf.dll");
        CopyToBinaries(Target, projectPath, dllPath + "msvcp120.dll");
        CopyToBinaries(Target, projectPath, dllPath + "msvcr120.dll");
        CopyToBinaries(Target, projectPath, dllPath + "netcdf.dll");
        CopyToBinaries(Target, projectPath, dllPath + "xdr.dll");
        CopyToBinaries(Target, projectPath, dllPath + "zlib1.dll");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }

    // Copy our dynamic libraries (DLL) files
    // These are needed a runtime, and should be in the binary folder
    // https://answers.unrealengine.com/questions/402515/how-to-modify-build-file-to-copy-dlls-to-binaries.html
    private void CopyToBinaries(TargetInfo Target, string projectPath, string dllPath)
    {

        // Calculate the location to copy
        string binariesDir = Path.Combine(projectPath, "Binaries", Target.Platform.ToString());
        string filename = Path.GetFileName(dllPath);

        // Create directory if not there
        if (!Directory.Exists(binariesDir))
            Directory.CreateDirectory(binariesDir);

        // Copy over the dll files
        if (!File.Exists(Path.Combine(binariesDir, filename)))
            File.Copy(dllPath, Path.Combine(binariesDir, filename), true);

    }
}

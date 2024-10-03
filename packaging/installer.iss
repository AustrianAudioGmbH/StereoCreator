#define Version Trim(FileRead(FileOpen("..\VERSION")))
#define ProductName 'StereoCreator3'
#define Publisher 'AustrianAudio'
#define Year GetDateTimeString("yyyy","","")
#define SC_BUILD_DIR GetEnv('SC_BUILD_DIR')
#define SC_BUILD_ARCHIVE GetEnv('SC_BUILD_ARCHIVE')
#define SC_BUILD_TYPE GetEnv('SC_BUILD_TYPE')

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppName={#ProductName}
OutputBaseFilename="{#ProductName}_{#GetEnv('SC_BUILD_MARK')}_Installer"
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3\{#ProductName}.vst3"
DisableDirPage=yes
OutputDir={#SC_BUILD_ARCHIVE}

; MAKE SURE YOU READ THE FOLLOWING!
LicenseFile="EULA"
UninstallFilesDir="{commonappdata}\{#ProductName}\uninstall"

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#ProductName}Data"

; MSVC adds a .ilk when building the plugin. Let's not include that.
[Files]
Source: "{#SC_BUILD_DIR}\StereoCreator_artefacts\{#SC_BUILD_TYPE}\VST3\{#ProductName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#ProductName}.vst3\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs;

[Run]
Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\VST3"; \
    Parameters: "/C mklink /D ""{commoncf64}\VST3\{#ProductName}Data"" ""{commonappdata}\{#ProductName}"""; \
    Flags: runascurrentuser;

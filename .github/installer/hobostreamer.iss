#ifndef MyAppVersion
  #define MyAppVersion "0.0.0"
#endif

#ifndef SourceDir
  #define SourceDir "payload"
#endif

#ifndef OutputDir
  #define OutputDir "."
#endif

#define MyAppName "HoboStreamer Studio"
#define MyAppPublisher "HoboStreamer"
#define MyAppURL "https://hobostreamer.com/"
#define MyAppExeName "bin\64bit\obs64.exe"

[Setup]
AppId={{7B0BE498-649C-452E-93CB-AAC92D912451}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\HoboStreamer Studio
DefaultGroupName=HoboStreamer Studio
DisableProgramGroupPage=yes
OutputDir={#OutputDir}
OutputBaseFilename=HoboStreamer-{#MyAppVersion}-Windows-x64-Installer
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupLogging=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\HoboStreamer Studio"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\HoboStreamer Studio"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch HoboStreamer Studio"; Flags: nowait postinstall skipifsilent

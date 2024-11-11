
; use Inno Setup to compile this file to an installation pack

#define AppName "The Force Engine VR"
;#define AppVersionNumber "v0.8.35-vr"
#define AppPublisher "So VR Here"
#define AppURL "https://github.com/SoVRHere/TheForceEngineVR"
#define DstExeName "TheForceEngineVR.exe"
;#define ProgramPath "BuildArtifacts/TheForceEngineVR"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{C7AF0339-4A5F-4910-A829-0945ED58644E}
AppName={#AppName}
AppVersion={#AppVersionNumber}
;AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppPublisher}\{#AppName}
DefaultGroupName={#AppPublisher}\{#AppName}
DisableProgramGroupPage=yes
LicenseFile=EULA.rtf
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputDir=..\..\BuildArtifacts\Installer\
OutputBaseFilename=TheForceEngineVR.Installer
SetupIconFile=tfe.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
WizardSmallImageFile=tfe55x55.bmp
WizardImageFile=tfe164x314.bmp
WizardImageStretch=no
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#DstExeName}
UninstallDisplayName={#AppName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Files]
Source: "{#ProgramPath}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
Source: "{#MSVCPath}\vc_redist.x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Dirs]
Name: {app}; Permissions: users-full

[Icons]
;Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#DstExeName}"
Name: "{group}\{#AppName}"; Filename: "{app}\{#DstExeName}";
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#DstExeName}"; Tasks: desktopicon

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/quiet /norestart"; Flags: waituntilterminated; StatusMsg: "Installing VC++ redistributables..."
Filename: "{app}\{#DstExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

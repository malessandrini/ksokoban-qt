[Setup]
AppName=ksokoban
AppVersion=0.5.0
DefaultDirName={pf}\ksokoban
DisableProgramGroupPage=yes
DisableWelcomePage=no
OutputBaseFilename=install_ksokoban
WizardImageFile=data/128-apps-ksokoban.bmp
WizardSmallImageFile=data/128-apps-ksokoban-small.bmp
LicenseFile=COPYING
ChangesAssociations=no
UninstallDisplayIcon={app}\ksokoban-qt.exe,0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
Source: "build\ksokoban-qt.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "doc\index.html"; DestDir: "{app}"
Source: "AUTHORS"; DestDir: "{app}"
Source: "COPYING"; DestDir: "{app}"
Source: "COPYING.DOC"; DestDir: "{app}"
Source: "NEWS"; DestDir: "{app}"
Source: "README"; DestDir: "{app}"
Source: "TODO"; DestDir: "{app}"
Source: "C:\Qt\Qt5.13.0\5.13.0\mingw73_32\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.13.0\5.13.0\mingw73_32\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.13.0\5.13.0\mingw73_32\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.13.0\Tools\mingw730_32\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.13.0\Tools\mingw730_32\bin\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.13.0\Tools\mingw730_32\bin\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{commonprograms}\ksokoban"; Filename: "{app}\ksokoban-qt.exe"
Name: "{commondesktop}\ksokoban"; Filename: "{app}\ksokoban-qt.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\ksokoban-qt.exe"; Description: "{cm:LaunchProgram,sokoban}"; Flags: nowait postinstall skipifsilent

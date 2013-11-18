!define PRODUCT_NAME XFrameFix
!define PRODUCT_DISPLAY_NAME "X:BtF frame fix (all users)"
!define APP_EXE XFrameFix.exe

!addplugindir .

OutFile ..\Release\${PRODUCT_NAME}PerMachineInstaller.exe
RequestExecutionLevel admin
#TargetMinimalOS 6.2

Name "${PRODUCT_DISPLAY_NAME}"

InstallDir $PROGRAMFILES\${PRODUCT_NAME}

Page instfiles
UninstPage instfiles

ShowInstDetails show
ShowUninstDetails show

Section Binaries
	SetShellVarContext all
	SetOutPath $INSTDIR
	File ..\Release\${APP_EXE}
SectionEnd

Section SetAsDebugger
	SetRegView 64
	WriteRegStr HKLM "Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\X.exe" Debugger "$INSTDIR\${APP_EXE}"
	SetRegView 32
	WriteRegStr HKLM "Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\X.exe" Debugger "$INSTDIR\${APP_EXE}"
SectionEnd

Section UninstallInformation
	WriteRegStr SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} DisplayName "${PRODUCT_DISPLAY_NAME}"
	WriteRegStr SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} UninstallString "$INSTDIR\Uninstall.exe"
	WriteRegStr SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} InstallLocation "$INSTDIR"
	WriteRegStr SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} Publisher "Jakub Berezanski"
	WriteUninstaller $INSTDIR\Uninstall.exe
SectionEnd

Section Uninstall
	SetShellVarContext all
	SetOutPath "$TEMP"
	Processes::KillProcess ${APP_EXE}
	Delete $INSTDIR\Uninstall.exe
	Delete $INSTDIR\${APP_EXE}
	RMDir $INSTDIR
	DeleteRegKey SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}
	SetAutoClose true
SectionEnd

Section un.SetAsDebugger
	SetRegView 64
	DeleteRegKey HKLM "Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\X.exe"
	SetRegView 32
	DeleteRegKey HKLM "Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\X.exe"
SectionEnd

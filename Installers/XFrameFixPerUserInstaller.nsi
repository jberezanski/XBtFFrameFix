!define PRODUCT_NAME XFrameFix
!define PRODUCT_DISPLAY_NAME "X:BtF frame fix (per-user)"
!define APP_EXE XFrameFix.exe

!addplugindir .

OutFile ..\Release\${PRODUCT_NAME}PerUserInstaller.exe
RequestExecutionLevel user
#TargetMinimalOS 6.2

Name "${PRODUCT_DISPLAY_NAME}"

InstallDir $LOCALAPPDATA\${PRODUCT_NAME}

Page instfiles

ShowInstDetails show
ShowUninstDetails show

Section Binaries
	SetShellVarContext current
	SetOutPath $INSTDIR
	File ..\Release\${APP_EXE}
SectionEnd

Section Autostart
	SetShellVarContext current
	SetOutPath $INSTDIR
	CreateShortCut "$SMSTARTUP\XBtF frame fix.lnk" $INSTDIR\${APP_EXE}
SectionEnd

Section UninstallInformation
	WriteRegStr HKCU Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} DisplayName "${PRODUCT_DISPLAY_NAME}"
	WriteRegStr HKCU Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} UninstallString "$INSTDIR\Uninstall.exe"
	WriteRegStr HKCU Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} InstallLocation "$INSTDIR"
	WriteRegStr HKCU Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} Publisher "Jakub Bere¿añski"
	WriteUninstaller $INSTDIR\Uninstall.exe
SectionEnd

Section Uninstall
	SetShellVarContext current
	SetOutPath "$TEMP"
	Processes::KillProcess ${APP_EXE}
	Delete $INSTDIR\Uninstall.exe
	Delete $INSTDIR\${APP_EXE}
	Delete "$SMSTARTUP\XBtF frame fix.lnk"
	RMDir $INSTDIR
	DeleteRegKey HKCU Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}
SectionEnd

!define PRODUCT_NAME XFrameFix
!define PRODUCT_DISPLAY_NAME "X:BtF frame fix (per-user)"
!define APP_EXE XFrameFix.exe

!addplugindir .

OutFile ..\Release\${PRODUCT_NAME}PerUserInstaller.exe
RequestExecutionLevel user
#TargetMinimalOS 6.2

Name "${PRODUCT_DISPLAY_NAME}"

InstallDir $LOCALAPPDATA\Programs\${PRODUCT_NAME}

Page instfiles
UninstPage instfiles

ShowInstDetails show
ShowUninstDetails show

Section Binaries
	SetShellVarContext current
	SetOutPath $INSTDIR
	File ..\Release\${APP_EXE}
SectionEnd

Section Autostart
	CreateShortCut "$SMSTARTUP\XBtF frame fix.lnk" $INSTDIR\${APP_EXE}
SectionEnd

Section UninstallInformation
	WriteRegStr SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} DisplayName "${PRODUCT_DISPLAY_NAME}"
	WriteRegStr SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} UninstallString "$INSTDIR\Uninstall.exe"
	WriteRegStr SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} InstallLocation "$INSTDIR"
	WriteRegStr SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} Publisher "Jakub Berezanski"
	WriteUninstaller $INSTDIR\Uninstall.exe
SectionEnd

Section Uninstall
	SetShellVarContext current
	SetOutPath "$TEMP"
	Processes::KillProcess ${APP_EXE}
	Delete $INSTDIR\Uninstall.exe
	Delete $INSTDIR\${APP_EXE}
	RMDir $INSTDIR
	DeleteRegKey SHCTX Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}
	SetAutoClose true
SectionEnd

Section un.Autostart
	Delete "$SMSTARTUP\XBtF frame fix.lnk"
SectionEnd

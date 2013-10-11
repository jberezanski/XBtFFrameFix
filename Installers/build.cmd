@echo off
echo Building installers
for %%a in (%~dp0*.nsi) do (
	echo makensis.exe /V2 %%a
	makensis.exe /V2 %%a || exit /b 1
)

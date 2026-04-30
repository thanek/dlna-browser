!ifndef VERSION
    !define VERSION "1.0.0"
!endif

!define APP_NAME "DLNA Browser"
!define APP_EXE "DlnaBrowser.exe"
!define REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\DlnaBrowser"

Name "${APP_NAME} ${VERSION}"
OutFile "DlnaBrowser-Windows-x64-Setup.exe"
InstallDir "$PROGRAMFILES64\DlnaBrowser"
InstallDirRegKey HKLM "Software\DlnaBrowser" "InstallPath"
RequestExecutionLevel admin

Unicode True

!include "MUI2.nsh"
!include "FileFunc.nsh"

!define MUI_ICON "resources\DlnaBrowser.ico"
!define MUI_UNICON "resources\DlnaBrowser.ico"
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Uruchom ${APP_NAME}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Polish"

Section "MainSection" SEC01
    SetOutPath "$INSTDIR"
    File /r "dist\bin\*"

    WriteUninstaller "$INSTDIR\Uninstall.exe"

    CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"

    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

    WriteRegStr HKLM "${REG_KEY}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "${REG_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKLM "${REG_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "${REG_KEY}" "DisplayIcon" "$INSTDIR\${APP_EXE}"
    WriteRegStr HKLM "${REG_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "${REG_KEY}" "Publisher" "DLNA Browser"
    WriteRegDWord HKLM "${REG_KEY}" "NoModify" 1
    WriteRegDWord HKLM "${REG_KEY}" "NoRepair" 1

    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "${REG_KEY}" "EstimatedSize" "$0"
SectionEnd

Section "Uninstall"
    Delete "$DESKTOP\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
    RMDir "$SMPROGRAMS\${APP_NAME}"

    RMDir /r "$INSTDIR"

    DeleteRegKey HKLM "${REG_KEY}"
    DeleteRegKey /ifempty HKLM "Software\DlnaBrowser"
SectionEnd

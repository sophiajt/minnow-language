; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

!include "EnvVarUpdate.nsh"

;--------------------------------

; The name of the installer
Name "Minnow"

; The file to write
OutFile "minnow.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Minnow

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Minnow" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Minnow (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "minnow.nsi"

  SetOutPath $INSTDIR\bin
  File "bin\minnowc.exe"
  File "libaquarium.dll"

  SetOutPath $INSTDIR\include\minnow
  FILE "aquarium\Actor.hpp"
  FILE "aquarium\Aquarium.hpp"
  FILE "aquarium\Char_String.hpp"
  FILE "aquarium\Common.hpp"
  FILE "aquarium\Message.hpp"
  FILE "aquarium\Message_Channel.hpp"
  FILE "aquarium\Minnow_Prelude.hpp"
  FILE "aquarium\Object_Feature.hpp"
  FILE "aquarium\Scheduler.hpp"
  FILE "aquarium\Typeless_Dictionary.hpp"
  FILE "aquarium\Typeless_Vector.hpp"

  SetOutPath $INSTDIR\lib
  FILE "libaquarium.dll.a"

  SetOutPath $INSTDIR\share\minnow
  FILE "prelude.mno"

  ${EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\bin"
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\bin"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Minnow "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Minnow" "DisplayName" "Minnow Programming Language"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Minnow" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Minnow" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Minnow" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Minnow"
  CreateShortCut "$SMPROGRAMS\Minnow\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  ;CreateShortCut "$SMPROGRAMS\Minnow\Minnow (MakeNSISW).lnk" "$INSTDIR\minnow.nsi" "" "$INSTDIR\minnow.nsi" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Minnow"
  DeleteRegKey HKLM SOFTWARE\Minnow

  ; Remove files and uninstaller
  Delete $INSTDIR\minnow.nsi
  Delete $INSTDIR\uninstall.exe
  Delete $INSTDIR\bin\minnowc.exe
  Delete $INSTDIR\bin\libaquarium.dll

  Delete $INSTDIR\include\minnow\Actor.hpp
  Delete $INSTDIR\include\minnow\Aquarium.hpp
  Delete $INSTDIR\include\minnow\Char_String.hpp
  Delete $INSTDIR\include\minnow\Common.hpp
  Delete $INSTDIR\include\minnow\Message.hpp
  Delete $INSTDIR\include\minnow\Message_Channel.hpp
  Delete $INSTDIR\include\minnow\Minnow_Prelude.hpp
  Delete $INSTDIR\include\minnow\Object_Feature.hpp
  Delete $INSTDIR\include\minnow\Scheduler.hpp
  Delete $INSTDIR\include\minnow\Typeless_Vector.hpp

  Delete $INSTDIR\lib\libaquarium.dll.a

  Delete $INSTDIR\share\minnow\prelude.mno
  RMDir /R $INSTDIR


  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\bin"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Minnow\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Minnow"
  RMDir "$INSTDIR"

SectionEnd

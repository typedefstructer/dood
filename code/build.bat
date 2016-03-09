@echo off
if not exist ..\build mkdir ..\build
pushd ..\build
      cl -Fegumd -nologo -FC -Zi ..\code\gumd.cpp user32.lib shell32.lib
popd

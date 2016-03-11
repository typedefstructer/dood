@echo off
if not exist ..\build mkdir ..\build
pushd ..\build
      rc -fodood.res -v ..\code\dood.rc
      cl -Fedood -nologo -FC -Zi ..\code\dood.cpp dood.res user32.lib shell32.lib
popd

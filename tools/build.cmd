@echo off
setlocal
where cl >nul 2>&1
if errorlevel 1 (
  echo Open an x64 Native Tools Command Prompt for Visual Studio, then run this script.
  exit /b 1
)
pushd "%~dp0.."
if not exist build mkdir build
rc /nologo /i "src\SDK\include" /fo "build\virus.res" "src\virus.rc"
if errorlevel 1 exit /b 1
cl /nologo /std:c++latest /EHsc /O2 /LD /MD /I "src\SDK\include" "src\virus.cpp" "build\virus.res" /Fo"build\virus.obj" /Fe"build\fmmp-plague-trace.dll" /link /IMPLIB:build\virus.lib
if errorlevel 1 exit /b 1
for %%T in (virus_test ramp_test pulse_test curses_test multitarget_test regen_timer_test soul_harvest_test) do (
  cl /nologo /std:c++latest /EHsc /O2 /MD /I "src\SDK\include" "src\%%T.cpp" /Fo"build\%%T.obj" /Fe"build\%%T.exe"
  if errorlevel 1 exit /b 1
  "build\%%T.exe"
  if errorlevel 1 exit /b 1
)
popd
echo Build and all seven regression suites passed.

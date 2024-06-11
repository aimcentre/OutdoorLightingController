@echo off
for /f "tokens=3" %%a in (
    'REG QUERY HKLM\HARDWARE\DEVICEMAP\SERIALCOMM'
) do set "COMPORT=%%a" 
echo %COMPORT%

for /f "tokens=2" %%a in (
    'MODE %COMPORT% ^| FIND /I "Baud"'
) do set "SPEED=%%a" 
echo %SPEED%
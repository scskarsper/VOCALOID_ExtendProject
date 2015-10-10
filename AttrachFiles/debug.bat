@echo off
cd D:\BPM\Vocaloid4
d:
del u2pa4_CHS.dll >> nul
COPY /y g2pa4_CHS\DEBUG\u2pa4.dll u2pa4_CHS.dll
echo %time%
choice /T 1 /D y >> nul
echo ===================================
dt.bat
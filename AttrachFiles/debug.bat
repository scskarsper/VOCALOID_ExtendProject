@echo off
cd D:\BPM\Vocaloid4
d:
del u2pa4_ENG.dll >> nul
COPY /y g2pa4_CHS\Debug\u2pa4.dll u2pa4_ENG.dll
echo %time%
choice /T 1 /D y >> nul
echo ===================================
dt.bat
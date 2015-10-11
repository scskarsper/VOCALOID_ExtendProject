@echo off
cd D:\Program Files\Vocaloids\Vocaloid4FE_2
d:
del u2pa4_CHS.dll >> nul
COPY /y G:\BMPE\VocaloidEP\VOCALOID_ExtendProject\Debug\u2pa4.dll u2pa4_CHS.dll
del g2pa4_JPN.dll >> nul
COPY /y G:\BMPE\VocaloidEP\VOCALOID_ExtendProject\Debug\u2pa4.dll g2pa4_JPN.dll
echo %time%
choice /T 1 /D y >> nul
echo ===================================
G:\BMPE\VocaloidEP\VOCALOID_ExtendProject\AttrachFiles\debug.bat
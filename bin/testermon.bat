set /p answer=Destroy Previous System State? [Y/N]:
if "%answer%" EQU "Y" del ./System_State_Files/ *.*
cls && cd .. && builderman.bat && cd ..\..\bin && start python GUI_Op.0.0.py && NT4
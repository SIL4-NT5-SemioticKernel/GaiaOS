cd src
md build
cd build
cmake ..
cmake --build .
cd..
copy build\debug\NT4.exe ..\bin\NT4.exe
pause
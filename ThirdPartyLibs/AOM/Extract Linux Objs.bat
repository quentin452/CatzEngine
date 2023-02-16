mkdir Linux\build\Release
cd Linux\build\Release
rmdir /s /q obj
mkdir obj
cd obj
..\..\..\..\..\AR\ar.exe -x ..\..\..\libaom.a
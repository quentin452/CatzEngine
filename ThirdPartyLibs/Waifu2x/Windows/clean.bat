rmdir /s /q .vs
rmdir /s /q Debug
rmdir /s /q "Generated Files"
rmdir /s /q x64\Debug

del *.sdf
del *.VC.db

del Release\*.obj
del Release\*.log
del Release\*.tlog
del Release\*.txt
del Release\*.lastbuildstate
del Release\*.recipe
rmdir /s /q Release\Waifu2x.tlog

del "Release Universal\*.obj"
del "Release Universal\*.log"
del "Release Universal\*.tlog"
del "Release Universal\*.txt"
del "Release Universal\*.lastbuildstate"
del "Release Universal\*.recipe"
rmdir /s /q "Release Universal\Waifu2x.tlog"

del x64\Release\*.obj
del x64\Release\*.log
del x64\Release\*.txt
del x64\Release\*.recipe
rmdir /s /q x64\Release\Waifu2x.tlog

del "x64\Release Universal\*.obj"
del "x64\Release Universal\*.log"
del "x64\Release Universal\*.txt"
del "x64\Release Universal\*.recipe"
rmdir /s /q "x64\Release Universal\Waifu2x.tlog"

del "ARM\Release Universal\*.obj"
del "ARM\Release Universal\*.log"
del "ARM\Release Universal\*.resfiles"
del "ARM\Release Universal\*.txt"
del "ARM\Release Universal\*.xml"
del "ARM\Release Universal\*.intermediate"
del "ARM\Release Universal\*.pri"
rmdir /s /q "ARM\Release Universal\embed"
rmdir /s /q "ARM\Release Universal\Waifu2x"
rmdir /s /q "ARM\Release Universal\Waifu2x.tlog"

del Emscripten\Release\*.o
del Emscripten\Release\*.log
del Emscripten\Release\*.tlog
del Emscripten\Release\*.lastbuildstate
del Emscripten\Release\*.unsuccessfulbuild
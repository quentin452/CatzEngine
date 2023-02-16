cd Windows
meson ../lib --default-library=static --backend vs
change code generation from MT DLL to MT, for projects:
	libdav1d
	libdav1d_bitdepth_16
	libdav1d_bitdepth_8
	libdav1d_entrypoint
compile
copy Windows\include\dav1d\version.h to dav1d\lib\include\dav1d\version.h
rename Windows\src\libdav1d.a as *.lib
delete everything in Windows except Windows\src\libdav1d.lib
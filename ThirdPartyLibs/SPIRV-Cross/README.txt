download https://github.com/KhronosGroup/SPIRV-Cross

EDIT "spirv_glsl.hpp":
		bool enable_row_major_load_workaround = true;
->
		bool enable_row_major_load_workaround = false;

BUT IN NEWER VERSIONS JUST USE
spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ENABLE_ROW_MAJOR_LOAD_WORKAROUND, SPVC_FALSE);
in "Shader Compiler.cpp", and remove these comments

update Esenthel\ThirdPartyLibs\SPIRV-Cross\include from package

use CMAKE Configure
SPIRV_CROSS_CLI = 0
SPIRV_ENABLE_TESTS = 0
SPIRV_ENABLE_UTIL = 0
Generate
open SPIRV-Cross.sln
set Release Config
select all projects and change "C/C++\Code Generation\Runtime Library=Multi-threaded (/MT)"
Right click on solution
Rebuild Solution

replace new libs to Esenthel\ThirdPartyLibs\SPIRV-Cross\lib
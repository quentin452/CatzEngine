import glob
import json
import os

# Set compilations ids/version (useless)
CMAKE_CXX_STANDARD = "11"
CMAKE_CXX_COMPILER_ID = "GNU"
CMAKE_CXX_COMPILER_VERSION = "10.2.0"

# Common compilation options
common_compile_options = [
    "-std=c++" + CMAKE_CXX_STANDARD,
    "-Wall",
    "-compilerandversion=" + CMAKE_CXX_COMPILER_ID + "-" + CMAKE_CXX_COMPILER_VERSION,
    "-Wmicrosoft",
    "-Wno-invalid-token-paste",
    "-Wno-unknown-pragmas",
    "-Wno-unused-value",
    "-fsyntax-only",
    "\\\"-D_MT\\\"",
    "\\\"-D_DLL\\\"",
    "\\\"-DWIN32\\\"",
    "\\\"-D_WINDOWS\\\"",
    "\\\"-DCMAKE_INTDIR=/\\\"Debug/\\\"\\\"",
    "\\\"-D_DEBUG_FUNCTIONAL_MACHINERY\\\""
]

# Add each include directory to the compilation options
ALL_INCLUDE_DIR = [os.getcwd() + "/include/"]
common_compile_options.extend(['-I\\\"' + include_dir + '\\\"' for include_dir in ALL_INCLUDE_DIR])

# Convert the list to a string with spaces between the options
common_compile_options_string = " ".join(common_compile_options)

# Filter the source files to keep only .cpp and .hpp files
PROJECT_SOURCE_DIR = os.getcwd()
SOURCES = glob.glob(PROJECT_SOURCE_DIR + '/**/*.c', recursive=True) + \
          glob.glob(PROJECT_SOURCE_DIR + '/**/*.cpp', recursive=True) + \
          glob.glob(PROJECT_SOURCE_DIR + '/**/*.hpp', recursive=True) + \
          glob.glob(PROJECT_SOURCE_DIR + '/**/*.h', recursive=True)

sources = [source for source in SOURCES if source.endswith(('.c', '.h'))]

# Create compilation database entries
compile_commands = []
for source_file in sources:
    compile_command = {
        "directory": str(PROJECT_SOURCE_DIR),
        "command": "\\\"" + str(CMAKE_CXX_COMPILER_ID) + "\\\" -x c++ \\\"" + source_file + "\\\" " + common_compile_options_string,
        "file": str(source_file)
    }
    compile_commands.append(compile_command)

# Write the compilation database to a JSON file
with open(os.path.join(PROJECT_SOURCE_DIR, "compile_commands.json"), "w") as f:
    json.dump(compile_commands, f, indent=4)

# Print confirmation message
print("Compilation database generated successfully.")
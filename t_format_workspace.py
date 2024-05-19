import glob
import os
import subprocess
import sys
import threading

BLACKLIST_FILES = [""]
BLACKLIST_DIRS = ["ThirdPartyLibs", "_Build_", "build", "CMakeFiles", "Animation", "Code", "Game", "Graphics", "Gui", "Math", "Mesh", "Misc", "Shaders", "Sound"]
FORMAT_STYLE = "{BasedOnStyle: llvm, IndentWidth: 4, ColumnLimit: 0}"

def format_files(files, params):
    for file in files:
        if os.path.basename(file) not in BLACKLIST_FILES and not any(blacklist_dir in file for blacklist_dir in BLACKLIST_DIRS):
            print(f"Formatting {file}...")
            subprocess.run(["clang-format", "-i", "-style", params, file], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            print(f"Skipping {file}...")

def main():
    files_to_format = sys.argv[1:] if len(sys.argv) > 1 else glob.glob("**/*.[ch]", recursive=True) + glob.glob("**/*.cpp", recursive=True) + glob.glob("**/*.cc", recursive=True)
    print(f"Formatting file(s) {files_to_format}")
    format_thread = threading.Thread(target=format_files, args=(files_to_format, FORMAT_STYLE))
    format_thread.start()
    format_thread.join()
    print("Done.")

if __name__ == "__main__":
    main()

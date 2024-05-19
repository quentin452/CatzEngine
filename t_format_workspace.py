import glob
import os
import subprocess
import sys
import threading

def format_files(files, params):
    blacklist_files = [""]
    blacklist_dirs = ["ThirdPartyLibs", "_Build_","build","CMakeFiles", "Animation", "Code", "Game", "Graphics", "Gui", "Math", "Mesh", "Misc", "Shaders", "Sound", "Physics", "Net", "Input"]
    for file in files:
        if os.path.basename(file) not in blacklist_files and not any(blacklist_dir in file for blacklist_dir in blacklist_dirs):
            print(f"Formatting {file}...")
            subprocess.run(["clang-format", "-i", "-style", params, file], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            print(f"Skipping {file}...")

if len(sys.argv) > 1:
    print(f"Formatting file(s) {sys.argv[1:]}")
    format_thread = threading.Thread(target=format_files, args=(sys.argv[1:], "{BasedOnStyle: llvm, IndentWidth: 4, ColumnLimit: 0}"))
    format_thread.start()
    format_thread.join()
    print("Done.")
else:
    print("Formatting...")
    files_to_format = glob.glob("**/*.[ch]", recursive=True) + glob.glob("**/*.cpp", recursive=True) + glob.glob("**/*.cc", recursive=True)
    format_thread = threading.Thread(target=format_files, args=(files_to_format, "{BasedOnStyle: llvm, IndentWidth: 4, ColumnLimit: 0}"))
    format_thread.start()
    format_thread.join()
    print("Done.")
import glob
import os
import subprocess
import sys
import threading

def format_files(files):
    blacklist = [""]
    blacklist_dirs = ["ThirdPartyLibs", "Shaders","Engine","Project","Source"]
    for file in files:
        if os.path.basename(file) not in blacklist and not any(blacklist_dir in file for blacklist_dir in blacklist_dirs):
            print(f"Formatting {file}...")
            subprocess.run(["clang-format", "-i", "-style=file", file], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            print(f"Skipping {file}...")

if len(sys.argv) > 1:
    print(f"formatting file(s) {sys.argv[1:]}")
    format_thread = threading.Thread(target=format_files, args=(sys.argv[1:],))
    format_thread.start()
    format_thread.join()
    print("done.")
else:
    print("formatting...")
    files_to_format = glob.glob("**/*.[chin]", recursive=True)
    format_thread = threading.Thread(target=format_files, args=(files_to_format,))
    format_thread.start()
    format_thread.join()
    print("done.")
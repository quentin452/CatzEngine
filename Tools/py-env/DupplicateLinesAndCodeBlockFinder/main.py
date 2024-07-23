# COPYRIGHT MIT : created by https://github.com/quentin452
import tkinter as tk
from tkinter import scrolledtext, filedialog, messagebox
from collections import Counter
import os

def find_duplicate_lines(code):
    lines = [line for line in code.split('\n') if line.strip() != '']  # Ignore empty lines
    line_counts = Counter(lines)
    duplicates = {line: count for line, count in line_counts.items() if count > 1}
    # Sort by count in descending order
    duplicates = dict(sorted(duplicates.items(), key=lambda item: item[1], reverse=True))
    return duplicates

def detect_duplicates():
    code = code_text.get("1.0", tk.END)
    line_duplicates = find_duplicate_lines(code)
    block_duplicates = find_duplicate_blocks(code)

    result_text.delete("1.0", tk.END)

    if line_duplicates:
        result_text.insert(tk.END, "Duplicate Lines:\n")
        for line, count in sorted(line_duplicates.items(), key=lambda item: item[1], reverse=True):
            # Add a separator and indentation for better readability
            result_text.insert(tk.END, f"\n--- Duplicate Line (Count: {count}) ---\n")
            result_text.insert(tk.END, f"{line}\n")
    else:
        result_text.insert(tk.END, "No duplicate lines found.\n")
    
    if block_duplicates:
        result_text.insert(tk.END, "\nDuplicate Blocks:\n")
        for block, count in sorted(block_duplicates.items(), key=lambda item: item[1], reverse=True):
            # Add a separator and indentation for better readability
            result_text.insert(tk.END, f"\n--- Duplicate Block (Count: {count}) ---\n")
            result_text.insert(tk.END, f"{block}\n\n")
    else:
        result_text.insert(tk.END, "No duplicate blocks found.\n")

def find_duplicate_blocks(code, block_size=4):
    lines = code.split('\n')
    blocks = ['\n'.join(lines[i:i+block_size]) for i in range(len(lines) - block_size + 1)]
    block_counts = Counter(blocks)
    duplicates = {block: count for block, count in block_counts.items() if count > 1}
    return dict(sorted(duplicates.items(), key=lambda item: item[1], reverse=True))

def open_file():
    file_path = filedialog.askopenfilename(filetypes=[("C++ Files", "*.cpp *.h"), ("All Files", "*.*")])
    if file_path:
        try:
            with open(file_path, 'r', encoding='utf-8') as file:
                code = file.read()
                code_text.delete("1.0", tk.END)
                code_text.insert(tk.END, code)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to read file: {e}")

def open_folder():
    folder_path = filedialog.askdirectory()
    if folder_path:
        code = ""
        try:
            # Traverse the folder and read all C++ files
            for root_dir, _, files in os.walk(folder_path):
                for file_name in files:
                    if file_name.endswith(('.cpp', '.h')):
                        file_path = os.path.join(root_dir, file_name)
                        with open(file_path, 'r', encoding='utf-8') as file:
                            code += file.read() + "\n"  # Add newline to separate file contents
            code_text.delete("1.0", tk.END)
            code_text.insert(tk.END, code)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to read files: {e}")

# Initialize the main window
root = tk.Tk()
root.title("C++ Code Duplicate Detector")

# Create and pack the widgets
open_file_button = tk.Button(root, text="Open File", command=open_file)
open_file_button.pack()

open_folder_button = tk.Button(root, text="Open Folder", command=open_folder)
open_folder_button.pack()

detect_button = tk.Button(root, text="Detect Duplicates", command=detect_duplicates)
detect_button.pack()

code_label = tk.Label(root, text="C++ Code:")
code_label.pack()

code_text = scrolledtext.ScrolledText(root, wrap=tk.WORD, width=80, height=20)
code_text.pack()

result_label = tk.Label(root, text="Results:")
result_label.pack()

result_text = scrolledtext.ScrolledText(root, wrap=tk.WORD, width=80, height=10)
result_text.pack()

# Run the main event loop
root.mainloop()
import glob
import os
import heapq
import logging

# Liste des dossiers et fichiers à ignorer
blacklist = ["ThirdPartyLibs", "CMakeFiles"]


def find_largest_files(files):
    sizes_files = []
    for file in files:
        size = os.path.getsize(file)
        with open(file, encoding="ISO-8859-1") as f:
            num_lines = len(f.readlines())
        ratio = size * num_lines / 10000 if num_lines > 0 else 0
        sizes_files.append((ratio, size, file))
    return sizes_files


def is_blacklisted(file, blacklist):
    return any(blacklisted in file for blacklisted in blacklist)


# Récupérer le nom du script sans l'extension
script_name = os.path.splitext(os.path.basename(__file__))[0]
log_filename = f"{script_name}.txt"

# Configurer le logger pour écrire dans un fichier .txt
logging.basicConfig(filename=log_filename, filemode="w", level=logging.INFO)

directories = [
    "**/*.c",  # C source files
    "**/*.h",  # C header files
    "**/*.cpp",  # C++ source files
    "**/*.hpp",  # C++ header files
    "**/*.cc",  # C++ source files
    "**/*.cxx",  # C++ source files
    "**/*.C",  # C++ source files (case sensitive)
    "**/*.hh",  # C++ header files
    "**/*.hxx",  # C++ header files
    "**/*.H",  # C++ header files (case sensitive)
    "**/*.inl",  # Inline include files
    "**/*.tcc",  # Model definition files
]

all_files = []
for directory in directories:
    files = glob.glob(directory, recursive=True)
    if files:
        files = [file for file in files if not is_blacklisted(file, blacklist)]
        all_files.extend(files)

# Trouver les n plus grands fichiers dans la liste globale
largest_files = find_largest_files(all_files)

# Logging des fichiers classés par taille décroissante
logged_files = set()
logging.info("Largest files:")
for ratio, size, file in sorted(largest_files, reverse=True, key=lambda x: x[0]):
    if file not in logged_files:
        logged_files.add(file)
        directory = next(
            (d for d in directories if glob.fnmatch.fnmatch(file, d)), None
        )
        logging.info(f"Directory: {directory}")
        logging.info(f"File: {file}")
        logging.info(f"Size: {size} bytes")
        try:
            with open(file, encoding="utf-8") as f:
                num_lines = len(f.readlines())
            logging.info(f"Number of lines: {num_lines}")
            logging.info(f"Size/Line ratio: {ratio}")
        except UnicodeDecodeError:
            logging.warning(f"Could not decode file: {file}")
        logging.info("")

# Vérification du nombre de fichiers loggés
logged_file_count = len(largest_files)
logging.info(f"Total logged files: {logged_file_count}")

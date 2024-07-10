import os
import subprocess

if 'MSBuild' in os.environ:
    msbuild_path = os.environ['MSBuild']
else:
    msbuild_path = None
    print("MSBuild is not defined in environment variable (CANNOT BUILD)")

print()
# Vérifie si la solution existe
if not os.path.exists("Titan.sln"):
    print("Titan.sln not found.")
    exit(1)

# Exécute msbuild
msbuild_command = [
    msbuild_path,
    '/verbosity:normal',
    'Titan.sln'
]
try:
    result = subprocess.run(msbuild_command, check=True)
    print("Build succeeded.")
except subprocess.CalledProcessError as e:
    print(f"Build failed: {e}")
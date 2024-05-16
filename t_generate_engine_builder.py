import os
import subprocess

if 'MSBuild' in os.environ:
    msbuild_path = os.environ['MSBuild']
else:
    msbuild_path = None
    print("MSBuild is not defined in environment variable (CANNOT BUILD)")

architectures = {
    1: "1) 64 bit",
    2: "1) 32 bit",
    3: "3) ARM",
    4: "4) Web",
    5: "5) Nintendo Switch",
    6: "6) Android"
}

configurations = {
    1: "Debug  DX11",
    2: "Debug GL",
    3: "Debug Universal DX11",
    4: "Release DX11",
    5: "Release GL",
    6: "Release Universal DX11"
}

print("Enter The Architecture :")
for index, architecture in architectures.items():
    print(f"{index}: {architecture}")

architectureIndex = int(input("Enter the number corresponding to your choice : "))
if architectureIndex not in architectures:
    print("Invalid architecture choice.")
    exit(1)
architecture = architectures[architectureIndex]

print("\nPlease enter the build configuration:")
for index, configuration in configurations.items():
    print(f"{index}: {configuration}")

configurationIndex = int(input("Enter the number corresponding to your choice : "))
if configurationIndex not in configurations:
    print("Invalid configuration choice.")
    exit(1)
configuration = configurations[configurationIndex]

print()
# Vérifie si la solution existe
if not os.path.exists("Titan.sln"):
    print("Titan.sln not found.")
    exit(1)

# Exécute msbuild
msbuild_command = [
    msbuild_path,
    f'/p:Configuration={configuration}',
    f'/p:Platform={architecture}',
    '/verbosity:normal',
    'Titan.sln'
]
try:
    result = subprocess.run(msbuild_command, check=True)
    print("Build succeeded.")
except subprocess.CalledProcessError as e:
    print(f"Build failed: {e}")
import os


if 'python' in os.environ:
    python_path = os.environ['python']
else:
    python_path = None
    print("python is not defined in environment variable (CANNOT BUILD)")

if 'MSBuild' in os.environ:
    msbuild_path = os.environ['MSBuild']
else:
    msbuild_path = None
    print("MSBuild is not defined in environment variable (CANNOT BUILD)")

architectures = {
    1: "x64",
    2: "x86",
    3: "ARM",
    4: "Web",
    5: "NintendoSwitch",
    6: "Android"
}

configurations = {
    1: "DebugDX11",
    2: "DebugGL",
    3: "DebugUniversalDX11",
    4: "ReleaseDX11",
    5: "ReleaseGL",
    6: "ReleaseUniversalDX11"
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
msbuild_command = f'"{msbuild_path}" /p:Configuration={configuration} /p:Platform={architecture} /verbosity:normal Titan.sln'
os.system(msbuild_command)

input("Press Enter to continue...")
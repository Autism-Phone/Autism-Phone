import subprocess
import os
import shutil
import platform

if not shutil.which("emcc"):
    print("Emscripten not found. Please install it and add it to your PATH.")
    exit(1)

def compile(file):
    output_file = os.path.join(output_folder, os.path.basename(file).replace(".cpp", ".js"))

    cmd = [
        "emcc", 
        file, 
        "-o", output_file, 
        "-O3",
        "-s", "EXPORTED_RUNTIME_METHODS=ccall,cwrap",
        "-s", "EXPORTED_FUNCTIONS=_main",
        "-s", "STACK_SIZE=1000000",
        "-s", "USE_SDL=2",
        "-s", "INITIAL_MEMORY=64MB",
        "-s", "ALLOW_MEMORY_GROWTH=1",
        "-s", "USE_WEBGL2=1",
    ]

    if platform.system() == "Windows":
        try:
            subprocess.run(cmd, check=True, stderr=subprocess.PIPE, text=True, shell=True)
            print(f"Compilation of {file} was successful.")
        except subprocess.CalledProcessError as e:
            print(f"Error compiling {file}:\n{e.stderr}")
    else:
        try:
            subprocess.run(cmd, check=True, stderr=subprocess.PIPE, text=True)
            print(f"Compilation of {file} was successful.")
        except subprocess.CalledProcessError as e:
            print(f"Error compiling {file}:\n{e.stderr}")



if platform.system() == "Windows":
    try:
        subprocess.run(["embuilder", "build", "sdl2"], check=True, stderr=subprocess.PIPE, text=True, shell=True)
        print(f"SDL2 built.")
    except subprocess.CalledProcessError as e:
        print(f"Error building SDL2:\n{e.stderr}")
else:
    try:
        subprocess.run(["embuilder", "build", "sdl2"], check=True, stderr=subprocess.PIPE, text=True)
        print(f"SDL2 built.")
    except subprocess.CalledProcessError as e:
        print(f"Error building SDL2:\n{e.stderr}")


input_folder = os.path.join(os.getcwd(), "src")
output_folder = os.path.join(os.getcwd(), "scripts", "compiled")

if not os.path.exists(output_folder):
    os.makedirs(output_folder)

files = [
    os.path.join(input_folder, f) 
    for f in os.listdir(input_folder) 
    if f.endswith(".cpp") and os.path.isfile(os.path.join(input_folder, f))
]

for file in files:
    compile(file)
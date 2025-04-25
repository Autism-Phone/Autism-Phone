import subprocess
import os
import shutil
import platform

if not shutil.which("emcc"):
    print("Emscripten not found. Please install it and add it to your PATH.")
    exit(1)

def compile(folder):
    cpp_files = [
        os.path.join(folder, f)
        for f in os.listdir(folder)
        if f.endswith(".cpp") and os.path.isfile(os.path.join(folder, f))
    ]

    if not cpp_files:
        print(f"No .cpp files found in {folder}. Skipping.")
        return

    output_file = os.path.join(output_folder, os.path.basename(folder) + ".js")

    cmd = [
        "emcc",
        *cpp_files,
        "-o", output_file,
        "-O3",
        "-s", "EXPORTED_RUNTIME_METHODS=ccall,cwrap",
        "-s", "EXPORTED_FUNCTIONS=_main",
        "-s", "STACK_SIZE=2000000",
        "-s", "USE_SDL=2",
        "-s", "INITIAL_MEMORY=64MB",
        "-s", "ALLOW_MEMORY_GROWTH=1",
        "-s", "USE_WEBGL2=1",
        "-s", "FETCH=1",
        "-lembind",
    ]

    try:
        subprocess.run(cmd, check=True, stderr=subprocess.PIPE, text=True, shell=(platform.system() == "Windows"))
        print(f"Compilation of {folder} was successful.")
    except subprocess.CalledProcessError as e:
        print(f"Error compiling {folder}:\n{e.stderr}")



try:
    subprocess.run(["embuilder", "build", "sdl2"],
                    check=True, stderr=subprocess.PIPE, text=True, shell=(platform.system() == "Windows"))
    print(f"SDL2 built.")
except subprocess.CalledProcessError as e:
    print(f"Error building SDL2:\n{e.stderr}")



input_folder = os.path.join(os.getcwd(), "src")
output_folder = os.path.join(os.getcwd(), "scripts", "compiled")

if not os.path.exists(output_folder):
    os.makedirs(output_folder)

folders = [
    os.path.join(input_folder, d)
    for d in os.listdir(input_folder)
    if os.path.isdir(os.path.join(input_folder, d))
]

for folder in folders:
    compile(folder)
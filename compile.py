import subprocess
import os
import shutil

print(shutil.which("emcc") + '\n')

def compile(file):
    output_file = os.path.join(output_folder, os.path.basename(file).replace(".cpp", ".js"))

    cmd = [
        "emcc", 
        file, 
        "-o", output_file, 
        "-O3",
        "-s", "EXTRA_EXPORTED_RUNTIME_METHODS=ccall,cwrap",
        "-s", "EXPORTED_FUNCTIONS=_main",
        "-s", "STACK_SIZE=1000000",
    ]

    try:
        subprocess.run(cmd, check=True, stderr=subprocess.PIPE, text=True, shell=True)
        print(f"Compilation of {file} was successful.")
    except subprocess.CalledProcessError as e:
        print(f"Error compiling {file}:\n{e.stderr}")


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
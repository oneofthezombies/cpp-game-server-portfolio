import sys
import shutil
import os
import subprocess


def clean():
    if os.path.exists("build"):
        shutil.rmtree("build")
    if os.path.exists("local"):
        shutil.rmtree("local")


def build():
    subprocess.run(
        [
            "cmake",
            "-S",
            ".",
            "-B",
            "build",
            "-G",
            "Ninja",
            "-DCMAKE_BUILD_TYPE=Debug",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
            f"-DCMAKE_INSTALL_PREFIX={os.getcwd()}/local",
        ],
        check=True,
    )
    subprocess.run(["cmake", "--build", "build"], check=True)
    subprocess.run(["cmake", "--install", "build"], check=True)


def print_help():
    print("Usage: python dev.py <command>")
    print("Commands:")
    print("  clean    - Clean the project")
    print("  build    - Build the project")


def main():
    args = sys.argv[1:]
    command = args[0] if args else "help"

    if command == "clean":
        clean()
    elif command == "build":
        build()
    else:
        print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()

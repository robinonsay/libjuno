from pathlib import Path
import shutil
import os
import re
from argparse import ArgumentParser

THIS_DIR = Path(__file__).parent.absolute()
PROJECT_DIR = THIS_DIR.parent
TEMPLATE_PATH = PROJECT_DIR / "templates" / "template_lib"

template_keywords = ["template", "Template", "TEMPLATE"]
impl_keywords = ["impl", "Impl", "IMPL"]


def rename_in_file(path: Path, old: str, new:str):
    new_contents = []
    with open(path, "r") as file:
        for line in file:
            if old.islower():
                new = new.lower()
            elif old.isupper():
                new = new.upper()
            elif old[0].isupper():
                new = new.capitalize()
                new = new.replace('_', '')
            new_contents.append(line.replace(old, new))
    with open(path, "w") as file:
        file.writelines(new_contents)

def copy_and_rename_template_files(lib_path: Path, implementations: list):
    shutil.copytree(TEMPLATE_PATH, lib_path)
    paths = [path for path in lib_path.rglob("*")]
    dirs = []
    impl_files = []
    for path in paths:
        if path.is_dir():
            dirs.append(path)
            continue
        for keyword in template_keywords:
            name = path.name
            rename_in_file(path, keyword, lib_path.name)
            if keyword not in name:
                continue
            new_name = name.replace(keyword, lib_path.name)
            print(f"Renaming {path.parent / name} -> {path.parent / new_name}")
            path = path.rename(path.parent / new_name)
        has_impl = False
        for implementation in implementations:
            for keyword in impl_keywords:
                name = path.name
                if keyword not in name:
                    continue
                new_name = name.replace(keyword, implementation)
                new_path = path.parent / new_name
                if not path.is_dir() and path != new_path:
                    shutil.copy2(path, new_path)
                    print(f"Creating {new_path}")
                    impl_files.append((new_path, implementation))
                    has_impl = True
        if has_impl:
            os.remove(path)
    for path, impl in impl_files:
        for keyword in impl_keywords:
            rename_in_file(path, keyword, impl)
    for path in dirs:
        for keyword in template_keywords:
            name = path.name
            if keyword not in name:
                continue
            new_name = name.replace(keyword, lib_path.name)
            print(f"Renaming {path.parent / name} -> {path.parent / new_name}")
            path = path.rename(path.parent / new_name)

def main():
    parser = ArgumentParser()
    parser.add_argument("lib_path")
    parser.add_argument("implementation_names", nargs='+')
    args = parser.parse_args()
    lib_path = Path(args.lib_path)
    impl_names = args.implementation_names
    copy_and_rename_template_files(lib_path, impl_names)


if __name__ == '__main__':
    main()

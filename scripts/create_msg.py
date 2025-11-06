from pathlib import Path
import shutil
import os
import re
from argparse import ArgumentParser

THIS_DIR = Path(__file__).parent.absolute()
PROJECT_DIR = THIS_DIR.parent
TEMPLATE_PATH = PROJECT_DIR / "templates" / "template_msg"

template_keywords = ["template", "Template", "TEMPLATE"]
impl_keywords = ["msg", "Msg", "MSG"]


def rename_in_file(path: Path, old: str, new:str):
    new_contents = []
    with open(path, "r") as file:
        for line in file:
            if old.islower():
                new_mod = new.lower()
            elif old.isupper():
                new_mod = new.upper()
            elif old[0].isupper():
                new_split = new.split('_')
                new_split = [w.capitalize() for w in new_split]
                new_mod = ''.join(new_split)
            new_contents.append(line.replace(old, new_mod))
    with open(path, "w") as file:
        file.writelines(new_contents)

def copy_and_rename_template_files(lib_path: Path):
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
    args = parser.parse_args()
    lib_path = Path(args.lib_path)
    copy_and_rename_template_files(lib_path)


if __name__ == '__main__':
    main()

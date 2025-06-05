#!/usr/bin/env python3
import os
import argparse

# The license header to prepend
LICENSE_HEADER = """/**
    @file
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/
"""

# A unique substring to detect if the header is already present
HEADER_MARKER = "Copyright (c) 2025 Robin A. Onsay"

def prepend_header_to_file(file_path: str, header: str) -> None:
    """
    Reads the entire contents of `file_path`, checks the first few lines for HEADER_MARKER,
    and if not found, rewrites the file with `header` prepended to the original contents.
    """
    # Read the original contents
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Check the first few lines for the marker
    first_lines = "\n".join(content.splitlines()[:5])
    if HEADER_MARKER in first_lines:
        print(f"Skipping (header already present): {file_path}")
        return

    # Prepend the header since it wasn't found
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(header + '\n' + content)
    print(f"Prepended header to: {file_path}")

def process_directory(directory: str, header: str) -> None:
    """
    Walks through `directory` (recursively), and for every file ending in `.h`,
    calls prepend_header_to_file() to add the license header if needed.
    """
    for root, _, files in os.walk(directory):
        for filename in files:
            if filename.lower().endswith('.h')      or \
                filename.lower().endswith('.hpp')   or \
                filename.lower().endswith('.c')     or \
                filename.lower().endswith('.cpp'):
                full_path = os.path.join(root, filename)
                prepend_header_to_file(full_path, header)

def main():
    parser = argparse.ArgumentParser(
        description="Prepend a license header to every .h file in a directory (if not already present)."
    )
    parser.add_argument(
        "directory",
        help="Path to the directory containing .h files (will recurse into subdirectories)."
    )
    args = parser.parse_args()
    target_dir = args.directory

    if not os.path.isdir(target_dir):
        print(f"Error: '{target_dir}' is not a valid directory.")
        return

    process_directory(target_dir, LICENSE_HEADER)

if __name__ == "__main__":
    main()

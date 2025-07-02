#!/usr/bin/env python3
import os
import re
import argparse

def extract_docs_from_file(filepath):
    """
    Extract all /**DOC ... */ blocks from the given file and clean up the content.
    Returns a list of markdown strings.
    """
    # Regex to match /**DOC ... */ including multiline
    pattern = re.compile(r"/\*\*DOC(.*?)\*/", re.DOTALL)
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    matches = pattern.findall(content)
    docs = []
    for block in matches:
        # Split into lines and strip leading '*' and whitespace
        lines = block.strip().splitlines()
        cleaned_lines = []
        for line in lines:
            # Remove optional leading '*' and any leading spaces
            cleaned = re.sub(r'^ {4}', '', line)
            cleaned_lines.append(cleaned)
        docs.append('\n'.join(cleaned_lines).strip())
    return docs


def main(root_dir):
    """
    Walk the project directory, find .c and .h files, extract docs, and write .md files.
    """
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith(('.c', '.h', '.cpp', '.hpp')):
                filepath = os.path.join(dirpath, filename)
                docs = extract_docs_from_file(filepath)
                if docs:
                    base, _ = os.path.splitext(filename)
                    md_filename = f"{base.upper()}.md"
                    md_path = os.path.join(dirpath, md_filename)
                    with open(md_path, 'w', encoding='utf-8') as md_file:
                        # Join multiple doc blocks with two newlines
                        md_file.write('\n\n'.join(docs))
                    print(f"Extracted {len(docs)} DOC block(s) from {filename} -> {md_filename}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Extract /**DOC ... */ blocks to Markdown files')
    parser.add_argument('root', nargs='?', default='.', help='Root directory of the project')
    args = parser.parse_args()
    main(args.root)

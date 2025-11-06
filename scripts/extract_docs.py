#!/usr/bin/env python3
import os
import re
import json
import argparse

def extract_docs_from_file(filepath):
    """
    Extract all /**DOC ... */ blocks from the given file and clean up the content.
    Also extracts all code between consecutive /**DOC blocks (or to end of file).
    Returns a list of markdown strings with embedded code blocks.
    
    If /**END*/ is encountered, code extraction stops at that marker.
    """
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Regex to match /**DOC ... */ including multiline, capturing end position
    doc_pattern = re.compile(r"/\*\*DOC(.*?)\*/", re.DOTALL)
    # Regex to match /**END*/ marker
    end_pattern = re.compile(r"/\*\*END\*/")

    docs = []
    doc_matches = list(doc_pattern.finditer(content))
    end_match = end_pattern.search(content)
    end_pos_limit = end_match.start() if end_match else len(content)

    for idx, match in enumerate(doc_matches):
        doc_block = match.group(1)
        end_pos = match.end()

        # Clean up documentation block
        lines = doc_block.strip().splitlines()
        cleaned_lines = []
        for line in lines:
            # Remove optional leading '*' and any leading spaces (keep code alone untouched)
            cleaned = re.sub(r'^ {4}', '', line)
            cleaned_lines.append(cleaned)
        doc_text = '\n'.join(cleaned_lines).strip()

        # Determine the end position for code extraction
        if idx + 1 < len(doc_matches):
            # There's another /**DOC block - extract code up to it
            next_match = doc_matches[idx + 1]
            code_end_pos = next_match.start()
        else:
            # This is the last /**DOC block - extract to end of file or /**END*/
            code_end_pos = end_pos_limit

        # Extract code between this /**DOC and the next (or end of file/END marker)
        code_section = content[end_pos:code_end_pos]
        # IMPORTANT: Do NOT strip leading whitespace; we want to preserve indentation.
        # If you want to remove only trailing whitespace, use rstrip() instead:
        # code_section = code_section.rstrip()

        if code_section:
            # Combine doc and code
            combined = f"{doc_text}\n\n```cpp\n{code_section}\n```"
            docs.append(combined)
        else:
            # No code after this doc block
            docs.append(doc_text)

    return docs


def process_tutorial(tutorial_path, base_dir):
    """
    Process a tutorial.json file and generate a combined markdown file
    from the specified source files in order.
    """
    with open(tutorial_path, 'r', encoding='utf-8') as f:
        tutorial_config = json.load(f)

    title = tutorial_config.get('title', 'Tutorial')
    files = tutorial_config.get('files', [])

    if not files:
        print(f"Warning: No files specified in {tutorial_path}")
        return

    all_docs = []

    # Process each file in order
    for file_entry in files:
        file_path = file_entry.get('path')
        if not file_path:
            continue

        # Resolve path relative to the tutorial.json location
        full_path = os.path.join(base_dir, file_path)

        if not os.path.exists(full_path):
            print(f"Warning: File not found: {full_path}")
            continue

        print(f"  Processing: {file_path}")
        docs = extract_docs_from_file(full_path)

        if docs:
            # Add a section header for this file
            file_section = f"## {os.path.basename(file_path)}\n\n" + '\n\n'.join(docs)
            all_docs.append(file_section)

    if all_docs:
        # Write combined markdown file
        md_filename = f"{title}.md".replace(' ', '_')
        md_path = os.path.join(base_dir, md_filename)

        with open(md_path, 'w', encoding='utf-8') as md_file:
            md_file.write(f"# {title}\n\n")
            md_file.write('\n\n'.join(all_docs))

        print(f"Created tutorial: {md_filename} with {len(files)} source file(s)")
    else:
        print(f"Warning: No documentation found in specified files for {tutorial_path}")


def main(root_dir):
    """
    Walk the project directory, find tutorial.json files or .c/.h files, 
    extract docs, and write .md files.
    """
    for dirpath, _, filenames in os.walk(root_dir):
        # Check for tutorial.json first
        if 'tutorial.json' in filenames:
            tutorial_path = os.path.join(dirpath, 'tutorial.json')
            print(f"Found tutorial configuration: {tutorial_path}")
            process_tutorial(tutorial_path, dirpath)
            break# Skip individual file processing in this directory

        # Otherwise, process individual files
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
                        md_file.write(f"# {base.capitalize()} Docs\n\n")
                        md_file.write('\n\n'.join(docs))
                    print(f"Extracted {len(docs)} DOC block(s) from {filename} -> {md_filename}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Extract /**DOC ... */ blocks to Markdown files')
    parser.add_argument('root', nargs='?', default='.', help='Root directory of the project')
    args = parser.parse_args()
    main(args.root)

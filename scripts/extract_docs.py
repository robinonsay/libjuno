#!/usr/bin/env python3
import os
import re
import argparse

def extract_docs_from_file(filepath):
    """
    Extract all /**DOC ... */ blocks from the given file and clean up the content.
    Also extracts all code between consecutive /**DOC blocks (or to end of file).
    Returns a list of markdown strings with embedded code blocks.
    """
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Regex to match /**DOC ... */ including multiline, capturing end position
    pattern = re.compile(r"/\*\*DOC(.*?)\*/", re.DOTALL)
    
    docs = []
    matches = list(pattern.finditer(content))
    
    for idx, match in enumerate(matches):
        doc_block = match.group(1)
        end_pos = match.end()
        
        # Clean up documentation block
        lines = doc_block.strip().splitlines()
        cleaned_lines = []
        for line in lines:
            # Remove optional leading '*' and any leading spaces
            cleaned = re.sub(r'^ {4}', '', line)
            cleaned_lines.append(cleaned)
        doc_text = '\n'.join(cleaned_lines).strip()
        
        # Determine the end position for code extraction
        if idx + 1 < len(matches):
            # There's another /**DOC block - extract code up to it
            next_match = matches[idx + 1]
            code_end_pos = next_match.start()
        else:
            # This is the last /**DOC block - extract to end of file
            code_end_pos = len(content)
        
        # Extract code between this /**DOC and the next (or end of file)
        code_section = content[end_pos:code_end_pos]
        
        # Strip leading/trailing whitespace but preserve internal structure
        code_section = code_section.strip()
        
        if code_section:
            # Combine doc and code
            combined = f"{doc_text}\n\n```cpp\n{code_section}\n```"
            docs.append(combined)
        else:
            # No code after this doc block
            docs.append(doc_text)
    
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
                        md_file.write(f"# {base.capitalize()} Docs\n\n")
                        md_file.write('\n\n'.join(docs))
                    print(f"Extracted {len(docs)} DOC block(s) from {filename} -> {md_filename}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Extract /**DOC ... */ blocks to Markdown files')
    parser.add_argument('root', nargs='?', default='.', help='Root directory of the project')
    args = parser.parse_args()
    main(args.root)

#!/usr/bin/env python3
import os
import re
import argparse

def extract_docs_from_file(filepath):
    """
    Extract all /**DOC ... */ blocks from the given file and clean up the content.
    Also extracts the code following the comment block.
    Returns a list of markdown strings with embedded code blocks.
    """
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Regex to match /**DOC ... */ including multiline, capturing end position
    pattern = re.compile(r"/\*\*DOC(.*?)\*/", re.DOTALL)
    
    docs = []
    for match in pattern.finditer(content):
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
        
        # Extract code following the comment
        # Skip whitespace after the comment
        remaining = content[end_pos:]
        code_start = len(remaining) - len(remaining.lstrip())
        remaining = remaining.lstrip()
        
        if remaining:
            # Extract until we hit the next /**DOC or end of file
            # or a blank line followed by another comment
            next_doc = pattern.search(remaining)
            if next_doc:
                code_end = next_doc.start()
            else:
                # Extract until we find a reasonable stopping point
                # Look for next top-level construct (function, struct, etc.)
                code_end = len(remaining)
            
            # Try to extract a single logical unit (function, struct, typedef, etc.)
            code_block = extract_code_block(remaining[:code_end])
            
            if code_block:
                # Combine doc and code
                combined = f"{doc_text}\n\n```cpp\n{code_block.rstrip()}\n```"
                docs.append(combined)
            else:
                docs.append(doc_text)
        else:
            docs.append(doc_text)
    
    return docs


def extract_code_block(text):
    """
    Extract a single logical code block (function, struct, typedef, macro, etc.)
    from the beginning of the text.
    """
    text = text.lstrip()
    if not text:
        return ""
    
    lines = text.splitlines()
    if not lines:
        return ""
    
    # Track braces for functions/structs
    brace_count = 0
    in_string = False
    in_char = False
    in_comment = False
    escape_next = False
    code_lines = []
    
    for i, line in enumerate(lines):
        # Check if we've hit another comment block or blank lines suggesting end
        if i > 0 and not code_lines[-1].strip() and line.strip().startswith('/*'):
            break
        
        code_lines.append(line)
        
        # Simple brace counting (doesn't handle all edge cases but good enough)
        for char in line:
            if escape_next:
                escape_next = False
                continue
            
            if char == '\\':
                escape_next = True
                continue
                
            if in_comment:
                if char == '/' and line[max(0, line.index(char)-1)] == '*':
                    in_comment = False
                continue
            
            if in_string:
                if char == '"':
                    in_string = False
                continue
            
            if in_char:
                if char == "'":
                    in_char = False
                continue
            
            if char == '/' and line.find('/*', line.index(char)) == line.index(char):
                in_comment = True
                continue
            
            if char == '"':
                in_string = True
            elif char == "'":
                in_char = True
            elif char == '{':
                brace_count += 1
            elif char == '}':
                brace_count -= 1
                if brace_count == 0 and i > 0:
                    # End of function/struct
                    return '\n'.join(code_lines[:i+1])
        
        # Check for simple declarations/macros (no braces)
        if i == 0 and line.strip().startswith('#define'):
            # Single line macro
            if not line.rstrip().endswith('\\'):
                return line
            # Multi-line macro - continue until no backslash
        elif line.rstrip().endswith('\\'):
            continue  # Multi-line macro continues
        elif brace_count == 0 and ';' in line and i > 0:
            # Simple declaration ended
            return '\n'.join(code_lines[:i+1])
    
    # Return what we collected
    return '\n'.join(code_lines).rstrip()


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

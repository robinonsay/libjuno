#!/usr/bin/env python3
import re
import sys
from pathlib import Path

# Path to your top‐level CMakeLists.txt
cmake_file = Path("CMakeLists.txt")
text = cmake_file.read_text()

# Matches: project(juno VERSION <major>.<minor>.<patch> LANGUAGES C)
pattern = (
    r"(project\s*\(\s*juno\s+VERSION\s+)"
    r"(\d+)\.(\d+)\.(\d+)"
    r"(\s+LANGUAGES\s+C\s*CXX\s*\))"
)

def bump_match(m):
    # Parse existing major/minor/patch
    major = int(m.group(2))
    minor = int(m.group(3))
    patch = int(m.group(4))
    # Always bump minor, reset patch to 0
    new_minor = minor
    new_patch = patch + 1
    # Reconstruct the line prefix + new version + suffix
    return f"{m.group(1)}{major}.{new_minor}.{new_patch}{m.group(5)}"

# Search for the version‐line
m = re.search(pattern, text)
if not m:
    print("⚠️  No matching version line found (or already bumped). Exiting.", file=sys.stderr)
    sys.exit(0)

# Compute the “new version” string (so that we can print it later)
old_major = int(m.group(2))
old_minor = int(m.group(3))
old_patch = int(m.group(4))
new_major = old_major
new_minor = old_minor
new_patch = old_patch + 1
new_version = f"{new_major}.{new_minor}.{new_patch}"

# Apply the substitution
new_text = re.sub(pattern, bump_match, text)
cmake_file.write_text(new_text)

# Print only the new version (e.g. "1.2.0") to stdout
print(new_version)

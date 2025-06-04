import re
from pathlib import Path
import sys

# Path to your top‐level CMakeLists.txt
cmake_file = Path("CMakeLists.txt")
text = cmake_file.read_text()

# Match: project(juno VERSION <major>.<minor>.<patch> LANGUAGES C)
pattern = (
    r"(project\s*\(\s*juno\s+VERSION\s+)"
    r"(\d+)\.(\d+)\.(\d+)"
    r"(\s+LANGUAGES\s+C\s*\))"
)

def bump_match(m):
    major = int(m.group(2))
    minor = int(m.group(3))
    # ignore the old patch; we’ll reset it:
    new_minor = minor + 1
    new_patch = 0
    return f"{m.group(1)}{major}.{new_minor}.{new_patch}{m.group(5)}"

new_text = re.sub(pattern, bump_match, text)
if new_text == text:
    print("⚠️  No matching version line found (or already bumped). Exiting.")
    sys.exit(0)

cmake_file.write_text(new_text)
print("✅  Version line updated.")

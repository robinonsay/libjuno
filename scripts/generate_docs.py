#!/usr/bin/env python3
"""
generate_docs.py — LibJuno Requirements Document Generator

Generates AsciiDoc SRS (IEEE 830), RTM, SDD (IEEE 1016), and validates
traceability consistency from requirements.json files and source/test
annotations.

Supports output in AsciiDoc (.adoc), HTML, and PDF formats.

Usage:
    python generate_docs.py --type srs --format all
    python generate_docs.py --type rtm --format html
    python generate_docs.py --type sdd --format pdf
    python generate_docs.py --type all --module heap
    python generate_docs.py --validate-only
"""

import argparse
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional


# ---------------------------------------------------------------------------
# Project path constants
# ---------------------------------------------------------------------------

SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_DIR = SCRIPT_DIR.parent
REQUIREMENTS_DIR = PROJECT_DIR / "requirements"
SRC_DIR = PROJECT_DIR / "src"
INCLUDE_DIR = PROJECT_DIR / "include"
TESTS_DIR = PROJECT_DIR / "tests"
DEFAULT_OUTPUT_DIR = PROJECT_DIR / "docs" / "generated"
SDD_SOURCE_DIR = PROJECT_DIR / "docs" / "sdd"

# Regex for annotation tags in source and test files
# Matches: // @{"req": ["REQ-XXX-001", ...]}   (C/C++ style)
# Matches: # @{"req": ["REQ-XXX-001", ...]}    (CMake/Python style)
# Matches: // @{"verify": ["REQ-XXX-001", ...]}
TAG_PATTERN = re.compile(
    r'(?://|#)\s*@(\{.*?\})', re.DOTALL
)


# ---------------------------------------------------------------------------
# Data classes
# ---------------------------------------------------------------------------

@dataclass
class Requirement:
    """A single requirement parsed from requirements.json."""
    id: str
    title: str
    description: str
    rationale: str
    verification_method: str
    uses: list = field(default_factory=list)
    implements: list = field(default_factory=list)
    module: str = ""


@dataclass
class CodeTrace:
    """A code annotation linking a function to requirement IDs."""
    file: str
    line: int
    function: str
    req_ids: list = field(default_factory=list)


@dataclass
class TestTrace:
    """A test annotation linking a test function to requirement IDs."""
    file: str
    line: int
    function: str
    req_ids: list = field(default_factory=list)


@dataclass
class DesignTrace:
    """A design annotation linking an SDD section to requirement IDs."""
    file: str
    line: int
    section: str
    req_ids: list = field(default_factory=list)


@dataclass
class ValidationMessage:
    """A validation finding."""
    level: str  # ERROR, WARN, INFO
    message: str

    def __str__(self):
        return f"[{self.level}] {self.message}"


# ---------------------------------------------------------------------------
# Parsing: requirements.json
# ---------------------------------------------------------------------------

def load_requirements(requirements_dir: Path,
                      module_filter: Optional[str] = None
                      ) -> dict[str, list[Requirement]]:
    """Load all requirements.json files from the requirements directory.

    Returns a dict of module_name -> list[Requirement].
    """
    modules = {}
    if not requirements_dir.is_dir():
        return modules

    for module_dir in sorted(requirements_dir.iterdir()):
        if not module_dir.is_dir():
            continue
        if module_filter and module_dir.name.lower() != module_filter.lower():
            continue
        req_file = module_dir / "requirements.json"
        if not req_file.is_file():
            continue
        with open(req_file, "r", encoding="utf-8") as f:
            data = json.load(f)
        module_name = data.get("module", module_dir.name.upper())
        reqs = []
        for r in data.get("requirements", []):
            reqs.append(Requirement(
                id=r["id"],
                title=r.get("title", ""),
                description=r.get("description", ""),
                rationale=r.get("rationale", ""),
                verification_method=r.get("verification_method", ""),
                uses=r.get("uses", []),
                implements=r.get("implements", []),
                module=module_name,
            ))
        modules[module_name] = reqs
    return modules


# ---------------------------------------------------------------------------
# Parsing: source and test annotations
# ---------------------------------------------------------------------------

def _find_section_name_before(lines: list[str], tag_line_idx: int) -> str:
    """Find the nearest AsciiDoc section heading near the tag line.

    First checks lines immediately after the tag (the common case where the
    annotation precedes the section it covers), then falls back to searching
    backwards.
    """
    # Look forward first (up to 3 lines) for a heading right after the tag
    for offset in range(1, min(4, len(lines) - tag_line_idx)):
        line = lines[tag_line_idx + offset].strip()
        m = re.match(r'^=+\s+(.+)$', line)
        if m:
            return m.group(1).strip()
    # Fall back to searching backwards
    for offset in range(tag_line_idx, -1, -1):
        line = lines[offset].strip()
        m = re.match(r'^=+\s+(.+)$', line)
        if m:
            return m.group(1).strip()
    return "<unknown section>"


def _find_function_name_after(lines: list[str], tag_line_idx: int) -> str:
    """Heuristically find the C function name after a tag line.

    Looks for the first line that looks like a function definition
    (identifier followed by '(') within a few lines after the tag.
    """
    # Pattern: return_type FunctionName(  or  void FunctionName(
    func_pattern = re.compile(
        r'(?:^|[\s\*])([A-Za-z_]\w*)\s*\(', re.MULTILINE
    )
    # Search the next few lines for a function definition
    for offset in range(0, min(5, len(lines) - tag_line_idx)):
        line = lines[tag_line_idx + offset]
        # Skip comment lines, blank lines, and preprocessor directives
        stripped = line.strip()
        if not stripped or stripped.startswith("//") or stripped.startswith("#"):
            continue
        m = func_pattern.search(line)
        if m:
            name = m.group(1)
            # Filter out common C keywords that aren't function names
            keywords = {
                "if", "else", "while", "for", "switch", "return",
                "sizeof", "typeof", "static", "const", "void",
                "struct", "union", "enum", "typedef", "extern",
                "inline", "volatile", "register", "unsigned",
                "signed", "int", "char", "long", "short", "float",
                "double", "bool", "uint8_t", "uint16_t", "uint32_t",
                "uint64_t", "int8_t", "int16_t", "int32_t", "int64_t",
                "size_t", "true", "false",
            }
            if name not in keywords:
                return name
    return "<unknown>"


def _scan_file(fpath: Path, tag_key: str) -> list:
    """Scan a single file for annotation tags and return trace entries."""
    traces = []
    with open(fpath, "r", encoding="utf-8") as f:
        content = f.read()
    lines = content.splitlines()
    for i, line in enumerate(lines):
        m = TAG_PATTERN.search(line)
        if not m:
            continue
        try:
            tag_data = json.loads(m.group(1))
        except json.JSONDecodeError:
            continue
        ids = tag_data.get(tag_key, [])
        if not ids:
            continue
        try:
            rel_path = str(fpath.relative_to(PROJECT_DIR))
        except ValueError:
            rel_path = str(fpath)
        if tag_key == "design":
            section = _find_section_name_before(lines, i)
            traces.append(DesignTrace(
                file=rel_path,
                line=i + 1,
                section=section,
                req_ids=ids,
            ))
        else:
            func_name = _find_function_name_after(lines, i)
            trace_cls = TestTrace if tag_key == "verify" else CodeTrace
            traces.append(trace_cls(
                file=rel_path,
                line=i + 1,
                function=func_name,
                req_ids=ids,
            ))
    return traces


def scan_annotations(directories: list[Path],
                     extensions: tuple[str, ...] = (".c", ".h", ".cpp", ".hpp"),
                     tag_key: str = "req",
                     extra_files: Optional[list[Path]] = None,
                     filenames: Optional[tuple[str, ...]] = None,
                     ) -> list:
    """Scan source files for @{"req": [...]} or @{"verify": [...]} annotations.

    Parameters
    ----------
    directories : list[Path]
        Directories to recursively scan.
    extensions : tuple[str, ...]
        File extensions to include.
    tag_key : str
        The JSON key to look for ("req" or "verify").
    extra_files : list[Path] | None
        Additional individual files to scan (any extension).
    filenames : tuple[str, ...] | None
        Additional exact filenames to match (e.g. ``("CMakeLists.txt",)``).

    Returns
    -------
    list of CodeTrace or TestTrace
    """
    traces = []
    for directory in directories:
        if not directory.is_dir():
            continue
        for root, _dirs, files in os.walk(directory):
            for fname in sorted(files):
                matches_ext = fname.endswith(extensions)
                matches_name = filenames and fname in filenames
                if not (matches_ext or matches_name):
                    continue
                fpath = Path(root) / fname
                traces.extend(_scan_file(fpath, tag_key))
    # Scan individual extra files
    for fpath in (extra_files or []):
        if fpath.is_file():
            traces.extend(_scan_file(fpath, tag_key))
    return traces


def scan_code_traces(module_filter: Optional[str] = None) -> list[CodeTrace]:
    """Scan src/, include/, and cmake/ for @{"req": [...]} annotations.

    Also scans the top-level CMakeLists.txt.
    """
    cmake_dir = PROJECT_DIR / "cmake"
    traces = scan_annotations(
        [SRC_DIR, INCLUDE_DIR, cmake_dir],
        extensions=(".c", ".h"),
        tag_key="req",
        extra_files=[PROJECT_DIR / "CMakeLists.txt"],
        filenames=("CMakeLists.txt",),
    )
    if module_filter:
        mod = module_filter.upper()
        traces = [
            t for t in traces
            if any(mod in rid for rid in t.req_ids)
        ]
    return traces


def scan_test_traces(module_filter: Optional[str] = None) -> list[TestTrace]:
    """Scan tests/ for @{"verify": [...]} annotations."""
    traces = scan_annotations(
        [TESTS_DIR],
        extensions=(".c", ".cpp"),
        tag_key="verify",
    )
    if module_filter:
        mod = module_filter.upper()
        traces = [
            t for t in traces
            if any(mod in rid for rid in t.req_ids)
        ]
    return traces


def scan_design_traces(module_filter: Optional[str] = None
                      ) -> list[DesignTrace]:
    """Scan docs/sdd/ for @{"design": [...]} annotations."""
    traces = scan_annotations(
        [SDD_SOURCE_DIR],
        extensions=(".adoc",),
        tag_key="design",
    )
    if module_filter:
        mod = module_filter.upper()
        traces = [
            t for t in traces
            if any(mod in rid for rid in t.req_ids)
        ]
    return traces


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------

def validate_traceability(
    modules: dict[str, list[Requirement]],
    code_traces: list[CodeTrace],
    test_traces: list[TestTrace],
    design_traces: Optional[list[DesignTrace]] = None,
) -> list[ValidationMessage]:
    """Validate traceability consistency.

    Checks:
    - Every requirement has at least one code annotation
    - Every requirement with verification_method=Test has a test annotation
    - Every requirement has at least one design annotation
    - No orphaned code/test/design tags referencing nonexistent requirements
    - All uses/implements links resolve to valid requirement IDs
    - No duplicate requirement IDs
    """
    if design_traces is None:
        design_traces = []
    messages = []

    # Build lookup of all known requirement IDs
    all_req_ids = set()
    id_to_req = {}
    for reqs in modules.values():
        for req in reqs:
            if req.id in all_req_ids:
                messages.append(ValidationMessage(
                    "ERROR",
                    f"Duplicate requirement ID: {req.id}"
                ))
            all_req_ids.add(req.id)
            id_to_req[req.id] = req

    # Build lookup of traced IDs
    code_traced_ids = set()
    for trace in code_traces:
        for rid in trace.req_ids:
            code_traced_ids.add(rid)

    test_traced_ids = set()
    for trace in test_traces:
        for rid in trace.req_ids:
            test_traced_ids.add(rid)

    # Check: every requirement has code annotation
    for rid in sorted(all_req_ids):
        if rid not in code_traced_ids:
            messages.append(ValidationMessage(
                "WARN",
                f"{rid} has no code annotation (@{{\"req\": [...]}})"
            ))

    # Check: test-verified requirements have test annotations
    for rid in sorted(all_req_ids):
        req = id_to_req[rid]
        if req.verification_method == "Test" and rid not in test_traced_ids:
            messages.append(ValidationMessage(
                "WARN",
                f"{rid} (verification_method=Test) has no test annotation "
                f"(@{{\"verify\": [...]}})"
            ))

    # Check: orphaned code tags
    for trace in code_traces:
        for rid in trace.req_ids:
            if rid not in all_req_ids:
                messages.append(ValidationMessage(
                    "ERROR",
                    f"Orphaned code tag @{{\"req\": [\"{rid}\"]}} "
                    f"in {trace.file}:{trace.line} — requirement does not exist"
                ))

    # Check: orphaned test tags
    for trace in test_traces:
        for rid in trace.req_ids:
            if rid not in all_req_ids:
                messages.append(ValidationMessage(
                    "ERROR",
                    f"Orphaned test tag @{{\"verify\": [\"{rid}\"]}} "
                    f"in {trace.file}:{trace.line} — requirement does not exist"
                ))

    # Build lookup of design-traced IDs
    design_traced_ids = set()
    for trace in design_traces:
        for rid in trace.req_ids:
            design_traced_ids.add(rid)

    # Check: every requirement has design annotation
    for rid in sorted(all_req_ids):
        if rid not in design_traced_ids:
            messages.append(ValidationMessage(
                "WARN",
                f"{rid} has no design annotation "
                f"(@{{\"design\": [...]}})"
            ))

    # Check: orphaned design tags
    for trace in design_traces:
        for rid in trace.req_ids:
            if rid not in all_req_ids:
                messages.append(ValidationMessage(
                    "ERROR",
                    f"Orphaned design tag @{{\"design\": [\"{rid}\"]}} "
                    f"in {trace.file}:{trace.line} — requirement does not exist"
                ))

    # Check: uses/implements links resolve
    for reqs in modules.values():
        for req in reqs:
            for parent_id in req.uses:
                if parent_id not in all_req_ids:
                    messages.append(ValidationMessage(
                        "ERROR",
                        f"{req.id} uses {parent_id} — "
                        f"target requirement does not exist"
                    ))
            for child_id in req.implements:
                if child_id not in all_req_ids:
                    messages.append(ValidationMessage(
                        "ERROR",
                        f"{req.id} implements {child_id} — "
                        f"target requirement does not exist"
                    ))

    return messages


def print_validation_report(
    messages: list[ValidationMessage],
    modules: dict[str, list[Requirement]],
    code_traces: list[CodeTrace],
    test_traces: list[TestTrace],
    design_traces: Optional[list[DesignTrace]] = None,
):
    """Print a human-readable validation report to stderr."""
    if design_traces is None:
        design_traces = []
    total_reqs = sum(len(reqs) for reqs in modules.values())
    code_traced_ids = set()
    for t in code_traces:
        code_traced_ids.update(t.req_ids)
    test_traced_ids = set()
    for t in test_traces:
        test_traced_ids.update(t.req_ids)
    design_traced_ids = set()
    for t in design_traces:
        design_traced_ids.update(t.req_ids)

    all_req_ids = set()
    for reqs in modules.values():
        for req in reqs:
            all_req_ids.add(req.id)

    traced_count = len(all_req_ids & code_traced_ids)
    tested_count = len(all_req_ids & test_traced_ids)
    designed_count = len(all_req_ids & design_traced_ids)

    errors = [m for m in messages if m.level == "ERROR"]
    warnings = [m for m in messages if m.level == "WARN"]

    print("\n=== Traceability Validation Report ===", file=sys.stderr)
    print(f"  Total requirements: {total_reqs}", file=sys.stderr)
    if total_reqs > 0:
        print(
            f"  Traced to code:   {traced_count}/{total_reqs} "
            f"({100*traced_count//total_reqs}%)",
            file=sys.stderr,
        )
        print(
            f"  Traced to tests:  {tested_count}/{total_reqs} "
            f"({100*tested_count//total_reqs}%)",
            file=sys.stderr,
        )
        print(
            f"  Traced to design: {designed_count}/{total_reqs} "
            f"({100*designed_count//total_reqs}%)",
            file=sys.stderr,
        )
    print(f"  Errors:   {len(errors)}", file=sys.stderr)
    print(f"  Warnings: {len(warnings)}", file=sys.stderr)

    if errors:
        print("\n--- Errors ---", file=sys.stderr)
        for m in errors:
            print(f"  {m}", file=sys.stderr)
    if warnings:
        print("\n--- Warnings ---", file=sys.stderr)
        for m in warnings:
            print(f"  {m}", file=sys.stderr)

    if not errors and not warnings:
        print("  ✓ All traceability checks passed.", file=sys.stderr)
    print("", file=sys.stderr)


# ---------------------------------------------------------------------------
# AsciiDoc generation: SRS (IEEE 830)
# ---------------------------------------------------------------------------

def generate_srs_adoc(
    modules: dict[str, list[Requirement]],
    code_traces: list[CodeTrace],
    test_traces: list[TestTrace],
    messages: list[ValidationMessage],
) -> str:
    """Generate an IEEE 830-style SRS in AsciiDoc format."""

    # Build trace lookups: req_id -> list of locations
    code_map: dict[str, list[str]] = {}
    for t in code_traces:
        for rid in t.req_ids:
            code_map.setdefault(rid, []).append(
                f"`{t.file}:{t.line}` ({t.function})"
            )
    test_map: dict[str, list[str]] = {}
    for t in test_traces:
        for rid in t.req_ids:
            test_map.setdefault(rid, []).append(
                f"`{t.file}:{t.line}` ({t.function})"
            )

    lines = []
    w = lines.append

    w("= LibJuno Software Requirements Specification (SRS)")
    w(":toc: left")
    w(":toclevels: 3")
    w(":sectnums:")
    w(":icons: font")
    w("")

    # --- Section 1: Introduction ---
    w("== Introduction")
    w("")
    w("=== Purpose")
    w("This Software Requirements Specification (SRS) defines the functional "
      "and non-functional requirements for LibJuno, a freestanding C11 embedded "
      "systems micro-framework.")
    w("")
    w("This document follows the IEEE 830-1998 standard structure.")
    w("")
    w("=== Scope")
    w("LibJuno provides data structures, memory management primitives, CRC "
      "algorithms, and other utilities for embedded systems development. "
      "It requires zero dynamic memory allocation and supports freestanding "
      "C11 environments.")
    w("")
    w("=== Definitions and Acronyms")
    w("")
    w("[cols=\"1,3\"]")
    w("|===")
    w("| Term | Definition")
    w("")
    w("| Freestanding | A C implementation that does not require a hosted "
      "standard library")
    w("| DI | Dependency Injection — a pattern where dependencies are provided "
      "to a component rather than created internally")
    w("| Vtable | A struct of function pointers enabling polymorphic dispatch")
    w("| SRS | Software Requirements Specification")
    w("| RTM | Requirements Traceability Matrix")
    w("|===")
    w("")
    w("=== References")
    w("")
    w("* IEEE 830-1998: Recommended Practice for Software Requirements "
      "Specifications")
    w("* LibJuno README and documentation")
    w("")

    # --- Section 2: Overall Description ---
    w("== Overall Description")
    w("")
    w("=== Product Perspective")
    w("LibJuno is a standalone C11 library designed to be used as a dependency "
      "in embedded systems projects. It makes no assumptions about the target "
      "platform and uses dependency injection to remain adaptable.")
    w("")
    w("=== Design Constraints")
    w("")
    w("* No dynamic memory allocation (`malloc`, `calloc`, `realloc`, `free`)")
    w("* All memory is caller-owned and injected")
    w("* Must compile as freestanding C11 (`-nostdlib -ffreestanding`)")
    w("* All warnings treated as errors (`-Werror`)")
    w("")
    w("=== Assumptions and Dependencies")
    w("")
    w("* A C11-compliant compiler is available")
    w("* CMake ≥ 3.10 is available for building")
    w("* Unity test framework is available for testing (build-time only)")
    w("")

    # --- Section 3: Specific Requirements ---
    w("== Specific Requirements")
    w("")

    if not modules:
        w("_No requirements found. Run `generate_docs.py` after creating "
          "`requirements/<module>/requirements.json` files._")
        w("")
    else:
        for module_name in sorted(modules.keys()):
            reqs = modules[module_name]
            w(f"=== Module: {module_name}")
            w("")
            for req in reqs:
                w(f"==== {req.id}: {req.title}")
                w("")
                w(f"**Description**:: {req.description}")
                w("")
                w(f"**Rationale**:: {req.rationale}")
                w("")
                w(f"**Verification Method**:: {req.verification_method}")
                w("")
                if req.uses:
                    w(f"**Uses (parent)**:: {', '.join(req.uses)}")
                    w("")
                if req.implements:
                    w(f"**Implements (children)**:: "
                      f"{', '.join(req.implements)}")
                    w("")

                # Code traces
                code_locs = code_map.get(req.id, [])
                if code_locs:
                    w("**Code Traces**::")
                    for loc in code_locs:
                        w(f"  * {loc}")
                    w("")

                # Test traces
                test_locs = test_map.get(req.id, [])
                if test_locs:
                    w("**Test Traces**::")
                    for loc in test_locs:
                        w(f"  * {loc}")
                    w("")

    # --- Section 4: Validation Summary ---
    w("== Traceability Validation Summary")
    w("")
    errors = [m for m in messages if m.level == "ERROR"]
    warnings = [m for m in messages if m.level == "WARN"]
    if not errors and not warnings:
        w("✓ All traceability checks passed.")
    else:
        if errors:
            w("=== Errors")
            w("")
            for m in errors:
                w(f"* {m}")
            w("")
        if warnings:
            w("=== Warnings")
            w("")
            for m in warnings:
                w(f"* {m}")
            w("")

    return "\n".join(lines)


# ---------------------------------------------------------------------------
# AsciiDoc generation: RTM
# ---------------------------------------------------------------------------

def generate_rtm_adoc(
    modules: dict[str, list[Requirement]],
    code_traces: list[CodeTrace],
    test_traces: list[TestTrace],
    messages: list[ValidationMessage],
    design_traces: Optional[list[DesignTrace]] = None,
) -> str:
    """Generate a Requirements Traceability Matrix in AsciiDoc format."""
    if design_traces is None:
        design_traces = []

    # Build trace lookups
    code_map: dict[str, list[str]] = {}
    for t in code_traces:
        for rid in t.req_ids:
            code_map.setdefault(rid, []).append(
                f"{t.file}:{t.line} ({t.function})"
            )
    test_map: dict[str, list[str]] = {}
    for t in test_traces:
        for rid in t.req_ids:
            test_map.setdefault(rid, []).append(
                f"{t.file}:{t.line} ({t.function})"
            )
    design_map: dict[str, list[str]] = {}
    for t in design_traces:
        for rid in t.req_ids:
            design_map.setdefault(rid, []).append(
                f"{t.file}:{t.line} ({t.section})"
            )

    lines = []
    w = lines.append

    w("= LibJuno Requirements Traceability Matrix (RTM)")
    w(":toc: left")
    w(":toclevels: 2")
    w(":sectnums:")
    w(":icons: font")
    w("")

    # --- Coverage Summary ---
    w("== Coverage Summary")
    w("")

    all_req_ids = set()
    test_required_ids = set()
    for reqs in modules.values():
        for req in reqs:
            all_req_ids.add(req.id)
            if req.verification_method == "Test":
                test_required_ids.add(req.id)

    code_traced_ids = set()
    for t in code_traces:
        code_traced_ids.update(t.req_ids)
    test_traced_ids = set()
    for t in test_traces:
        test_traced_ids.update(t.req_ids)
    design_traced_ids = set()
    for t in design_traces:
        design_traced_ids.update(t.req_ids)

    total = len(all_req_ids)
    traced = len(all_req_ids & code_traced_ids)
    tested = len(test_required_ids & test_traced_ids)
    test_total = len(test_required_ids)
    designed = len(all_req_ids & design_traced_ids)

    w("[cols=\"2,1,1\"]")
    w("|===")
    w("| Metric | Count | Percentage")
    w("")
    w(f"| Total requirements | {total} | —")
    if total > 0:
        w(f"| Traced to code | {traced} | {100*traced//total}%")
        w(f"| Traced to design | {designed} | {100*designed//total}%")
    else:
        w("| Traced to code | 0 | —")
        w("| Traced to design | 0 | —")
    if test_total > 0:
        w(f"| Traced to tests (of test-verified) | {tested} "
          f"| {100*tested//test_total}%")
    else:
        w("| Traced to tests (of test-verified) | 0 | —")
    w("|===")
    w("")

    # --- Per-module summary ---
    w("== Per-Module Summary")
    w("")
    w("[cols=\"1,1,1,1,1\"]")
    w("|===")
    w("| Module | Requirements | Traced to Code | Traced to Design "
      "| Traced to Tests")
    w("")
    for module_name in sorted(modules.keys()):
        reqs = modules[module_name]
        mod_ids = {r.id for r in reqs}
        mod_code = len(mod_ids & code_traced_ids)
        mod_design = len(mod_ids & design_traced_ids)
        mod_test = len(mod_ids & test_traced_ids)
        w(f"| {module_name} | {len(reqs)} | {mod_code} "
          f"| {mod_design} | {mod_test}")
    w("|===")
    w("")

    # --- Full Traceability Matrix ---
    w("== Traceability Matrix")
    w("")

    for module_name in sorted(modules.keys()):
        reqs = modules[module_name]
        w(f"=== Module: {module_name}")
        w("")
        w("[cols=\"1,2,2,2,1,1\", options=\"header\"]")
        w("|===")
        w("| Requirement | Code Location | Design Location "
          "| Test Location | Verification Method | Status")
        w("")

        for req in reqs:
            code_locs = code_map.get(req.id, [])
            test_locs = test_map.get(req.id, [])
            design_locs = design_map.get(req.id, [])

            code_str = " +\n".join(code_locs) if code_locs else "_none_"
            test_str = " +\n".join(test_locs) if test_locs else "_none_"
            design_str = " +\n".join(design_locs) if design_locs else "_none_"

            # Determine status
            has_code = bool(code_locs)
            has_test = bool(test_locs)
            has_design = bool(design_locs)
            needs_test = req.verification_method == "Test"

            if has_code and has_design and (has_test or not needs_test):
                status = "✓ Complete"
            elif has_code and not has_design:
                status = "⚠ Missing design"
            elif has_code and needs_test and not has_test:
                status = "⚠ Missing test"
            elif not has_code:
                status = "✗ Untraced"
            else:
                status = "? Unknown"

            w(f"| {req.id}: {req.title}")
            w(f"| {code_str}")
            w(f"| {design_str}")
            w(f"| {test_str}")
            w(f"| {req.verification_method}")
            w(f"| {status}")
            w("")

        w("|===")
        w("")

    # --- Gap Report ---
    errors = [m for m in messages if m.level == "ERROR"]
    warnings = [m for m in messages if m.level == "WARN"]
    if errors or warnings:
        w("== Gap Report")
        w("")
        if errors:
            w("=== Errors")
            w("")
            for m in errors:
                w(f"* {m}")
            w("")
        if warnings:
            w("=== Warnings")
            w("")
            for m in warnings:
                w(f"* {m}")
            w("")

    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Output: write AsciiDoc and convert
# ---------------------------------------------------------------------------

def generate_sdd(sdd_source_dir: Path, formats: list[str]):
    """Convert the hand-authored SDD AsciiDoc into HTML and/or PDF.

    The SDD source lives at docs/sdd/sdd.adoc and uses include::
    directives for per-module subsections.  Unlike the SRS and RTM,
    the SDD content is maintained directly in AsciiDoc rather than
    generated from requirements.json, so this function only performs
    format conversion.

    Parameters
    ----------
    sdd_source_dir : Path
        Directory containing sdd.adoc and modules/ subdirectory.
    formats : list[str]
        Target formats: any of "adoc", "html", "pdf".
    """
    adoc_path = sdd_source_dir / "sdd.adoc"
    if not adoc_path.is_file():
        print(
            f"  ERROR: SDD source not found at {adoc_path}",
            file=sys.stderr,
        )
        return

    print(f"  SDD source: {adoc_path}", file=sys.stderr)

    if "html" in formats:
        convert_adoc(adoc_path, "html")
    if "pdf" in formats:
        convert_adoc(adoc_path, "pdf")

    # "adoc" is a no-op for the SDD since the source already exists
    if "adoc" in formats:
        print(f"  AsciiDoc source already at: {adoc_path}", file=sys.stderr)


def write_adoc(content: str, output_path: Path):
    """Write AsciiDoc content to a file."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"  Written: {output_path}", file=sys.stderr)


def convert_adoc(adoc_path: Path, fmt: str):
    """Convert an AsciiDoc file to HTML or PDF via asciidoctor.

    Parameters
    ----------
    adoc_path : Path
        The .adoc source file.
    fmt : str
        "html" or "pdf".
    """
    if fmt == "html":
        cmd = ["asciidoctor", str(adoc_path)]
    elif fmt == "pdf":
        cmd = ["asciidoctor-pdf", str(adoc_path)]
    else:
        print(f"  Unknown format: {fmt}", file=sys.stderr)
        return

    try:
        subprocess.run(cmd, check=True, capture_output=True, text=True)
        ext = ".html" if fmt == "html" else ".pdf"
        out = adoc_path.with_suffix(ext)
        print(f"  Converted: {out}", file=sys.stderr)
    except FileNotFoundError:
        print(
            f"  WARNING: '{cmd[0]}' not found. Install it to generate "
            f"{fmt.upper()} output.",
            file=sys.stderr,
        )
        print(
            f"    gem install asciidoctor"
            + (" asciidoctor-pdf" if fmt == "pdf" else ""),
            file=sys.stderr,
        )
    except subprocess.CalledProcessError as e:
        print(
            f"  ERROR running {cmd[0]}: {e.stderr}",
            file=sys.stderr,
        )


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="LibJuno Requirements Document Generator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --type srs --format adoc
  %(prog)s --type rtm --format all
  %(prog)s --type sdd --format pdf
  %(prog)s --type all --format html --module heap
  %(prog)s --validate-only
        """,
    )
    parser.add_argument(
        "--type",
        choices=["srs", "rtm", "sdd", "all"],
        default="all",
        help="Document type to generate (default: all)",
    )
    parser.add_argument(
        "--format",
        choices=["adoc", "html", "pdf", "all"],
        default="all",
        help="Output format (default: all)",
    )
    parser.add_argument(
        "--module",
        default=None,
        help="Filter to a specific module (e.g., heap, crc)",
    )
    parser.add_argument(
        "--output-dir",
        default=None,
        help="Output directory (default: <project>/docs/generated)",
    )
    parser.add_argument(
        "--validate-only",
        action="store_true",
        help="Only run validation, do not generate documents",
    )
    parser.add_argument(
        "--project-dir",
        default=None,
        help="Override project root directory",
    )

    args = parser.parse_args()

    # Resolve project paths (allow --project-dir override)
    project_dir = (
        Path(args.project_dir).resolve() if args.project_dir else PROJECT_DIR
    )
    requirements_dir = project_dir / "requirements"
    src_dir = project_dir / "src"
    include_dir = project_dir / "include"
    tests_dir = project_dir / "tests"
    default_output_dir = project_dir / "docs" / "generated"
    output_dir = Path(args.output_dir) if args.output_dir else default_output_dir

    # --- Load data ---
    print("Loading requirements...", file=sys.stderr)
    modules = load_requirements(requirements_dir, args.module)
    if not modules:
        print(
            "  No requirements found. Ensure requirements/<module>/"
            "requirements.json files exist.",
            file=sys.stderr,
        )

    print("Scanning code annotations...", file=sys.stderr)
    cmake_dir = project_dir / "cmake"
    code_traces = scan_annotations(
        [src_dir, include_dir, cmake_dir],
        extensions=(".c", ".h"),
        tag_key="req",
        extra_files=[project_dir / "CMakeLists.txt"],
        filenames=("CMakeLists.txt",),
    )
    if args.module:
        mod = args.module.upper()
        code_traces = [
            t for t in code_traces
            if any(mod in rid for rid in t.req_ids)
        ]
    print(f"  Found {len(code_traces)} code trace(s).", file=sys.stderr)

    print("Scanning test annotations...", file=sys.stderr)
    test_traces = scan_annotations(
        [tests_dir],
        extensions=(".c", ".cpp"),
        tag_key="verify",
    )
    if args.module:
        mod = args.module.upper()
        test_traces = [
            t for t in test_traces
            if any(mod in rid for rid in t.req_ids)
        ]
    print(f"  Found {len(test_traces)} test trace(s).", file=sys.stderr)

    print("Scanning design annotations...", file=sys.stderr)
    design_traces = scan_annotations(
        [project_dir / "docs" / "sdd"],
        extensions=(".adoc",),
        tag_key="design",
    )
    if args.module:
        mod = args.module.upper()
        design_traces = [
            t for t in design_traces
            if any(mod in rid for rid in t.req_ids)
        ]
    print(f"  Found {len(design_traces)} design trace(s).", file=sys.stderr)

    # --- Validate ---
    print("Validating traceability...", file=sys.stderr)
    messages = validate_traceability(
        modules, code_traces, test_traces, design_traces
    )
    print_validation_report(
        messages, modules, code_traces, test_traces, design_traces
    )

    if args.validate_only:
        errors = [m for m in messages if m.level == "ERROR"]
        sys.exit(1 if errors else 0)

    # --- Determine formats ---
    formats = []
    if args.format == "all":
        formats = ["adoc", "html", "pdf"]
    else:
        formats = [args.format]

    # --- Generate documents ---
    doc_types = []
    if args.type == "all":
        doc_types = ["srs", "rtm", "sdd"]
    else:
        doc_types = [args.type]

    for doc_type in doc_types:
        print(f"\nGenerating {doc_type.upper()}...", file=sys.stderr)
        if doc_type == "sdd":
            # SDD is hand-authored AsciiDoc; only convert formats
            sdd_dir = project_dir / "docs" / "sdd"
            generate_sdd(sdd_dir, formats)
            continue
        elif doc_type == "srs":
            content = generate_srs_adoc(
                modules, code_traces, test_traces, messages
            )
            adoc_path = output_dir / "srs" / "srs.adoc"
        elif doc_type == "rtm":
            content = generate_rtm_adoc(
                modules, code_traces, test_traces, messages,
                design_traces
            )
            adoc_path = output_dir / "rtm" / "rtm.adoc"
        else:
            continue

        # Always write AsciiDoc source
        write_adoc(content, adoc_path)

        # Convert to other formats if requested
        if "html" in formats:
            convert_adoc(adoc_path, "html")
        if "pdf" in formats:
            convert_adoc(adoc_path, "pdf")

    print("\nDone.", file=sys.stderr)


if __name__ == "__main__":
    main()

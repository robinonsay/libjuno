#!/usr/bin/env python3
"""
verify_traceability.py -- Traceability coverage verification for LibJuno.

Loads all requirements/<module>/requirements.json files, scans source and
test files for annotation tags, and cross-references to report coverage
errors and warnings.

Exit code: 0 = no errors found, 1 = one or more errors found.

Usage:
    python scripts/verify_traceability.py
    python scripts/verify_traceability.py --module HEAP
    python scripts/verify_traceability.py --verbose
    python scripts/verify_traceability.py --root /path/to/project --module CRC --verbose
"""

import argparse
import json
import re
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

VALID_VERIFICATION_METHODS = frozenset({"Test", "Inspection", "Analysis", "Demonstration"})

# Requirement ID must match REQ-MODULENAME-NNN (e.g. REQ-HEAP-001)
REQ_ID_RE = re.compile(r"^REQ-[A-Z]+-[0-9]{3}$")

# Matches a single-line C/TS comment annotation: // @{...}
# The JSON payload is captured in group 1.
ANNOTATION_RE = re.compile(r"//\s*@(\{.+\})\s*$")

# Matches a single-line CMake/Python comment annotation: # @{...}
CMAKE_ANNOTATION_RE = re.compile(r"#\s*@(\{.+\})\s*$")


# ---------------------------------------------------------------------------
# Project root detection
# ---------------------------------------------------------------------------

def find_root(start):
    """Walk upward from *start* looking for a directory that contains both
    ``requirements/`` and ``src/`` sub-directories.  Raises FileNotFoundError
    if no such directory is found within 12 levels."""
    candidate = Path(start).resolve()
    for _ in range(12):
        if (candidate / "requirements").is_dir() and (candidate / "src").is_dir():
            return candidate
        parent = candidate.parent
        if parent == candidate:
            break
        candidate = parent
    raise FileNotFoundError(
        "Could not auto-detect project root from '{}'. "
        "Use --root PATH to specify it explicitly.".format(start)
    )


# ---------------------------------------------------------------------------
# Requirements loading and validation
# ---------------------------------------------------------------------------

def load_requirements(root):
    """Load and validate every ``requirements/<module>/requirements.json`` file.

    Returns:
        all_reqs  -- dict mapping req_id -> requirement dict (with ``_source`` key)
        errors    -- list of ``[ERROR]`` strings
        warnings  -- list of ``[WARNING]`` strings
    """
    all_reqs = {}
    errors = []
    warnings = []
    req_root = root / "requirements"

    if not req_root.is_dir():
        errors.append("[ERROR] requirements/ directory not found under {}".format(root))
        return all_reqs, errors, warnings

    for module_dir in sorted(p for p in req_root.iterdir() if p.is_dir()):
        req_file = module_dir / "requirements.json"
        if not req_file.exists():
            continue
        _load_module_file(req_file, root, all_reqs, errors)

    return all_reqs, errors, warnings


def _load_module_file(req_file, root, all_reqs, errors):
    rel = req_file.relative_to(root)
    try:
        with open(req_file, encoding="utf-8") as fh:
            data = json.load(fh)
    except (json.JSONDecodeError, OSError) as exc:
        errors.append("[ERROR] {}: could not read/parse: {}".format(rel, exc))
        return

    if not isinstance(data, dict):
        errors.append("[ERROR] {}: root element must be a JSON object".format(rel))
        return

    reqs_list = data.get("requirements")
    if not isinstance(reqs_list, list):
        errors.append("[ERROR] {}: missing or invalid 'requirements' array".format(rel))
        return

    for idx, req in enumerate(reqs_list):
        if not isinstance(req, dict):
            errors.append("[ERROR] {}[{}]: entry is not a JSON object".format(rel, idx))
            continue
        _validate_req(req, idx, rel, all_reqs, errors)


def _validate_req(req, idx, rel, all_reqs, errors):
    loc = "{}[{}]".format(rel, idx)

    # --- Required fields ---
    for field in ("id", "title", "description", "rationale", "verification_method"):
        if field not in req:
            errors.append("[ERROR] {}: missing required field '{}'".format(loc, field))

    req_id = req.get("id")
    if req_id is None:
        return  # Cannot continue without an id

    if not isinstance(req_id, str):
        errors.append("[ERROR] {}: 'id' must be a string, got {}".format(loc, type(req_id).__name__))
        return

    # --- ID format ---
    if not REQ_ID_RE.match(req_id):
        errors.append(
            "[ERROR] {}: id '{}' does not match pattern REQ-[A-Z]+-[0-9]{{3}}".format(loc, req_id)
        )
        return

    # --- Duplicate check ---
    if req_id in all_reqs:
        errors.append(
            "[ERROR] {}: duplicate id '{}' (first seen in {})".format(
                loc, req_id, all_reqs[req_id].get("_source", "?")
            )
        )
        return

    # --- verification_method enum ---
    method = req.get("verification_method")
    if method is not None and method not in VALID_VERIFICATION_METHODS:
        errors.append(
            "[ERROR] {}: invalid verification_method '{}'. "
            "Valid values: {}".format(
                loc, method, ", ".join(sorted(VALID_VERIFICATION_METHODS))
            )
        )

    # Store a copy with provenance metadata
    entry = dict(req)
    entry["_source"] = str(rel)
    all_reqs[req_id] = entry


# ---------------------------------------------------------------------------
# Fix missing implements arrays
# ---------------------------------------------------------------------------

def fix_implements(root, all_reqs):
    """Auto-populate missing ``implements`` arrays by inverting ``uses`` links.

    For every requirement that declares ``uses: [PARENT-ID, ...]``, adds that
    requirement's ID to the parent's ``implements`` list if it is not already
    present.  Each modified ``requirements.json`` file is written back to disk
    with two-space JSON indentation.

    Args:
        root      -- absolute Path to the project root.
        all_reqs  -- dict mapping req_id -> requirement dict (with ``_source``).

    Returns:
        files_updated  -- number of files written.
        entries_added  -- total number of new ``implements`` entries inserted.
    """
    # Build inverted map: parent_id -> set of child_ids that should be in implements
    to_add = {}
    for req_id, req in all_reqs.items():
        for parent_id in (req.get("uses") or []):
            if parent_id not in all_reqs:
                continue
            existing = set(all_reqs[parent_id].get("implements") or [])
            if req_id not in existing:
                to_add.setdefault(parent_id, set()).add(req_id)

    if not to_add:
        return 0, 0

    # Group affected parent IDs by the source file that contains them
    files_to_update = {}  # rel_path_str -> {parent_id: set_of_new_children}
    for parent_id, new_children in to_add.items():
        src = all_reqs[parent_id]["_source"]
        files_to_update.setdefault(src, {})[parent_id] = new_children

    files_updated = 0
    entries_added = 0

    for rel_path, change_map in sorted(files_to_update.items()):
        abs_path = root / rel_path
        try:
            with open(abs_path, encoding="utf-8") as fh:
                data = json.load(fh)
        except (json.JSONDecodeError, OSError) as exc:
            print("[WARNING] fix-implements: could not read {}: {}".format(rel_path, exc))
            continue

        reqs_list = data.get("requirements", [])
        for req in reqs_list:
            rid = req.get("id")
            if rid not in change_map:
                continue
            existing = set(req.get("implements") or [])
            new_entries = sorted(change_map[rid] - existing)
            req["implements"] = sorted(existing | change_map[rid])
            entries_added += len(new_entries)

        try:
            with open(abs_path, "w", encoding="utf-8") as fh:
                json.dump(data, fh, indent=2)
                fh.write("\n")
            files_updated += 1
        except OSError as exc:
            print("[WARNING] fix-implements: could not write {}: {}".format(rel_path, exc))

    return files_updated, entries_added


# ---------------------------------------------------------------------------
# Annotation scanning
# ---------------------------------------------------------------------------

def scan_annotations(root, dirs, extensions, tag_key):
    """Scan files under *dirs* whose suffix is in *extensions* for
    ``// @{tag_key: [...]}`` annotations.

    Returns:
        tag_map  -- dict mapping req_id -> list of (rel_path_str, lineno)
        errors   -- list of ``[ERROR]`` strings
    """
    tag_map = {}
    errors = []

    for dir_name in dirs:
        scan_dir = root / dir_name
        if not scan_dir.is_dir():
            continue
        for fpath in sorted(scan_dir.rglob("*")):
            if fpath.suffix not in extensions:
                continue
            _scan_file(fpath, root, tag_key, tag_map, errors)

    return tag_map, errors


def _scan_file(fpath, root, tag_key, tag_map, errors):
    rel = str(fpath.relative_to(root))
    try:
        with open(fpath, encoding="utf-8", errors="replace") as fh:
            lines = fh.readlines()
    except OSError as exc:
        errors.append("[ERROR] Could not read {}: {}".format(rel, exc))
        return

    for lineno, line in enumerate(lines, 1):
        m = ANNOTATION_RE.search(line) or CMAKE_ANNOTATION_RE.search(line)
        if not m:
            continue

        try:
            tag = json.loads(m.group(1))
        except json.JSONDecodeError as exc:
            errors.append(
                "[ERROR] {}:{}: malformed annotation JSON: {}".format(rel, lineno, exc)
            )
            continue

        if not isinstance(tag, dict) or tag_key not in tag:
            continue

        ids = tag[tag_key]
        if not isinstance(ids, list):
            errors.append(
                "[ERROR] {}:{}: '{}' value must be a JSON array".format(rel, lineno, tag_key)
            )
            continue

        for req_id in ids:
            if not isinstance(req_id, str):
                errors.append(
                    "[ERROR] {}:{}: '{}' array contains a non-string value".format(
                        rel, lineno, tag_key
                    )
                )
                continue
            tag_map.setdefault(req_id, []).append((rel, lineno))


# ---------------------------------------------------------------------------
# Link integrity
# ---------------------------------------------------------------------------

def check_link_integrity(all_reqs, errors, warnings):
    """Validate ``uses`` and ``implements`` cross-references across all
    loaded requirements and check bidirectional consistency."""
    for req_id in sorted(all_reqs):
        req = all_reqs[req_id]
        src = req["_source"]

        uses = req.get("uses") or []
        if not isinstance(uses, list):
            errors.append(
                "[ERROR] {}: '{}' 'uses' must be an array".format(src, req_id)
            )
            uses = []

        for ref_id in uses:
            if ref_id not in all_reqs:
                errors.append(
                    "[ERROR] {}: '{}' has broken 'uses' link -> '{}' (id not found)".format(
                        src, req_id, ref_id
                    )
                )
            else:
                # Bidirectional: ref_id.implements should contain req_id
                ref_impl = all_reqs[ref_id].get("implements") or []
                if isinstance(ref_impl, list) and req_id not in ref_impl:
                    warnings.append(
                        "[WARNING] Bidirectional inconsistency: '{}'.uses contains '{}' "
                        "but '{}'.implements does not list '{}'".format(
                            req_id, ref_id, ref_id, req_id
                        )
                    )

        implements = req.get("implements") or []
        if not isinstance(implements, list):
            errors.append(
                "[ERROR] {}: '{}' 'implements' must be an array".format(src, req_id)
            )
            implements = []

        for ref_id in implements:
            if ref_id not in all_reqs:
                errors.append(
                    "[ERROR] {}: '{}' has broken 'implements' link -> '{}' (id not found)".format(
                        src, req_id, ref_id
                    )
                )
            else:
                # Bidirectional: ref_id.uses should contain req_id
                ref_uses = all_reqs[ref_id].get("uses") or []
                if isinstance(ref_uses, list) and req_id not in ref_uses:
                    warnings.append(
                        "[WARNING] Bidirectional inconsistency: '{}'.implements contains '{}' "
                        "but '{}'.uses does not list '{}'".format(
                            req_id, ref_id, ref_id, req_id
                        )
                    )


# ---------------------------------------------------------------------------
# Coverage verification
# ---------------------------------------------------------------------------

def verify_coverage(all_reqs, req_tags, verify_tags, module_filter,
                    errors, warnings, info, verbose):
    """Cross-reference loaded requirements with source and test annotations.

    When *module_filter* is set, coverage checks and orphan reports are
    restricted to requirement IDs whose prefix matches the filter.
    """
    filter_prefix = "REQ-{}-".format(module_filter.upper()) if module_filter else None

    # Determine which requirement IDs to report coverage for
    if filter_prefix:
        report_ids = sorted(rid for rid in all_reqs if rid.startswith(filter_prefix))
    else:
        report_ids = sorted(all_reqs)

    for req_id in report_ids:
        req = all_reqs[req_id]
        method = req.get("verification_method", "")

        # WARNING: no @req annotation in source
        if req_id not in req_tags:
            warnings.append(
                "[WARNING] '{}' ('{}') has no @req annotation in source".format(
                    req_id, req.get("title", "")
                )
            )
        elif verbose:
            for fpath, lineno in req_tags[req_id]:
                info.append("[INFO] '{}' implemented at {}:{}".format(req_id, fpath, lineno))

        # ERROR: Test-method requirements must have a @verify annotation
        if method == "Test":
            if req_id not in verify_tags:
                errors.append(
                    "[ERROR] '{}' ('{}') has verification_method 'Test' "
                    "but no @verify annotation in tests".format(req_id, req.get("title", ""))
                )
            elif verbose:
                for fpath, lineno in verify_tags[req_id]:
                    info.append("[INFO] '{}' verified at {}:{}".format(req_id, fpath, lineno))

    # ERROR: orphaned @req tags
    for req_id in sorted(req_tags):
        if req_id in all_reqs:
            continue
        if filter_prefix and not req_id.startswith(filter_prefix):
            continue
        for fpath, lineno in req_tags[req_id]:
            errors.append(
                "[ERROR] {}:{}: orphaned @req tag '{}' "
                "(not found in any requirements.json)".format(fpath, lineno, req_id)
            )

    # ERROR: orphaned @verify tags
    for req_id in sorted(verify_tags):
        if req_id in all_reqs:
            continue
        if filter_prefix and not req_id.startswith(filter_prefix):
            continue
        for fpath, lineno in verify_tags[req_id]:
            errors.append(
                "[ERROR] {}:{}: orphaned @verify tag '{}' "
                "(not found in any requirements.json)".format(fpath, lineno, req_id)
            )


# ---------------------------------------------------------------------------
# Report printing
# ---------------------------------------------------------------------------

def print_report(all_reqs, req_tags, verify_tags, errors, warnings, info,
                 module_filter, verbose):
    width = 60
    print()
    print("=" * width)
    print("TRACEABILITY COVERAGE REPORT")
    print("=" * width)
    if module_filter:
        print("Module filter      : {}".format(module_filter.upper()))
    print("Requirements loaded: {}".format(len(all_reqs)))
    print(
        "Source @req tags   : {}".format(
            sum(len(v) for v in req_tags.values())
        )
    )
    print(
        "Test @verify tags  : {}".format(
            sum(len(v) for v in verify_tags.values())
        )
    )
    print()

    if verbose and info:
        for msg in sorted(info):
            print(msg)
        print()

    if warnings:
        print("Warnings ({}):".format(len(warnings)))
        for msg in warnings:
            print("  " + msg)
        print()

    if errors:
        print("Errors ({}):".format(len(errors)))
        for msg in errors:
            print("  " + msg)
        print()

    print("-" * width)
    if errors:
        print(
            "RESULT: FAIL  -- {} error(s), {} warning(s)".format(
                len(errors), len(warnings)
            )
        )
    else:
        print(
            "RESULT: PASS  -- 0 errors, {} warning(s)".format(len(warnings))
        )


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Verify traceability coverage for LibJuno requirements.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "Exit code: 0 = no errors, 1 = errors found.\n\n"
            "Examples:\n"
            "  python scripts/verify_traceability.py\n"
            "  python scripts/verify_traceability.py --module HEAP\n"
            "  python scripts/verify_traceability.py --verbose\n"
            "  python scripts/verify_traceability.py --root /path/to/project --module CRC\n"
        ),
    )
    parser.add_argument(
        "--root",
        metavar="PATH",
        default=None,
        help="Project root directory (default: auto-detect from cwd)",
    )
    parser.add_argument(
        "--module",
        metavar="MODULE",
        default=None,
        help=(
            "Restrict coverage report to one module (e.g. HEAP, CRC). "
            "All requirements are still loaded for link-integrity checks."
        ),
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print INFO lines showing where each requirement is annotated",
    )
    parser.add_argument(
        "--fix-implements",
        action="store_true",
        help=(
            "Auto-populate missing 'implements' arrays by inverting 'uses' links, "
            "then write the corrected requirements.json files back to disk."
        ),
    )
    args = parser.parse_args()

    # Resolve project root
    try:
        root = Path(args.root).resolve() if args.root else find_root(Path.cwd())
    except FileNotFoundError as exc:
        print("ERROR: {}".format(exc), file=sys.stderr)
        sys.exit(1)

    print("Project root: {}".format(root))

    errors = []
    warnings = []
    info = []

    # Step 1: Load ALL requirements (needed for link-integrity checks)
    all_reqs, req_load_errors, req_load_warnings = load_requirements(root)
    errors.extend(req_load_errors)
    warnings.extend(req_load_warnings)

    # Optional: fix missing implements arrays before validation
    if args.fix_implements:
        files_updated, entries_added = fix_implements(root, all_reqs)
        print(
            "fix-implements: {} file(s) updated, {} implements entries added".format(
                files_updated, entries_added
            )
        )
        if files_updated > 0:
            # Re-load so the rest of the report reflects the fixed state
            all_reqs, req_load_errors2, req_load_warnings2 = load_requirements(root)
            errors.extend(req_load_errors2)
            warnings.extend(req_load_warnings2)

    # Step 2: Scan source files for @req tags
    req_tags, src_errors = scan_annotations(
        root, ["src", "include", "vscode-extension/src"], {".c", ".h", ".ts"}, "req"
    )
    errors.extend(src_errors)

    # Also scan all CMakeLists.txt files in the project for @req tags
    cmake_errors = []
    for cmake_file in sorted(root.rglob("CMakeLists.txt")):
        _scan_file(cmake_file, root, "req", req_tags, cmake_errors)
    errors.extend(cmake_errors)

    # Step 3: Scan test files for @verify tags
    verify_tags, test_errors = scan_annotations(
        root, ["tests", "src", "vscode-extension/src"], {".c", ".cpp", ".ts"}, "verify"
    )
    errors.extend(test_errors)

    # Step 4: Link integrity (always across all requirements)
    check_link_integrity(all_reqs, errors, warnings)

    # Step 5: Coverage verification (respects --module filter)
    verify_coverage(
        all_reqs, req_tags, verify_tags,
        args.module, errors, warnings, info, args.verbose,
    )

    # Step 6: Print report
    print_report(
        all_reqs, req_tags, verify_tags,
        errors, warnings, info,
        args.module, args.verbose,
    )

    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()

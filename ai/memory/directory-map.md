# Directory Map — LibJuno Repository

## 1. Repository Layout

| Sub-Project | Root Directory | Description |
|-------------|---------------|-------------|
| LibJuno C library | `/workspaces/libjuno` | C11 embedded micro-framework |
| VSCode Extension | `/workspaces/libjuno/vscode-extension` | TypeScript VS Code extension |
| Python scripts | `/workspaces/libjuno` | Utility and verification scripts |

## 2. Command Directory Reference

| Command | Working Directory | Purpose |
|---------|-------------------|---------|
| `cd build && cmake --build .` | `/workspaces/libjuno` | Build LibJuno C library |
| `cd build && ctest --output-on-failure` | `/workspaces/libjuno` | Run C unit tests (Unity) |
| `cd build && ctest -R <name> --output-on-failure` | `/workspaces/libjuno` | Run specific C test |
| `python3 scripts/verify_traceability.py` | `/workspaces/libjuno` | Verify traceability annotations |
| `python3 scripts/generate_docs.py` | `/workspaces/libjuno` | Generate documentation |
| `npm test` | `/workspaces/libjuno/vscode-extension` | Run VSCode extension Jest tests |
| `npx jest --verbose` | `/workspaces/libjuno/vscode-extension` | Run Jest tests (verbose) |
| `npx jest --coverage` | `/workspaces/libjuno/vscode-extension` | Run Jest with coverage |
| `npm run compile` | `/workspaces/libjuno/vscode-extension` | Compile TypeScript extension |
| `npx stryker run` | `/workspaces/libjuno/vscode-extension` | Run mutation testing |

## 3. Pre-Command Checklist

1. Identify which sub-project the command belongs to (C library, VSCode extension, or Python scripts)
2. `cd` to the correct absolute directory FIRST
3. Use `pwd` to verify you're in the right place if unsure
4. Use absolute paths in `cd` commands (e.g., `cd /workspaces/libjuno/vscode-extension` not `cd vscode-extension`)

## 4. Common Mistakes

- Running `npm test` from `/workspaces/libjuno` (wrong — must be in `vscode-extension/`)
- Running `npx jest` from `/workspaces/libjuno` (wrong — must be in `vscode-extension/`)
- Running `cd build && cmake --build .` from `vscode-extension/` (wrong — must be in project root)
- Running `python3 scripts/verify_traceability.py` from `vscode-extension/` (wrong — must be in project root)
- Using relative `cd build` without first being in `/workspaces/libjuno`

## 5. Safe Command Patterns

**LibJuno C build + test:**
```bash
cd /workspaces/libjuno && cd build && cmake --build . && ctest --output-on-failure
```

**VSCode Extension test:**
```bash
cd /workspaces/libjuno/vscode-extension && npm test
```

**Traceability verification:**
```bash
cd /workspaces/libjuno && python3 scripts/verify_traceability.py
```

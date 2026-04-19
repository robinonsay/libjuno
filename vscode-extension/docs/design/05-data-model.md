> Part of: [Software Design Document](index.md) — Section 4: Data Model

// @{"design": ["REQ-VSCODE-003", "REQ-VSCODE-008", "REQ-VSCODE-009", "REQ-VSCODE-010", "REQ-VSCODE-011", "REQ-VSCODE-012", "REQ-VSCODE-014", "REQ-VSCODE-015"]}
## 4. Data Model

### 4.1 In-Memory Navigation Index

```typescript
interface NavigationIndex {
  // REQ-VSCODE-008: rootType → apiType (from JUNO_MODULE_ROOT)
  moduleRoots: Map<string, string>;

  // REQ-VSCODE-014: traitRootType → apiType (from JUNO_TRAIT_ROOT)
  traitRoots: Map<string, string>;

  // REQ-VSCODE-009, REQ-VSCODE-015: derivedType → rootType
  // (from JUNO_MODULE_DERIVE / JUNO_TRAIT_DERIVE)
  derivationChain: Map<string, string>;

  // REQ-VSCODE-012: apiType → ordered list of function pointer field names
  // Required for positional initializer resolution
  apiStructFields: Map<string, string[]>;

  // REQ-VSCODE-010, REQ-VSCODE-011, REQ-VSCODE-012:
  // apiType → fieldName → list of concrete implementations
  // ConcreteLocation.line is the function definition line (resolved in the same parse pass)
  vtableAssignments: Map<string, Map<string, ConcreteLocation[]>>;

  // REQ-VSCODE-016: rootType → list of concrete failure handlers
  failureHandlerAssignments: Map<string, ConcreteLocation[]>;

  // Chain-walk algorithm Step 5a: struct member name → API pointer type.
  // Populated during indexing: for any struct body (ROOT, IMPL, or any other),
  // if a member's declared type ends in _API_T it is recorded here.
  // Example: ptHeapPointerApi → JUNO_DS_HEAP_POINTER_API_T
  // Used by the chain-walk fallback to resolve non-ptApi API member names at call sites.
  apiMemberRegistry: Map<string, string>;

  // visitFunctionDefinition: functionName → list of definition records.
  // Multiple entries arise when identically named static functions exist in different files.
  // Used during vtable assignment resolution and as a fallback for the Vtable Resolver.
  functionDefinitions: Map<string, FunctionDefinitionRecord[]>;

  // Per-file local variable and parameter type information.
  // Populated by visitLocalDeclaration and visitFunctionParameters.
  // Keyed by workspace-relative file path.
  localTypeInfo: Map<string, LocalTypeInfo>;
}

interface ConcreteLocation {
  functionName:    string;
  file:            string;
  line:            number;      // Line of the function definition
  assignmentFile?: string;      // File where the vtable assignment occurs (composition root)
  assignmentLine?: number;      // Line of the vtable assignment (composition root)
}

interface FunctionDefinitionRecord {
  functionName: string;
  file:         string;
  line:         number;
  isStatic:     boolean;  // true if the function was declared with the `static` keyword
  signature?:   string;  // Full function signature text (return type + name + params)
}
```

The `NavigationIndex` is held in memory by the Workspace Indexer and shared by reference with the Vtable Resolver, Failure Handler Resolver, and MCP Server.

### 4.2 JSON Cache Schema

File: `.libjuno/navigation-cache.json`

```jsonc
{
  "version": "1",
  "generatedAt": "2026-04-14T12:00:00.000Z",
  "fileHashes": {
    "include/juno/ds/heap_api.h": "a3f...bc1",
    "src/juno_heap.c": "9d2...f44"
  },
  "moduleRoots": {
    "JUNO_DS_HEAP_ROOT_T": "JUNO_DS_HEAP_API_T"
  },
  "traitRoots": {
    "JUNO_POINTER_T": "JUNO_POINTER_API_T"
  },
  "derivationChain": {
    "JUNO_DS_HEAP_IMPL_T": "JUNO_DS_HEAP_ROOT_T"
  },
  "apiStructFields": {
    "JUNO_DS_HEAP_API_T": ["Insert", "Heapify", "Pop"]
  },
  "vtableAssignments": {
    "JUNO_DS_HEAP_API_T": {
      "Insert":  [{ "functionName": "JunoDs_Heap_Insert",  "file": "src/juno_heap.c", "line": 259, "assignmentFile": "src/juno_heap.c", "assignmentLine": 18 }],
      "Heapify": [{ "functionName": "JunoDs_Heap_Heapify", "file": "src/juno_heap.c", "line": 270, "assignmentFile": "src/juno_heap.c", "assignmentLine": 19 }],
      "Pop":     [{ "functionName": "JunoDs_Heap_Pop",     "file": "src/juno_heap.c", "line": 285, "assignmentFile": "src/juno_heap.c", "assignmentLine": 20 }]
    }
  },
  "failureHandlerAssignments": {
    "JUNO_DS_HEAP_ROOT_T": [
      { "functionName": "MyFailureHandler", "file": "examples/example.c", "line": 42 }
    ]
  },
  "apiMemberRegistry": {
    "ptHeapPointerApi": "JUNO_DS_HEAP_POINTER_API_T"
  },
  "functionDefinitions": {
    "Publish": [
      { "file": "src/juno_broker.c", "line": 51, "isStatic": true, "signature": "static JUNO_STATUS_T Publish(JUNO_BROKER_ROOT_T *ptBroker, JUNO_BROKER_TOPIC_T tTopic, JUNO_POINTER_T tData)" }
    ],
    "JunoDs_Heap_Insert": [
      { "file": "src/juno_heap.c", "line": 259, "isStatic": false, "signature": "JUNO_STATUS_T JunoDs_Heap_Insert(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue)" }
    ]
  },
  "localTypeInfo": {
    "src/juno_heap.c": {
      "localVariables": {
        "JunoDs_Heap_Init": {
          "iCounter": { "name": "iCounter", "typeName": "int", "isPointer": false, "isConst": false, "isArray": false }
        }
      },
      "functionParameters": {
        "JunoDs_Heap_Init": [
          { "name": "ptHeap", "typeName": "JUNO_DS_HEAP_ROOT_T", "isPointer": true, "isConst": false, "isArray": false }
        ]
      }
    }
  }
}
```

**Schema fields:**

| Field | Type | Description |
|-------|------|-------------|
| `version` | `string` | Cache format version. Cache is discarded on version mismatch. |
| `generatedAt` | `string` (ISO 8601) | Timestamp of last full index. |
| `fileHashes` | `object` | workspace-relative path → SHA-256 hex of file content. |
| `moduleRoots` | `object` | rootType → apiType (from `JUNO_MODULE_ROOT`). |
| `traitRoots` | `object` | traitRootType → apiType (from `JUNO_TRAIT_ROOT`). |
| `derivationChain` | `object` | derivedType → immediate rootType. Chain is walked at resolution time. |
| `apiStructFields` | `object` | apiType → ordered field names (from struct definition). |
| `vtableAssignments` | `object` | apiType → fieldName → array of `ConcreteLocation`. Each `line` is the function **definition** line; `assignmentFile`/`assignmentLine` record the vtable assignment site (composition root). |
| `failureHandlerAssignments` | `object` | rootType → array of `ConcreteLocation`. |
| `apiMemberRegistry` | `object` | struct member name → API pointer type (`_API_T`). Built during indexing from all struct definitions. Used by the chain-walk fallback to resolve non-`ptApi` API member names. |
| `functionDefinitions` | `object` | functionName → array of `FunctionDefinitionRecord` (`file`, `line`, `isStatic`, `signature?`). Multiple entries arise when identically named `static` functions exist in different files. |
| `localTypeInfo` | `object` | filePath → `LocalTypeInfo` (local variable and parameter type maps, keyed per function). Used at query time for the chain-walk type resolver. |

---

// @{"design": ["REQ-VSCODE-003"]}
## 4.3 Per-File Type Information

```typescript
interface LocalTypeInfo {
  /** functionName → Map of variableName → declared type */
  localVariables: Map<string, Map<string, TypeInfo>>;
  /** functionName → list of parameter type entries */
  functionParameters: Map<string, TypeInfo[]>;
}

interface TypeInfo {
  name:      string;   // variable/parameter name
  typeName:  string;   // declared type, e.g. "JUNO_DS_HEAP_ROOT_T"
  isPointer: boolean;  // true if declared as pointer (*)
  isConst:   boolean;  // true if declared with const
  isArray:   boolean;  // true if declared as array
}
```

`LocalTypeInfo` is populated by `visitLocalDeclaration` and `visitFunctionParameters` during the parse pass. It is stored in `NavigationIndex.localTypeInfo` (keyed by file path) and serialized to the JSON cache under `localTypeInfo`.

---

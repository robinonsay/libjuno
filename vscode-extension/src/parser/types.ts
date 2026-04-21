// @{"req": ["REQ-VSCODE-001", "REQ-VSCODE-003", "REQ-VSCODE-031", "REQ-VSCODE-041"]}
/**
 * @file types.ts
 *
 * All shared TypeScript interfaces for the LibJuno VSCode Extension.
 *
 * Covers:
 *   - Parser output records (Section 3.1)
 *   - In-memory NavigationIndex and supporting types (Section 4.1)
 *   - JSON cache schema (Section 4.2)
 *   - Per-file type information (Section 4.3)
 *   - Resolver output types (Section 3.3 / 3.4)
 */

// ---------------------------------------------------------------------------
// Parser output types (Section 3.1)
// ---------------------------------------------------------------------------

/**
 * Full output produced by the Chevrotain parser + CST visitor for a single
 * C source file. Consumed by the Workspace Indexer to populate the
 * NavigationIndex.
 */
export interface ParsedFile {
  /** Absolute path of the parsed file. */
  filePath: string;
  /** All JUNO_MODULE_ROOT declarations found in the file. */
  moduleRoots: ModuleRootRecord[];
  /** All JUNO_TRAIT_ROOT declarations found in the file. */
  traitRoots: TraitRootRecord[];
  /** All JUNO_MODULE_DERIVE / JUNO_TRAIT_DERIVE declarations found in the file. */
  derivations: DerivationRecord[];
  /** All API struct definitions (structs whose tag ends in _API_TAG). */
  apiStructDefinitions: ApiStructRecord[];
  /** All vtable field assignments found in the file. */
  vtableAssignments: VtableAssignmentRecord[];
  /** All failure handler assignments found in the file. */
  failureHandlerAssigns: FailureHandlerRecord[];
  /** Positional vtable initializers that could not be resolved at parse time (API struct in different file). */
  pendingPositionalVtables: PendingPositionalVtable[];
  /** Call sites where a known API struct variable's address is passed as an argument. Populated by scanInitCallSites(). */
  initCallSites: InitCallRecord[];
  /** Local variable and parameter type information for all functions in the file. */
  localTypeInfo: LocalTypeInfo;
}

/**
 * A JUNO_MODULE_ROOT declaration. Records the mapping from a module root
 * struct type to its API struct type.
 *
 * Example source: `struct JUNO_DS_HEAP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_DS_HEAP_API_T, ...)`
 */
export interface ModuleRootRecord {
  /** e.g. "JUNO_DS_HEAP_ROOT_T" (produced by _TAG → _T substitution on the struct tag). */
  rootType: string;
  /** e.g. "JUNO_DS_HEAP_API_T" (first macro argument). */
  apiType: string;
  /** Absolute path of the file that contains this declaration. */
  file: string;
  /** 1-based line number of this declaration. */
  line: number;
}

/**
 * A JUNO_TRAIT_ROOT declaration. Semantically identical to ModuleRootRecord
 * but for trait root types.
 *
 * Example source: `struct JUNO_POINTER_TAG JUNO_TRAIT_ROOT(JUNO_POINTER_API_T, ...)`
 */
export interface TraitRootRecord {
  /** e.g. "JUNO_POINTER_T". */
  rootType: string;
  /** e.g. "JUNO_POINTER_API_T". */
  apiType: string;
  /** Absolute path of the file that contains this declaration. */
  file: string;
  /** 1-based line number of this declaration. */
  line: number;
}

/**
 * A JUNO_MODULE_DERIVE or JUNO_TRAIT_DERIVE declaration. Records the
 * single-step parent relationship in the derivation chain.
 *
 * Example source: `struct JUNO_DS_HEAP_IMPL_TAG JUNO_MODULE_DERIVE(JUNO_DS_HEAP_ROOT_T, ...)`
 */
export interface DerivationRecord {
  /** e.g. "JUNO_DS_HEAP_IMPL_T" (the derived / child type). */
  derivedType: string;
  /** e.g. "JUNO_DS_HEAP_ROOT_T" (the immediate parent type). */
  rootType: string;
  /** Absolute path of the file that contains this declaration. */
  file: string;
  /** 1-based line number of this declaration. */
  line: number;
}

/**
 * An API struct definition — a struct whose tag ends in `_API_TAG`. Captures
 * the ordered list of function pointer field names, required for positional
 * initializer resolution (REQ-VSCODE-012).
 */
export interface ApiStructRecord {
  /** e.g. "JUNO_DS_HEAP_API_T" (produced by _API_TAG → _API_T substitution). */
  apiType: string;
  /** Document-order function pointer field names, e.g. ["Insert", "Heapify", "Pop"]. */
  fields: string[];
  /** Absolute path of the file that contains this definition. */
  file: string;
  /** 1-based line number of this definition. */
  line: number;
}

/**
 * One vtable field assignment — maps an API type's function pointer field to
 * the concrete function that implements it.
 *
 * Produced by designated initializers (REQ-VSCODE-010), direct assignments
 * (REQ-VSCODE-011), and positional initializers (REQ-VSCODE-012).
 */
export interface VtableAssignmentRecord {
  /** e.g. "JUNO_DS_HEAP_API_T". */
  apiType: string;
  /** e.g. "Insert". */
  field: string;
  /** e.g. "JunoDs_Heap_Insert". */
  functionName: string;
  /** Absolute path of the file that contains this assignment. */
  file: string;
  /** 1-based line number of this assignment. */
  line: number;
  /** Variable name of the API struct containing this assignment (e.g. "gtMyLoggerApi"). Populated for top-level struct declarations. Used to locate the composition root call site. */
  varName?: string;
}

/**
 * A failure handler assignment — maps a module root type to the concrete
 * error/failure handler function registered for it (REQ-VSCODE-016).
 *
 * The `rootType` may be empty at parse time when it cannot be inferred from
 * the local source text; it is resolved during index-merge from LocalTypeInfo.
 */
export interface FailureHandlerRecord {
  /**
   * The module root type for which this handler is registered.
   * May be empty string at parse time; resolved at index-merge time.
   */
  rootType: string;
  /** e.g. "MyFailureHandler". */
  functionName: string;
  /** Absolute path of the file that contains this assignment. */
  file: string;
  /** 1-based line number of this assignment. */
  line: number;
}

/**
 * A positional vtable initializer that could not be resolved at parse time
 * because the API struct definition was not in the same file.
 * Deferred for cross-file resolution during full workspace indexing.
 */
export interface PendingPositionalVtable {
  /** The API struct type name (e.g. "MY_API_T"). */
  apiType: string;
  /** Function names in positional order. */
  initializers: string[];
  /** Absolute path of the file that contains this initializer. */
  file: string;
  /** 1-based line numbers corresponding to each initializer. */
  lines: number[];
  varName?: string;
}

/**
 * A call site where a known API struct variable's address is passed as an argument.
 * Used to locate the composition root — the site where a module instance is wired
 * to its concrete vtable at runtime (REQ-VSCODE-036).
 */
export interface InitCallRecord {
  /** Variable name of the API struct being passed, e.g. "gtMyLoggerApi". */
  apiVarName: string;
  /** Absolute path of the file containing this call site. */
  file: string;
  /** 1-based line number of the call site. */
  line: number;
}

// ---------------------------------------------------------------------------
// Per-file type information (Section 4.3)
// ---------------------------------------------------------------------------

/**
 * Per-file collection of local variable and parameter type maps, keyed by
 * function name. Populated by visitLocalDeclaration and
 * visitFunctionParameters during the parse pass.
 */
export interface LocalTypeInfo {
  /**
   * functionName → Map of variableName → TypeInfo.
   * Contains all local variables declared inside the named function body.
   */
  localVariables: Map<string, Map<string, TypeInfo>>;
  /**
   * functionName → ordered list of TypeInfo, one per parameter.
   * Contains the declared type information for each function parameter.
   */
  functionParameters: Map<string, TypeInfo[]>;
}

/**
 * Declared type information for one local variable or function parameter.
 * Used by the chain-walk algorithm (Section 5.1 Step 4a) to resolve the
 * type of the primary expression without a backward regex scan.
 */
export interface TypeInfo {
  /** Variable or parameter name, e.g. "ptHeap". */
  name: string;
  /** Declared type name, e.g. "JUNO_DS_HEAP_ROOT_T". */
  typeName: string;
  /** true if the declaration includes a `*` pointer qualifier. */
  isPointer: boolean;
  /** true if the declaration includes the `const` qualifier. */
  isConst: boolean;
  /** true if the declaration is an array (e.g. `TYPE name[]`). */
  isArray: boolean;
}

// ---------------------------------------------------------------------------
// In-memory Navigation Index (Section 4.1)
// ---------------------------------------------------------------------------

/**
 * In-memory index populated by the Workspace Indexer and shared by reference
 * with the Vtable Resolver, Failure Handler Resolver, and MCP Server.
 */
export interface NavigationIndex {
  /**
   * REQ-VSCODE-008: rootType → apiType, from JUNO_MODULE_ROOT declarations.
   * Example: "JUNO_DS_HEAP_ROOT_T" → "JUNO_DS_HEAP_API_T"
   */
  moduleRoots: Map<string, string>;

  /**
   * REQ-VSCODE-014: traitRootType → apiType, from JUNO_TRAIT_ROOT declarations.
   * Example: "JUNO_POINTER_T" → "JUNO_POINTER_API_T"
   */
  traitRoots: Map<string, string>;

  /**
   * REQ-VSCODE-009, REQ-VSCODE-015: derivedType → immediate rootType.
   * The Vtable Resolver walks this chain to reach the topmost root type.
   * Example: "JUNO_DS_HEAP_IMPL_T" → "JUNO_DS_HEAP_ROOT_T"
   */
  derivationChain: Map<string, string>;

  /**
   * REQ-VSCODE-012: apiType → ordered list of function pointer field names.
   * Required for positional initializer resolution.
   * Example: "JUNO_DS_HEAP_API_T" → ["Insert", "Heapify", "Pop"]
   */
  apiStructFields: Map<string, string[]>;

  /**
   * REQ-VSCODE-010, REQ-VSCODE-011, REQ-VSCODE-012:
   * apiType → fieldName → list of concrete implementations.
   * ConcreteLocation.line is the function definition line, resolved during
   * the same parse pass as the vtable assignment.
   */
  vtableAssignments: Map<string, Map<string, ConcreteLocation[]>>;

  /**
   * REQ-VSCODE-016: rootType → list of concrete failure handler locations.
   * Example: "JUNO_DS_HEAP_ROOT_T" → [ConcreteLocation, ...]
   */
  failureHandlerAssignments: Map<string, ConcreteLocation[]>;

  /**
   * struct member name → API pointer type (ends in _API_T).
   * Populated during indexing for all struct bodies (ROOT, IMPL, or any other)
   * where a member's declared type ends in _API_T.
   * Used by the chain-walk fallback (Section 5.1 Step 5a) to resolve
   * non-ptApi API member names at call sites.
   * Example: "ptHeapPointerApi" → "JUNO_DS_HEAP_POINTER_API_T"
   */
  apiMemberRegistry: Map<string, string>;

  /**
   * functionName → list of definition records.
   * Multiple entries arise when identically named static functions exist in
   * different files. Used during vtable assignment resolution and as a fallback
   * for the Vtable Resolver.
   */
  functionDefinitions: Map<string, FunctionDefinitionRecord[]>;

  /**
   * Per-file local variable and parameter type information.
   * Keyed by workspace-relative file path.
   * Populated by visitLocalDeclaration and visitFunctionParameters.
   */
  localTypeInfo: Map<string, LocalTypeInfo>;

  /**
   * REQ-VSCODE-036: apiVarName → list of call sites where &apiVarName appears as a
   * function argument. Populated by resolveCompositionRoots() after full index.
   * Used to stamp ConcreteLocation.initCallFile / initCallLine.
   */
  initCallIndex: Map<string, Array<{ file: string; line: number }>>;
}

/**
 * One function definition record, produced by visitFunctionDefinition during
 * the parse pass. The definition line is resolved in the same pass as vtable
 * assignments, eliminating the need for a second indexing pass.
 */
export interface FunctionDefinitionRecord {
  /** Fully qualified function name, e.g. "JunoDs_Heap_Insert". */
  functionName: string;
  /** Absolute path of the file containing this definition. */
  file: string;
  /** 1-based line number of the function definition. */
  line: number;
  /** true if the function was declared with the `static` storage-class keyword. */
  isStatic: boolean;
  /** Full function signature text (return type + name + params), e.g. "JunoDs_Heap_Insert(...)". */
  signature?: string;
}

/**
 * A resolved concrete implementation location. Used as elements in
 * VtableResolutionResult.locations and in failureHandlerAssignments.
 *
 * `line` is always the function **definition** line (not a call site or
 * forward declaration), resolved during the indexing parse pass.
 */
export interface ConcreteLocation {
  /** Fully qualified function name, e.g. "JunoDs_Heap_Insert". */
  functionName: string;
  /** Absolute path of the file containing the function definition. */
  file: string;
  /** 1-based line number of the function definition. */
  line: number;
  /** File where the vtable assignment occurs (composition root). REQ-VSCODE-031. */
  assignmentFile?: string;
  /** 1-based line of the vtable assignment (composition root). REQ-VSCODE-031. */
  assignmentLine?: number;
  /** Variable name of the API struct used in the vtable assignment. Used for cross-referencing initCallIndex. */
  apiVarName?: string;
  /** File where the composition-root call site occurs (preferred over assignmentFile for display). REQ-VSCODE-036. */
  initCallFile?: string;
  /** 1-based line of the composition-root call site. REQ-VSCODE-036. */
  initCallLine?: number;
  /** Caller of the Init function — the true composition root in user code (REQ-VSCODE-037). */
  compRootFile?: string;
  /** 1-based line of the composition root call site. */
  compRootLine?: number;
  /**
   * Discriminates the origin of this location. REQ-VSCODE-041.
   * - 'assignment' — location came from failureHandlerAssignments (a handler wiring site).
   * - 'invocation' — location came from functionDefinitions reached via a JUNO_FAIL* macro
   *   call site (the cursor is on an invocation, navigating to the handler definition).
   */
  kind?: 'assignment' | 'invocation';
}

// ---------------------------------------------------------------------------
// Resolver output types (Section 3.3 / 3.4)
// ---------------------------------------------------------------------------

/**
 * Output of VtableResolver and FailureHandlerResolver.
 * Returned to the VSCode Integration Layer and the MCP Server.
 */
export interface VtableResolutionResult {
  /** true if at least one concrete implementation was found. */
  found: boolean;
  /** Concrete implementation locations; empty when found is false. */
  locations: ConcreteLocation[];
  /** Human-readable explanation when found is false. */
  errorMsg?: string;
}

// ---------------------------------------------------------------------------
// JSON Cache schema (Section 4.2)
// ---------------------------------------------------------------------------

/**
 * Root schema of the JSON file written to `.libjuno/navigation-cache.json`.
 *
 * Maps are serialized as plain objects so they survive JSON round-trips.
 * The Workspace Indexer converts these objects back to ES6 Maps on load.
 *
 * Example path: <workspaceRoot>/.libjuno/navigation-cache.json
 */
export interface CacheFile {
  /** Cache format version string. Cache is discarded on version mismatch. */
  version: string;
  /** ISO 8601 timestamp of the last full index run. */
  generatedAt: string;
  /**
   * workspace-relative file path → SHA-256 hex digest of file content.
   * Used to detect stale cached entries on activation.
   */
  fileHashes: Record<string, string>;
  /**
   * rootType → apiType (from JUNO_MODULE_ROOT).
   * Serialized form of NavigationIndex.moduleRoots.
   */
  moduleRoots: Record<string, string>;
  /**
   * traitRootType → apiType (from JUNO_TRAIT_ROOT).
   * Serialized form of NavigationIndex.traitRoots.
   */
  traitRoots: Record<string, string>;
  /**
   * derivedType → immediate rootType.
   * Serialized form of NavigationIndex.derivationChain.
   */
  derivationChain: Record<string, string>;
  /**
   * apiType → ordered field names.
   * Serialized form of NavigationIndex.apiStructFields.
   */
  apiStructFields: Record<string, string[]>;
  /**
   * apiType → fieldName → ConcreteLocation[].
   * Serialized form of NavigationIndex.vtableAssignments.
   */
  vtableAssignments: Record<string, Record<string, ConcreteLocation[]>>;
  /**
   * rootType → ConcreteLocation[].
   * Serialized form of NavigationIndex.failureHandlerAssignments.
   */
  failureHandlerAssignments: Record<string, ConcreteLocation[]>;
  /**
   * struct member name → API pointer type.
   * Serialized form of NavigationIndex.apiMemberRegistry.
   */
  apiMemberRegistry: Record<string, string>;
  /**
   * functionName → FunctionDefinitionRecord[].
   * Serialized form of NavigationIndex.functionDefinitions.
   */
  functionDefinitions: Record<string, Array<Omit<FunctionDefinitionRecord, 'functionName'>>>;
  /**
   * workspace-relative filePath → serialized LocalTypeInfo.
   * The nested Maps are serialized as plain objects keyed by function name /
   * variable name respectively.
   */
  localTypeInfo: Record<string, CacheLocalTypeInfo>;
}

/**
 * Serialized form of LocalTypeInfo used inside CacheFile.
 * ES6 Maps are replaced by plain objects for JSON compatibility.
 */
export interface CacheLocalTypeInfo {
  /** functionName → { variableName → TypeInfo } */
  localVariables: Record<string, Record<string, TypeInfo>>;
  /** functionName → TypeInfo[] */
  functionParameters: Record<string, TypeInfo[]>;
}

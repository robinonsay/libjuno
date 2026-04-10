/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "LibJuno", "index.html", [
    [ "LibJuno", "index.html#autotoc_md40", null ],
    [ "The \"Library of Everything\" Problem", "index.html#autotoc_md41", null ],
    [ "Core Philosophy", "index.html#autotoc_md42", [
      [ "Memory Safety, Pointers, and Arrays", "index.html#autotoc_md43", null ]
    ] ],
    [ "Templates and Scripts", "index.html#autotoc_md44", null ],
    [ "Tutorial", "index.html#autotoc_md45", null ],
    [ "Version and API Stability", "index.html#autotoc_md46", null ],
    [ "Using LibJuno", "index.html#autotoc_md47", [
      [ "Documentation", "index.html#autotoc_md48", null ],
      [ "Dependencies", "index.html#autotoc_md49", null ],
      [ "Building and Testing", "index.html#autotoc_md50", [
        [ "CMake Build Options", "index.html#autotoc_md51", [
          [ "Freestanding Mode", "index.html#autotoc_md52", null ],
          [ "Development and Testing Workflow", "index.html#autotoc_md53", null ]
        ] ]
      ] ],
      [ "Installation", "index.html#autotoc_md54", null ],
      [ "Contributing", "index.html#autotoc_md55", null ],
      [ "Inspiration for the Name", "index.html#autotoc_md56", null ]
    ] ],
    [ "Juno Memory Module", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html", [
      [ "Overview", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md1", null ],
      [ "Core types (public headers)", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md2", [
        [ "<tt>JUNO_MEMORY_BLOCK_METADATA_T</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md3", null ],
        [ "<tt>JUNO_POINTER_T</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md4", null ],
        [ "<tt>JUNO_POINTER_API_T</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md5", null ],
        [ "<tt>JUNO_MEMORY_ALLOC_API_T</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md6", null ],
        [ "<tt>JUNO_MEMORY_ALLOC_ROOT_T</tt> and <tt>JUNO_MEMORY_ALLOC_BLOCK_T</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md7", null ],
        [ "<tt>JUNO_VALUE_POINTER_T</tt> and <tt>JUNO_VALUE_POINTER_API_T</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md8", null ]
      ] ],
      [ "Declaring memory and metadata", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md9", null ],
      [ "Pointer API (Copy and Reset)", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md10", [
        [ "Helpers provided by the API", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md11", null ]
      ] ],
      [ "Initializing the block allocator", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md12", null ],
      [ "Allocating, updating, and freeing", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md13", [
        [ "Typed convenience macros", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md14", null ]
      ] ],
      [ "Internal allocator behavior", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md15", null ],
      [ "Common pitfalls", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md16", null ],
      [ "Minimal end-to-end example (compiles like the tests)", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md17", null ],
      [ "Freestanding notes", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md18", null ],
      [ "Error Handling", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md19", null ],
      [ "Best Practices", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md20", null ],
      [ "Future work", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2memory_2README.html#autotoc_md21", null ]
    ] ],
    [ "LibJuno Module System & Dependency Injection", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html", [
      [ "Overview", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md23", null ],
      [ "Core Concepts", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md24", [
        [ "1. Modules", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md25", null ],
        [ "2. API Vtables", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md26", null ],
        [ "3. Dependency Injection", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md27", null ]
      ] ],
      [ "Module System Directory Structure", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md28", null ],
      [ "Example: Using the Module System", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md29", [
        [ "Define a Module", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md30", null ],
        [ "Initialize and Use", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md31", null ]
      ] ],
      [ "Traits vs Modules", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md32", null ],
      [ "Memory Safety", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md33", null ],
      [ "Status and Error Handling", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md34", null ],
      [ "Freestanding Compatibility", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md35", null ],
      [ "Documentation and Examples", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md36", null ],
      [ "Design Philosophy", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md37", null ],
      [ "Getting Started", "md__2home_2runner_2work_2libjuno_2libjuno_2include_2juno_2README.html#autotoc_md38", null ]
    ] ],
    [ "LibJuno Tutorial", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html", [
      [ "main.c", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md76", [
        [ "Logging", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md77", null ],
        [ "Time", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md78", null ],
        [ "Instantiating APIs", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md79", null ],
        [ "Failure Handler", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md80", null ],
        [ "The Entry Point", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md81", null ],
        [ "Dependency Injection in Action", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md82", null ],
        [ "Module Initialization", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md83", null ],
        [ "Runtime", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md84", null ]
      ] ],
      [ "engine_app.h", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md85", [
        [ "The Engine Application", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md86", null ]
      ] ],
      [ "Deriving an Application", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md87", null ],
      [ "The Application Structure", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md88", null ],
      [ "engine_cmd_msg.h", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md90", [
        [ "Engine Application Commands", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md91", null ],
        [ "Engine Command Pipe", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md92", null ]
      ] ],
      [ "engine_cmd_msg.c", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md93", [
        [ "Engine Command Pipe Implementation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md94", [
          [ "Pointer Copy", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md95", null ],
          [ "Pointer Reset", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md96", null ]
        ] ],
        [ "Pipe Api Assert", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md97", null ],
        [ "Pipe Init", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md98", null ],
        [ "Pipe Queue Implementation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md99", null ]
      ] ],
      [ "engine_app.c", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md100", [
        [ "Application Api", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md101", null ],
        [ "Verification Function", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md102", null ],
        [ "The Application Init Function", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md103", null ],
        [ "<tt>OnStart</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md104", null ],
        [ "<tt>OnProcess</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md105", null ],
        [ "<tt>OnExit</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md106", null ]
      ] ],
      [ "system_manager_app.c", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md107", null ],
      [ "Conclusion", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md108", null ]
    ] ],
    [ "Minimal_queue_example Docs", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2MINIMAL__QUEUE__EXAMPLE.html", [
      [ "The Queue Example", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2MINIMAL__QUEUE__EXAMPLE.html#autotoc_md120", null ]
    ] ],
    [ "LibJuno Examples", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html", [
      [ "Quick Start", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md122", null ],
      [ "Available Examples", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md123", [
        [ "1. Minimal Memory Example (<tt>minimal_memory_example.c</tt>)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md124", null ],
        [ "2. Minimal Queue Example (<tt>minimal_queue_example.c</tt>)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md126", null ],
        [ "3. State Machine Example (<tt>example_state_machine.c</tt>)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md128", null ],
        [ "4. Complete Tutorial Project (<tt>example_project/</tt>)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md130", null ]
      ] ],
      [ "Example Selection Guide", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md132", null ],
      [ "Using Examples as Templates", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md133", [
        [ "Code Generation Scripts", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md134", null ]
      ] ],
      [ "Example Code Style", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md135", null ],
      [ "Notes", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md136", null ],
      [ "Additional Resources", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md137", null ],
      [ "Questions?", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md138", null ]
    ] ],
    [ "Topics", "topics.html", "topics" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ]
      ] ]
    ] ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Variables", "functions_vars.html", "functions_vars" ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"group__juno__ds__array.html",
"juno__math__constants_8h_source.html",
"namespacejuno_1_1buff.html#a41121c4c2d402b6ef2db350fa82a1140",
"structJUNO__VEC4__I32__SPH__TAG.html#a54fe7a668e112a77fa5878da2ea2a7d3"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
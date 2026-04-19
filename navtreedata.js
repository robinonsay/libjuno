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
    [ "<strong>NEW</strong> <a href=\"https://marketplace.visualstudio.com/items?itemName=RobinOnsay.libjuno\" >VSCode Extension</a> — Navigate Dependency Injection Like Magic", "index.html#autotoc_md41", null ],
    [ "The \"Library of Everything\" Problem", "index.html#autotoc_md42", null ],
    [ "Core Philosophy", "index.html#autotoc_md43", [
      [ "Memory Safety, Pointers, and Arrays", "index.html#autotoc_md44", null ]
    ] ],
    [ "Templates and Scripts", "index.html#autotoc_md45", null ],
    [ "Tutorial", "index.html#autotoc_md46", null ],
    [ "Version and API Stability", "index.html#autotoc_md47", null ],
    [ "Using LibJuno", "index.html#autotoc_md48", [
      [ "Documentation", "index.html#autotoc_md49", [
        [ "Requirements Traceability Artifacts", "index.html#autotoc_md50", null ]
      ] ],
      [ "Dependencies", "index.html#autotoc_md51", null ],
      [ "Building and Testing", "index.html#autotoc_md52", [
        [ "CMake Build Options", "index.html#autotoc_md53", [
          [ "Freestanding Mode", "index.html#autotoc_md54", null ],
          [ "Development and Testing Workflow", "index.html#autotoc_md55", null ]
        ] ]
      ] ],
      [ "Installation", "index.html#autotoc_md56", null ],
      [ "Contributing", "index.html#autotoc_md57", null ],
      [ "Inspiration for the Name", "index.html#autotoc_md58", null ]
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
      [ "main.c", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md78", [
        [ "Logging", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md79", null ],
        [ "Time", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md80", null ],
        [ "Instantiating APIs", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md81", null ],
        [ "Failure Handler", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md82", null ],
        [ "The Entry Point", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md83", null ],
        [ "Dependency Injection in Action", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md84", null ],
        [ "Module Initialization", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md85", null ],
        [ "Runtime", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md86", null ]
      ] ],
      [ "engine_app.h", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md87", [
        [ "The Engine Application", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md88", null ]
      ] ],
      [ "Deriving an Application", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md89", null ],
      [ "The Application Structure", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md90", null ],
      [ "engine_cmd_msg.h", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md92", [
        [ "Engine Application Commands", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md93", null ],
        [ "Engine Command Pipe", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md94", null ]
      ] ],
      [ "engine_cmd_msg.c", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md95", [
        [ "Engine Command Pipe Implementation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md96", [
          [ "Pointer Copy", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md97", null ],
          [ "Pointer Reset", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md98", null ]
        ] ],
        [ "Pipe Api Assert", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md99", null ],
        [ "Pipe Init", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md100", null ],
        [ "Pipe Queue Implementation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md101", null ]
      ] ],
      [ "engine_app.c", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md102", [
        [ "Application Api", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md103", null ],
        [ "Verification Function", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md104", null ],
        [ "The Application Init Function", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md105", null ],
        [ "<tt>OnStart</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md106", null ],
        [ "<tt>OnProcess</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md107", null ],
        [ "<tt>OnExit</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md108", null ]
      ] ],
      [ "system_manager_app.c", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md109", null ],
      [ "Conclusion", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2example__project_2LibJuno__Tutorial.html#autotoc_md110", null ]
    ] ],
    [ "Minimal_queue_example Docs", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2MINIMAL__QUEUE__EXAMPLE.html", [
      [ "The Queue Example", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2MINIMAL__QUEUE__EXAMPLE.html#autotoc_md122", null ]
    ] ],
    [ "LibJuno Examples", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html", [
      [ "Quick Start", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md124", null ],
      [ "Available Examples", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md125", [
        [ "1. Minimal Memory Example (<tt>minimal_memory_example.c</tt>)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md126", null ],
        [ "2. Minimal Queue Example (<tt>minimal_queue_example.c</tt>)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md128", null ],
        [ "3. State Machine Example (<tt>example_state_machine.c</tt>)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md130", null ],
        [ "4. Complete Tutorial Project (<tt>example_project/</tt>)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md132", null ]
      ] ],
      [ "Example Selection Guide", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md134", null ],
      [ "Using Examples as Templates", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md135", [
        [ "Code Generation Scripts", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md136", null ]
      ] ],
      [ "Example Code Style", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md137", null ],
      [ "Notes", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md138", null ],
      [ "Additional Resources", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md139", null ],
      [ "Questions?", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2README.html#autotoc_md140", null ]
    ] ],
    [ "01-overview", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html", [
      [ "UDP Threads Example — Software Design Document", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md141", [
        [ "1. Overview", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md143", [
          [ "1.1 Requirements in Scope", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md144", null ]
        ] ],
        [ "2. Design Approach", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md146", [
          [ "2.1 Technology Stack", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md147", null ],
          [ "2.2 Architecture Overview", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md148", null ],
          [ "2.3 Module Dependency Graph", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md149", null ],
          [ "2.4 Design Constraints", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md150", null ],
          [ "2.5 Alternatives Considered", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md151", null ]
        ] ]
      ] ]
    ] ],
    [ "02-udp-module", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html", [
      [ "Section 3: UDP Socket Module Design", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md152", [
        [ "3.1 Purpose and Scope", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md154", null ],
        [ "3.2 Data Structures", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md156", [
          [ "3.2.1 <tt>UDP_THREAD_MSG_T</tt> — Message Structure", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md157", null ],
          [ "3.2.2 <tt>JUNO_UDP_CFG_T</tt> — Configuration Structure", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md159", null ],
          [ "3.2.3 <tt>JUNO_UDP_ROOT_T</tt> — Module Root Structure", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md161", null ],
          [ "3.2.4 <tt>JUNO_UDP_API_T</tt> — Vtable (Function Pointer Table)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md163", null ],
          [ "3.2.5 <tt>JUNO_UDP_LINUX_T</tt> — Linux Derivation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md165", null ],
          [ "3.2.6 <tt>JUNO_UDP_T</tt> — Module Union", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md167", null ]
        ] ],
        [ "3.3 Module Initialization", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md169", null ],
        [ "3.4 Vtable Operations — Algorithms", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md171", [
          [ "3.4.1 Open (REQ-UDP-003, REQ-UDP-004, REQ-UDP-005)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md172", null ],
          [ "3.4.2 Send (REQ-UDP-006, REQ-UDP-015)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md174", null ],
          [ "3.4.3 Receive (REQ-UDP-007, REQ-UDP-008, REQ-UDP-015)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md176", null ],
          [ "3.4.4 Close (REQ-UDP-009)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md178", null ]
        ] ],
        [ "3.5 Error Handling Strategy", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md180", null ],
        [ "3.6 Interface Header Constraints", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md182", [
          [ "Permitted includes in <tt>udp_api.h</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md183", null ],
          [ "Forbidden includes in <tt>udp_api.h</tt>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md184", null ],
          [ "Two-layer separation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md185", null ]
        ] ],
        [ "3.7 Memory Ownership", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_202-udp-module.html#autotoc_md187", null ]
      ] ]
    ] ],
    [ "03-thread-module", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html", [
      [ "Section 4: Thread Module Design", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md188", [
        [ "4.1 Purpose and Scope", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md189", null ],
        [ "4.2 Data Structures", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md191", [
          [ "4.2.1 <tt>JUNO_THREAD_ROOT_T</tt> — Module Root", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md192", null ],
          [ "4.2.2 <tt>JUNO_THREAD_API_T</tt> — Vtable", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md193", null ]
        ] ],
        [ "4.3 Module Initialization", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md195", null ],
        [ "4.4 Vtable Operations — Algorithms", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md197", [
          [ "4.4.1 Create (REQ-THREAD-003, REQ-THREAD-004, REQ-THREAD-015)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md198", null ],
          [ "4.4.2 Stop (REQ-THREAD-006, REQ-THREAD-007)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md199", null ],
          [ "4.4.3 Join (REQ-THREAD-005)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md200", null ]
        ] ],
        [ "4.5 Cooperative Shutdown Protocol", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md202", null ],
        [ "4.6 Error Handling Strategy", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md204", null ],
        [ "4.7 Interface Header Constraints", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md206", null ],
        [ "4.8 Memory Ownership", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_203-thread-module.html#autotoc_md208", null ]
      ] ]
    ] ],
    [ "04-applications", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html", [
      [ "Section 5: Application Designs", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md209", [
        [ "5.1 Overview", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md210", [
          [ "5.1.1 UDPTH_MSG_MID", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md211", null ]
        ] ],
        [ "5.2 SenderApp", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md213", [
          [ "5.2.1 Struct Definition", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md214", null ],
          [ "5.2.2 Init Function", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md215", null ],
          [ "5.2.3 Lifecycle Algorithms", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md216", null ]
        ] ],
        [ "5.3 MonitorApp", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md218", [
          [ "5.3.1 Struct Definition", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md219", null ],
          [ "5.3.2 Init Function", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md220", null ],
          [ "5.3.3 Lifecycle Algorithms", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md221", null ]
        ] ],
        [ "5.4 UdpBridgeApp", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md223", [
          [ "5.4.1 Struct Definition", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md224", null ],
          [ "5.4.2 Init Function", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md225", null ],
          [ "5.4.3 Lifecycle Algorithms", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md226", null ]
        ] ],
        [ "5.5 ProcessorApp", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md228", [
          [ "5.5.1 Struct Definition", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md229", null ],
          [ "5.5.2 Init Function", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md230", null ],
          [ "5.5.3 Lifecycle Algorithms", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md231", null ]
        ] ],
        [ "5.6 Memory Ownership", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_204-applications.html#autotoc_md233", null ]
      ] ]
    ] ],
    [ "05-composition-root", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html", [
      [ "Section 6: Composition Root Design", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md234", [
        [ "6.1 Overview", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md235", null ],
        [ "6.2 Static Module Inventory", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md237", null ],
        [ "6.3 Initialization Order", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md239", null ],
        [ "6.4 Scheduler Table Configuration", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md241", null ],
        [ "6.5 Thread Entry Functions", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md243", null ],
        [ "6.6 Thread Startup Sequence", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md245", null ],
        [ "6.7 Graceful Shutdown Sequence", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md247", null ],
        [ "6.8 main() Pseudo-code", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md249", null ],
        [ "6.9 Memory Ownership Summary", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md251", null ],
        [ "6.10 Module Dependency Diagram", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_205-composition-root.html#autotoc_md253", null ]
      ] ]
    ] ],
    [ "UDP Threads Example — Software Design Document", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_2index.html", null ],
    [ "UDP Module — Test Case Specifications", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html", [
      [ "Scope", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md257", null ],
      [ "Common Setup Notes", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md259", null ],
      [ "Test Cases: Module Initialization (REQ-UDP-016)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md261", [
        [ "TC-UDP-001 — Init with valid root and vtable succeeds", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md262", null ],
        [ "TC-UDP-002 — Init with null root returns error and invokes failure handler", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md264", null ],
        [ "TC-UDP-003 — Init with null vtable returns error and invokes failure handler", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md266", null ]
      ] ],
      [ "Test Cases: Open Operation (REQ-UDP-003)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md268", [
        [ "TC-UDP-004 — Open receiver socket with valid config succeeds", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md269", null ],
        [ "TC-UDP-005 — Open sender socket with valid remote config succeeds", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md271", null ],
        [ "TC-UDP-006 — Open with null root returns error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md273", null ],
        [ "TC-UDP-007 — Double open on already-open socket returns error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md275", null ]
      ] ],
      [ "Test Cases: Receiver Bind / Sender Connect (REQ-UDP-004, REQ-UDP-005)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md277", [
        [ "TC-UDP-008 — Receiver socket binds to local port (integration)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md278", null ],
        [ "TC-UDP-009 — Sender socket connects to remote address:port (integration)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md280", null ]
      ] ],
      [ "Test Cases: Send Operation (REQ-UDP-006, REQ-UDP-015)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md282", [
        [ "TC-UDP-010 — Send one message; receiver gets exactly sizeof(UDP_THREAD_MSG_T) bytes", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md283", null ],
        [ "TC-UDP-011 — Send with null message pointer returns error and invokes failure handler", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md285", null ]
      ] ],
      [ "Test Cases: Receive Operation (REQ-UDP-007, REQ-UDP-008, REQ-UDP-015)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md287", [
        [ "TC-UDP-012 — Receive with datagram available returns success and populates output", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md288", null ],
        [ "TC-UDP-013 — Receive times out when no sender is present", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md290", null ],
        [ "TC-UDP-014 — Receive with null output pointer returns error and invokes failure handler", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md292", null ]
      ] ],
      [ "Test Cases: Close Operation (REQ-UDP-009)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md294", [
        [ "TC-UDP-015 — Close open socket succeeds and resets descriptor to invalid sentinel", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md295", null ],
        [ "TC-UDP-016 — Close already-closed socket returns error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md297", null ]
      ] ],
      [ "Test Cases: Error Status Return and Failure Handler (REQ-UDP-012, REQ-UDP-013)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md299", [
        [ "TC-UDP-017 — Every API operation returns a JUNO_STATUS_T value (not void)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md300", null ],
        [ "TC-UDP-018 — Failed open due to bad address invokes failure handler with non-null user data", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md302", null ],
        [ "TC-UDP-019 — Sequence number wraps at UINT32_MAX", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md304", null ],
        [ "TC-UDP-020 — OS socket creation failure propagates error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_201-udp-module.html#autotoc_md306", null ]
      ] ]
    ] ],
    [ "UDP Threads — Thread Module Test Cases", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html", [
      [ "Module Initialization", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md309", [
        [ "TC-THREAD-001 — Init with valid root and vtable succeeds", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md310", null ],
        [ "TC-THREAD-002 — Init with null root invokes failure handler and returns error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md312", null ],
        [ "TC-THREAD-003 — Init with null vtable invokes failure handler and returns error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md314", null ]
      ] ],
      [ "Create Operation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md316", [
        [ "TC-THREAD-004 — Create with valid entry function starts the thread", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md317", null ],
        [ "TC-THREAD-005 — Create with null entry function returns error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md319", null ],
        [ "TC-THREAD-006 — Create on already-running root returns error (single thread per root)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md321", null ]
      ] ],
      [ "Stop Operation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md323", [
        [ "TC-THREAD-007 — Stop sets the stop flag; entry function observes it and exits", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md324", null ],
        [ "TC-THREAD-008 — Stop on un-created root returns error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md326", null ]
      ] ],
      [ "Join Operation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md328", [
        [ "TC-THREAD-009 — After Stop, Join blocks until thread exits and returns success", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md329", null ],
        [ "TC-THREAD-010 — Join on un-created root returns error", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md331", null ]
      ] ],
      [ "Error Status and Failure Handler", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md333", [
        [ "TC-THREAD-011 — Failed pthread_create causes failure handler invocation", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md334", null ],
        [ "TC-THREAD-012 — All error-returning operations return JUNO_STATUS_T (coverage roll-up)", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md336", null ]
      ] ],
      [ "</blockquote>", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md337", null ],
      [ "Test Double Reference", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_202-thread-module.html#autotoc_md338", null ]
    ] ],
    [ "UDP Threads — Integration Test Cases", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html", [
      [ "SenderApp Tests", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md341", [
        [ "TC-INT-001 — SenderApp OnStart opens the UDP sender socket with the correct config", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md342", null ],
        [ "TC-INT-002 — SenderApp OnProcess publishes to broker and transmits via UDP in one cycle", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md344", null ],
        [ "TC-INT-003 — SenderApp OnProcess increments sequence number monotonically across multiple cycles", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md346", null ],
        [ "TC-INT-004 — SenderApp OnExit closes the UDP sender socket exactly once", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md348", null ]
      ] ],
      [ "MonitorApp Tests", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md350", [
        [ "TC-INT-005 — MonitorApp OnStart registers a subscription for UDPTH_MSG_MID", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md351", null ],
        [ "TC-INT-006 — MonitorApp OnProcess dequeues and processes an available message", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md353", null ],
        [ "TC-INT-007 — MonitorApp OnProcess with empty pipe returns normally without blocking", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md355", null ]
      ] ],
      [ "UdpBridgeApp Tests", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md357", [
        [ "TC-INT-008 — UdpBridgeApp OnStart opens the UDP receiver socket bound to port 9000", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md358", null ],
        [ "TC-INT-009 — UdpBridgeApp OnProcess receives a datagram and publishes it to broker", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md360", null ],
        [ "TC-INT-010 — UdpBridgeApp OnProcess with receive timeout does not publish to broker", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md362", null ],
        [ "TC-INT-011 — UdpBridgeApp OnExit closes the UDP receiver socket exactly once", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md364", null ]
      ] ],
      [ "ProcessorApp Tests", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md366", [
        [ "TC-INT-012 — ProcessorApp OnStart registers a subscription for UDPTH_MSG_MID on Thread 2 broker", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md367", null ],
        [ "TC-INT-013 — ProcessorApp OnProcess dequeues and processes an available message", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md369", null ]
      ] ],
      [ "Composition Root Tests", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md371", [
        [ "TC-INT-014 — Composition root assigns correct applications to each thread's scheduler", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md372", null ],
        [ "TC-INT-015 — Composition root gracefully shuts down both threads without deadlock", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md374", null ]
      ] ],
      [ "Test Double Reference", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md376", null ],
      [ "Requirement Coverage Summary", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_203-integration.html#autotoc_md378", null ]
    ] ],
    [ "UDP Threads Example — Test Case Index", "md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2test-cases_2index.html", null ],
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
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
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
"globals_func_j.html",
"juno__buff_8hpp.html#a98eca69a6a189d91f31dfbbbe5d18201",
"md__2home_2runner_2work_2libjuno_2libjuno_2examples_2udp-threads_2docs_2design_201-overview.html#autotoc_md146",
"namespacejuno_1_1buff.html#acc50966045668aecf2102a642a6cc113",
"structJUNO__VEC3__I32__SPH__TAG.html#ab6c2550fa542700509dc21ab8e4c1b03",
"udp__bridge__app_8cpp.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
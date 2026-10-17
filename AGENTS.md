# AGENTS.md

## Project Overview

This is a C++20 CLI application that converts a JSON file into a TypeScript interface/object representation.

The application is called `json2ts`.

The implementation is split by concern under `src/`, with declarations under `include/json2ts/`:

- `src/main.cpp` — thin driver; forwards `argc`/`argv` to `json2ts::cli::run`.
- `src/cli.cpp` — argument parsing, conversion orchestration, file I/O.
- `src/parser.cpp` — strips JSON comments and extracts them for JSDoc mapping.
- `src/generator.cpp` — walks the JSON value and renders TypeScript interfaces.
- `src/utils.cpp` — identifier and string helpers.

The project uses CMake for building and includes the single-header version of `nlohmann::json` under "include/".

The "test/" directory contains JSON fixtures covering the supported input shapes and edge cases, plus an automated TypeScript test suite.

### Repository Structure

```text
.
├── CMakeLists.txt       # CMake build configuration (globs src/*.cpp)
├── Makefile             # Build and test convenience targets
├── README.md            # User/developer documentation
├── AGENTS.md            # Instructions for AI agents (this file)
├── LICENSE              # GPL-3.0-only license text
├── .gitignore           # Ignores build outputs and generated test files
├── build/               # Build output (gitignored)
│   └── json2ts          # Built executable
├── include/
│   ├── json2ts/         # Internal header files
│   │   ├── cli.h        # CLI entry points
│   │   ├── generator.h  # TypeScript generation functions
│   │   ├── parser.h     # Comment preprocessing functions
│   │   ├── types.h      # Shared structs and aliases
│   │   └── utils.h      # String/identifier helpers
│   └── nlohmann/
│       └── json.hpp     # Vendored nlohmann/json dependency
├── src/                 # Source files
│   ├── main.cpp         # Main driver for CLI app
│   ├── cli.cpp          # CLI implementation
│   ├── parser.cpp       # Comment stripping and extraction
│   ├── generator.cpp    # TypeScript interface rendering
│   └── utils.cpp        # Helper implementations
└── test/                # Test fixtures and automated test suite
    ├── cases/           # Input JSON fixtures
    ├── lib/             # Test helpers (schema building, validation)
    ├── runner.ts        # Automated test runner
    ├── build_test.sh    # Generates test/expect/*.ts from cases
    ├── README.md        # Fixture case list
    ├── tsconfig.json    # TypeScript configuration for the test suite
    └── package.json     # npm dependencies (tsx, typescript)
```

### Requirements

- C++20 (`g++`)
- CMake 3.14
- GNU Make (convenience targets in `Makefile`)
- TypeScript (`package.json` file is inside "test/")
- Node.js v22 or later with npm included

## Architecture

Data flow for a single run:

1. `json2ts::cli::run` (in `src/cli.cpp`) parses `argv` via `parse_args`, then calls `process_file`.
2. `json2ts::parser::preprocess_json` strips `//` and `/* */` comments and records them as `ExtractedComment` (text plus optional key).
3. `nlohmann::json::parse` parses the cleaned text; `process_file` rejects any root that is not an object.
4. `json2ts::parser::build_comments_by_key` builds the `UnorderedCommentsMap` used later for JSDoc lookup.
5. The interface name is resolved (`--name` as-is, else single root key, else filename stem) and sanitized.
6. `json2ts::generator::build_interfaces_recursive` renders definitions depth-first into `std::vector<InterfaceDef>`, appending each parent after its nested definitions. It returns the index of the definition it appended.
7. `process_file` resolves the output path, writes definitions de-duplicated by name, and prints `Generated: <path>`.

Shared types live in `include/json2ts/types.h` (`ExtractedComment`, `InterfaceDef`, `UnorderedCommentsMap`).

Conversion logic (`parser`, `generator`, `utils`) must stay independent from CLI concerns where practical.

## Code Conventions

The current implementation is intentionally small.

DO NOT introduce additional classes, directories, abstractions, or source files unless they make the code substantially clearer or are required by the feature.

Keep `src/main.cpp` a thin driver. If a component becomes too large, separate concerns into focused components rather than creating a large collection of generic utility classes.

Keep conversion logic independent from CLI concerns where practical.

### C++ Code Conventions

- Always use standard C++20 (`-std=c++20`).
- Follow modern C++ practices.
- Prefer RAII and standard-library facilities.
- Preserve `const` correctness.
- Avoid unnecessary dynamic allocation.
- Do not introduce a dependency when the C++ standard library is sufficient.
- Keep the implementation simple and appropriate for a small CLI utility.
- Keep the SPDX header in every source and header file:

  ```cpp
  // SPDX-FileCopyrightText: <copyright holder>
  // SPDX-License-Identifier: GPL-3.0-only
  ```

- Internal headers use `JSON2TS_<NAME>_H` include guards and are included as `<json2ts/<name>.h>`.
- Keep namespaces consistent: `json2ts::cli`, `json2ts::parser`, `json2ts::generator`, `json2ts::utils`.

### CMake

Keep the CMake configuration simple and target-oriented.

Prefer target-specific configuration such as:

```text
target_include_directories(...)
target_compile_features(...)
target_link_libraries(...)
```

`CMakeLists.txt` collects `src/*.cpp` with `GLOB_RECURSE ... CONFIGURE_DEPENDS`, so a new source file is picked up automatically on the next configure; there is no source list to maintain.

### Documentation

Update `README.md` when changing user-visible behavior, including:

- CLI syntax
- supported input
- generated TypeScript format
- error behavior
- new supported JSON structures

Keep documentation examples consistent with the actual implementation.

Verify examples by building and running the binary; do not edit examples from memory.

When adding a fixture, keep the case list in `test/README.md` up to date.

### JSON Library

The project uses the vendored single-header `nlohmann::json` library.

```cpp
#include <nlohmann/json.hpp>
```

DO NOT replace or upgrade the JSON library unless explicitly requested.

DO NOT introduce another JSON parser.

### JSON Semantics

Be careful distinguish the JSON types:

- object
- array
- string
- number
- boolean
- null

Nested objects and arrays should preserve their structure in the generated TypeScript representation.

Verified semantics to preserve:

- The default `nlohmann::json` object is ordered (`std::map`), so properties are emitted in alphabetical key order. This is deterministic; do not "fix" it to input order without discussing.
- `null` maps to TypeScript `null`, not `any`.
- Arrays of primitives use `<type>[]`; mixed primitive arrays produce a union in first-seen element order, e.g. `(number | boolean | null | string)[]`.
- Arrays of objects generate the interface from the first element only; remaining elements are assumed to share the shape.
- Empty arrays generate `any[]`. This is the only accepted use of `any`; DO NOT introduce other implicit `any` output.
- Identifiers are sanitized by `json2ts::utils::sanitize_identifier` (invalid characters become `_`, a leading digit is prefixed with `_`).

DO NOT silently change the interpretation of existing JSON types.

DO NOT silently use `any` type, logically there is no reason to do that.

If behavior for an ambiguous case is changed, update the documentation and relevant test fixtures.

## CLI

The application is a command-line tool.

When modifying CLI behavior:

- Preserve existing command-line behavior unless the task explicitly requires a breaking change.
- Provide useful error messages for invalid input.
- Handle missing files and invalid JSON gracefully.
- DO NOT silently ignore parsing errors.
- Keep CLI parsing and conversion logic reasonably separated as the code grows.

Current option behavior (keep `README.md` in sync):

- `--dts` is the default extension; `--ts` selects `.ts` and takes precedence when both are given. There is no mutual-exclusion validation.
- `--output` enforces the chosen extension: appended when the filename has none, replaced when it differs.
- `--name` is used as-is after `sanitize_identifier`; derived names (single root key or filename stem) are PascalCased; empty results fall back to `Root`.
- The `--name` branch must set `has_custom_name`, which drives the `is_interface_name_from_cli` branch in `process_file`. Dropping this assignment silently PascalCases CLI names (regression fixed in `0201807`).
- Options given without a value fall through to the `Unknown option` error path. If touched, improve the message instead of replicating the quirk.
- `use_dts` is currently parsed but unused; only `use_ts` is consulted when choosing the extension.

## Build

Use an out-of-source CMake build when possible.

Command to build the project:

```bash
make configure
make build
```

Or compile directly (for development only):

```bash
g++ -std=c++20 -Wall -Iinclude src/*.cpp -o json2ts
```

The resulting executable is `json2ts`.

If the repository already contains a preferred build command, follow that command instead.

DO NOT manually modify generated files or build artifacts.

## TypeScript Generation

Generated TypeScript should be deterministic.

Given the same JSON input, the converter should produce the same output every time.

Avoid introducing output that depends on:

- memory addresses,
- unordered iteration,
- timestamps,
- random values,
- environment-specific state.

Additional format rules:

- One `export interface` per object; nested interfaces are emitted before their parent, root last.
- Nested interface names are the parent name plus the PascalCased property name (e.g. `MyTeams` + `teams` → `MyTeamsTeams`).
- Properties are sorted alphabetically (see JSON Semantics).
- The file ends with a trailing newline.

When changing the generated format, update `README.md` and the relevant test documentation if the change is user-visible.

## Testing

The files under "test/" are input fixtures for the converter.

Full flow:

```bash
make configure-test   # npm install inside test/
make build-test       # build/json2ts over test/cases/*.json -> test/expect/*.ts
make test             # tsx test/runner.ts validates JSON inputs against schemas
```

`make test` errors when `make configure-test` or `make build-test` has not run first.

- `test/build_test.sh` runs the built binary for every fixture with `--ts`.
- `test/runner.ts` (with `test/lib/`) converts each generated `.ts` into a class with a runtime schema and validates the original JSON against it.
- `test/expect/` and `test/expect_classes/` are generated and gitignored. DO NOT commit them.

When changing conversion behavior:

- Identify which existing fixture is affected.
- The fixture should include edge-case.
- Run the converter against the fixture.
- Verify that the generated TypeScript is correct.
- Add a new fixture when the behavior represents a new supported case.
- At minimum, verify changes against the relevant files in "test/".

DO NOT modify existing fixtures merely to make a failing implementation pass. First determine whether the implementation or the expected behavior is incorrect.

When adding a fixture, use a descriptive filename that explains the behavior being tested.

Add the new case to the list in `test/README.md`.

## Git and Changes

Make the smallest change necessary.

DO NOT modify unrelated files.

DO NOT delete files (especially the core codebase) no matter what. If you want to delete something, PROMPT and discuss with the user first.

DO NOT remove existing fixtures without understanding their purpose.

DO NOT rewrite Git history (e.g., rebasing/editing commits).

DO NOT create commits unless explicitly requested.

DO NOT overwrite existing user changes.

This repository uses Conventional Commits in its history (e.g. `feat:`, `fix:`, `refactor:`, `docs:`, `build:`); follow the same style when asked to write a commit message.

Before finishing, review the final diff for accidental changes.

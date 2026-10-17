# json2ts

[![Build & Test](https://github.com/mitsuki31/json2ts/actions/workflows/build.yml/badge.svg)](https://github.com/mitsuki31/json2ts/actions/workflows/build.yml)
[![Release](https://github.com/mitsuki31/json2ts/actions/workflows/release.yml/badge.svg)](https://github.com/mitsuki31/json2ts/actions/workflows/release.yml)
[![License: GPL-3.0-only](https://img.shields.io/badge/License-GPL--3.0--only-blue)](./LICENSE)
<br/>
![C++ standard: 20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white)
![CMake 3.14+](https://img.shields.io/badge/CMake-3.14%2B-064F8C?logo=cmake&logoColor=white)
![Node.js 22+ (test suite only)](https://img.shields.io/badge/Node.js-22%2B-5FA04E?logo=nodedotjs&logoColor=white)

![Supported platforms: Linux, macOS, Windows](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows-orange?style=for-the-badge&logo=linux&logoColor=white)

Convert a JSON file into a TypeScript `.d.ts` or `.ts` file as an interface object.

`json2ts` reads a JSON document and generates TypeScript interfaces from it:

- nested objects become nested interfaces,
- arrays of objects become arrays of a generated interface,
- arrays of mixed primitives become union types,
- `//` and `/* */` comments in the JSON can be carried over as JSDoc.

The generated output is deterministic: the same input always produces the same output.

## Requirements

- C++20 compiler (`g++` or Clang)
- CMake 3.14 or later
- GNU Make
- Node.js v22 or later with npm included (only needed to run the test suite)

## Supported JSON Top Level

This project follows the standard [**RFC 4627 (2006)**](https://www.rfc-editor.org/info/rfc4627/), which only allows the top level to be either an object or an array.

> [!NOTE]
> Currently the top level array is not supported yet.

| Type | Supported | Example |
| ---- | :-------: | ------- |
| object | ✅ | `{ ... }` |
| array | ❌ | `[ ... ]` |
| string | ❌ | `"This is foo"` |
| number | ❌ | `404` |
| boolean | ❌ | `true` / `false` |
| null | ❌ | `null` |

## Build

Out-of-source build (preferred):

```bash
make configure
make build
```

The executable file will be named `json2ts` inside `"build/"` directory.

Or compile directly (for development only):

```bash
g++ -std=c++20 -Wall -Iinclude src/*.cpp -o json2ts
```

## Usage

```bash
json2ts <input.json>
    [--output <file>]
    [--name <Name>]
    [--no-comments]
    [--dts | --ts]
    [--help]
```

| Option | Description |
| ------ | ----------- |
| `<input.json>` | Input JSON file (required) |
| `--dts` | Output `.d.ts` extension (default) |
| `--ts` | Output `.ts` extension (takes precedence if both are given) |
| `--output <file>` | Output file (extension enforced/replaced) |
| `--name <Name>` | Interface name (default: single root key, or filename stem, or `Root`) |
| `--no-comments` | Exclude JSON comments from generated output |
| `--help` | Show help message |

On success, the tool prints the generated path as `Generated: <path>`.

### Output file

- Without `--output`, the generated file is written next to the input file using the same stem and the chosen extension: `data.json` → `data.d.ts`.
- With `--output`, the chosen extension is enforced: it is appended when the filename has no extension, and replaced when the filename has a different one.

### Interface name

- `--name` is used as-is after replacing characters that are not valid in a TypeScript identifier with `_`: `--name my-teams` → `my_teams`.
- Without `--name`, the name comes from the single root key when the root object has exactly one key, otherwise from the input filename stem. Derived names are converted to PascalCase: `single_key_root.json` → `SingleKeyRoot`.
- Names that would end up empty (or only `_`) fall back to `Root`.

## Example

Assume you have a JSON data as below:

```json
{
  "teams": [
    {
      "name": "Tiger Team",
      "id": 12001,
      "is_active": true
    },
    {
      "name": "Pineapple Team",
      "id": 11005,
      "is_active": false
    }
  ]
}
```

```bash
json2ts data.json --dts --output data.d.ts --name MyTeams
```

**Output:**

```ts
export interface MyTeamsTeams {
  id: number;
  is_active: boolean;
  name: string;
}
export interface MyTeams {
  teams: MyTeamsTeams[];
}
```

Note that properties are emitted alphabetically and nested interfaces are written before the interface that references them. See [Supported Input](#supported-input) for details.

## Supported Input

- JSON root must be an object (`{"key": ...}`). Arrays or primitives at root will fail.
- Supported JSON types: `object`, `array`, `string`, `number`, `boolean`, `null`.
- Nested objects and arrays preserve their structure in the generated TypeScript representation.
- Object keys are ordered alphabetically in the output. The ordering is deterministic, but it does not preserve the key order of the input file.
- Arrays of primitives use `<type>[]`. Arrays with mixed element types generate a union: `(number | boolean | null | string)[]`.
- Arrays of objects generate a nested interface from the first element; later elements are assumed to have the same shape.
- Empty arrays have no element type to infer and are generated as `any[]`. This is the only case where `any` is used.
- Properties whose names are not valid TypeScript identifiers are sanitized: invalid characters are replaced with `_`, and a leading digit is prefixed with `_`.

## JSON Comments

`//` line comments and `/* */` block comments in the input JSON are stripped before parsing, so commented fixtures still parse as JSON.

By default, extracted comments are mapped to JSDoc best-effort:

- All comments found in the file are emitted once as a JSDoc block at the top of the generated file (duplicates removed).
- A comment is attached to the nearest following property key and emitted as JSDoc above that property. A comment attached to the key that becomes the interface name can also be emitted above that interface.
- Use `--no-comments` to suppress JSDoc output. The comments are still stripped for parsing; only the generated JSDoc is omitted.

## Generated TypeScript Format

Generated output is deterministic: given the same JSON input, the output is identical.

- Every object becomes an `export interface <Name> { ... }` definition.
- Nested interface names are built from the parent name plus the PascalCased property name: `MyTeams` + `teams` → `MyTeamsTeams`.
- Nested interfaces are emitted before their parent interface, so the root interface is written last.
- Properties are sorted alphabetically and the file ends with a trailing newline.

## Error Behavior

- Missing input file: `Failed to open input file: <file>` and non-zero exit.
- Invalid JSON: parsing error message and non-zero exit.
- Root not an object: error message and non-zero exit.
- Unknown options, or an option given without its value, are rejected with an error message and non-zero exit.
- Output file that cannot be opened: error message and non-zero exit.
- `--dts` and `--ts` are not mutually exclusive: `.d.ts` is the default, and `--ts` takes precedence when both are given.

## Testing

Tests are automated by the test runner and require Node.js v22 or later:

```bash
make configure-test   # install test dependencies (npm install)
make build-test       # convert test/cases/*.json into test/expect/*.ts
make test             # validate every JSON input against its generated schema
```

`make build-test` runs the converter over every fixture in `test/cases/`, and `make test` loads the generated interfaces, checks their runtime schemas against the original JSON values, and reports a per-case result.

See [test/README.md](./test/README.md) for the list of cases.

## Development

### Repository layout

```text
src/       # C++ sources, one module per concern (cli, parser, generator, utils)
include/   # internal headers under include/json2ts/ and the vendored nlohmann/json
test/      # JSON fixtures, the TypeScript test harness, and the runner
build/     # CMake build output (gitignored)
```

### How it works

1. `src/main.cpp` hands `argv` to `json2ts::cli::run`.
2. `src/cli.cpp` parses the options and coordinates the conversion.
3. `src/parser.cpp` strips comments from the raw JSON, extracts them, and links them to keys.
4. `nlohmann::json` parses the cleaned document; the root must be an object.
5. `src/generator.cpp` walks the value recursively and renders interfaces, nested ones first.
6. `src/cli.cpp` resolves the output path, de-duplicates interfaces by name, writes the file, and prints the generated path.

### Adding a test case

1. Add `test/cases/<descriptive_name>.json` (comments are allowed).
2. Run `make build-test` to generate `test/expect/<descriptive_name>.ts`.
3. Run `make test` and check that the new case passes.
4. Add the case to the list in `test/README.md`.

---

## License

Licensed under the GNU General Public License v3.0 **only** (not "or later").
See [LICENSE](./LICENSE) or <https://www.gnu.org/licenses/gpl-3.0.html>.

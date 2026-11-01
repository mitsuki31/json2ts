# Tests (expected outputs)

Input fixtures and the automated test suite for `json2ts`.

## Running the tests

From the repository root:

```bash
make configure-test   # install the Node.js dependencies (npm install)
make build-test       # convert every fixture in test/cases/ into test/expect/
make test             # validate each JSON fixture against its generated schema
```

The suite requires Node.js v22 or later and a built `json2ts` binary (`make build`). `make test` fails when `make configure-test` or `make build-test` has not run first.

## How the suite works

1. `build_test.sh` runs the built binary over every `cases/*.json` with `--ts` and writes the generated definitions to `expect/*.ts`.
2. `runner.ts` reads each JSON fixture (with its comments stripped) and turns every generated interface into a class carrying a runtime schema. Root type aliases become runtime array schemas.
3. `lib/validate.ts` walks the JSON value and asserts it matches the schema. Array schemas accept any allowed element type for every element. Each case prints `✓` on success, or `✗` with the failing path on failure.

`expect/` and `expect_classes/` are generated at test time and are gitignored. `lib/` contains the harness helpers (`json.ts`, `modulegen.ts`, `validate.ts`, `types.d.ts`).

## Cases

1. `basic` — primitives (string, number, boolean)
2. `comments` — `//` line and `/* */` block comments mapped to JSDoc
3. `nested` — nested object + array of primitives
4. `array_primitive` — array of mixed primitives (union type: `(number | boolean | null | string)[]`)
5. `array_object` — array of objects (recursive interface)
6. `empty_object` — empty `{}` root
7. `single_key_root` — root object with a single key (`Data`)
8. `null_and_bool` — null, boolean, number values
9. `array_root_object` — top-level array of objects with a nested object
10. `array_root_deep` — top-level array object containing arrays of objects
11. `array_root_primitive` — top-level array of strings
12. `array_root_mixed` — top-level mixed-primitive array with repeated types
13. `array_root_mixed_object_primitive` — top-level mixed object and primitive array
14. `array_root_empty` — top-level empty array (`any[]`)
15. `array_root_nested` — top-level array of number arrays
16. `array_root_comments` — comments inside top-level array elements
17. `array_root_single_item` — one-element top-level array uses the filename stem for its name

## Adding a case

1. Add `cases/<descriptive_name>.json` (comments are allowed).
2. Run `make build-test` and `make test`.
3. Add the case to the list above.

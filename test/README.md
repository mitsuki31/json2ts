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

1. `build_test.sh` runs the built binary over every `cases/*.json` with `--ts` and writes the generated interfaces to `expect/*.ts`.
2. `runner.ts` reads each JSON fixture (with its comments stripped) and turns every generated interface into a class carrying a runtime schema.
3. `lib/validate.ts` walks the JSON value and asserts it matches the schema. Each case prints `✓` on success, or `✗` with the failing path on failure.

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

## Adding a case

1. Add `cases/<descriptive_name>.json` (comments are allowed).
2. Run `make build-test` and `make test`.
3. Add the case to the list above.

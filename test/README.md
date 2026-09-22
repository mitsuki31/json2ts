# Manual tests (expected outputs)

Run the CLI against each `.json` and compare with the `.d.ts` file.
No automation — visual/manual verification only.

## Cases

1. `basic` — primitives (string, number, boolean)
2. `comments` — `//` line and `/* */` block comments mapped to JSDoc
3. `nested` — nested object + array of primitives
4. `array_primitive` — array of numbers
5. `array_object` — array of objects (recursive interface)
6. `empty_object` — empty `{}` root
7. `single_key_root` — root object with a single key (`Data`)
8. `null_and_bool` — null, boolean, number values

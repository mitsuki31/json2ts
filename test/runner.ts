/// <reference types="node" />
/// <reference path="./lib/types.d.ts" />

import fs from 'node:fs/promises';
import path from 'node:path';
import { pathToFileURL } from 'node:url';

import { readJSONFile } from './lib/json';
import { buildExpectedClassModule } from './lib/modulegen';
import { validateJSON } from './lib/validate';

async function glob(globPath: string): Promise<readonly string[]> {
  const files: string[] = [];

  for await (const file of fs.glob(globPath)) {
    files.push(file);
  }

  return files;
}

async function main(): Promise<void> {
  const TEST_CASES_DIR = path.resolve(import.meta.dirname, 'cases');
  const TEST_EXPECT_DIR = path.resolve(TEST_CASES_DIR, '..', 'expect');
  const TEST_EXPECT_CLASSES_DIR = path.resolve(TEST_CASES_DIR, '..', 'expect_classes');

  const JSON_FILES = await glob(path.join(TEST_CASES_DIR, '*.json'));

  for (const testFile of JSON_FILES) {
    const filename = path.basename(testFile, '.json');
    const expectedFile = path.join(TEST_EXPECT_DIR, `${filename}.ts`);
    const expectedClassFile = path.join(TEST_EXPECT_CLASSES_DIR, `${filename}.ts`);

    try {
      const jsonValue = await readJSONFile(testFile);

      const classes = await buildExpectedClassModule(
        expectedFile,
        expectedClassFile,
      );
      const firstClass = classes[classes.length - 1];

      const expectedModule: GeneratedModule =
        await import(pathToFileURL(expectedClassFile).href);
      const rootClass = expectedModule.root;

      if (
        typeof expectedModule.root !== 'function'
          || !('schema' in rootClass)
      ) {
        throw new TypeError(
          `No runtime schema found in ${expectedClassFile}`,
        );
      }

      validateJSON(jsonValue, rootClass, firstClass);
      console.log(`\x1b[1;32m✓\x1b[0m ${filename}`);
    } catch (error) {
      console.error(`\x1b[1;31m✗\x1b[0m ${filename}`);

      if (error instanceof Error) {
        console.error(`  ${error.message}`);
        console.trace(error);
      } else {
        console.error(`  ${String(error)}`);
      }

      process.exitCode = 1;
    }
  }
}

main();

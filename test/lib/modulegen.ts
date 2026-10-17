/// <reference types="node" />

import fs from 'node:fs/promises';
import path from 'node:path';
import { JSON_PRIMITIVE_TYPES } from './validate';

const INTERFACE_RE = /export\s+interface\s+([A-Za-z_$][\w$]*)\s*\{([\s\S]*?)\}/g;
const FIELD_RE = /^\s*([A-Za-z_$][\w$]*)\s*:\s*([^;]+);/gm;


function parseInterfaces(source: string): InterfaceDefinition[] {
  const interfaces: InterfaceDefinition[] = [];

  for (const match of source.matchAll(INTERFACE_RE)) {
    const [, name, body] = match;
    const fields: Field[] = [];

    for (const field of body.matchAll(FIELD_RE)) {
      const [, fieldName, fieldType] = field;

      fields.push({ name: fieldName, type: fieldType.trim() });
    }

    interfaces.push({ name, fields });
  }

  return interfaces;
}

function schemaValue(type: string): string {
  if (type.endsWith('[]')) {
    const typeTrunc = type.slice(0, -2);

    if (/^\(.+\)$/.test(typeTrunc)) {
      const types = [...new Set(typeTrunc.slice(1, -1).split('|'))]
        .map(t => schemaValue(t.trim()))
        .map(t => JSON_PRIMITIVE_TYPES.includes(t as any) ? `"${t}"` : t);
      return `[${types.join(', ')}]`;
    }
    return `[${schemaValue(typeTrunc)}]`;
  }

  switch (type as JSONPrimitiveValueName | "any") {
    case 'string':
    case 'number':
    case 'boolean':
    case 'null':
    case 'any':
      return JSON.stringify(type);

    default:
      return type;
  }
}

function generateClassModule(source: string): readonly [string, readonly string[]] {
  const interfaces = parseInterfaces(source);
  const output: string[] = [];

  const firstClass: string = interfaces[interfaces.length - 1].name;

  for (const definition of interfaces) {
    output.push(`export class ${definition.name} {`);

    for (const field of definition.fields) {
      output.push(`  ${field.name}!: ${field.type};`);
    }

    output.push('');
    output.push('  static readonly schema = {');

    for (const field of definition.fields) {
      output.push(
        `    ${field.name}: ${schemaValue(field.type)},`
      );
    }

    output.push('  } as const;');
    output.push('}');
    if (firstClass === definition.name) {
      // Expose the first class as `root` constant
      output.push(`export const root = ${firstClass};`);
    }
    output.push('');
  }

  return [output.join('\n'), interfaces.map(x => x.name)];
}

export async function buildExpectedClassModule(
  sourceFile: string,
  outputFile: string,
): Promise<readonly string[]> {
  const source = await fs.readFile(sourceFile, 'utf-8');
  const [generated, classes] = generateClassModule(source);

  await fs.mkdir(path.dirname(outputFile), {
    recursive: true,
  });

  await fs.writeFile(outputFile, generated, 'utf-8');
  return classes;
}

import fs from 'node:fs/promises';

export function stripJSONComments(source: string): string {
  let output = '';
  let inString = false;
  let escaped = false;

  for (let i = 0; i < source.length; i++) {
    const char = source[i];
    const next = source[i + 1];

    if (inString) {
      output += char;

      if (escaped) {
        escaped = false;
      } else if (char === '\\') {
        escaped = true;
      } else if (char === '"') {
        inString = false;
      }

      continue;
    }

    if (char === '"') {
      inString = true;
      output += char;
      continue;
    }

    if (char === '/' && next === '/') {
      i += 2;

      while (i < source.length && source[i] !== '\n') {
        i++;
      }

      if (i < source.length) output += '\n';

      continue;
    }

    if (char === '/' && next === '*') {
      i += 2;

      while (i + 1 < source.length) {
        if (source[i] === '*' && source[i + 1] === '/') {
          i++;
          break;
        }

        i++;
      }

      continue;
    }

    output += char;
  }

  return output;
}

export async function readJSONFile(file: string): Promise<unknown> {
  const source = await fs.readFile(file, 'utf-8');
  return JSON.parse(stripJSONComments(source));
}

/// <reference path="./types.d.ts" />

export const JSON_PRIMITIVE_TYPES = [
  "string",
  "number",
  "boolean",
  "null"
] as const satisfies readonly JSONPrimitiveValueName[];
export const JSON_UNKNOWN_PRIMITIVE_TYPES = [
  "function",
  "bigint",
  "undefined",
  "symbol"
] as const satisfies readonly JSONUnsupportedValueName[];


function getRuntimeType(
  value: unknown
): JSONPrimitiveValueName | "object" | "array" {
  if (value === null) return 'null';
  if (Array.isArray(value)) return 'array';

  const type = typeof value;

  switch (type) {
    case 'string':
    case 'number':
    case 'boolean':
      return type as JSONPrimitiveValueName;

    default:
      return 'object';
  }
}

function assertType(
  value: unknown,
  schema: RuntimeSchema,
  path: string,
): void {
  if (schema === 'any') return;

  if (typeof schema === 'string') {
    const actual = getRuntimeType(value);

    if (actual !== schema) {
      throw new TypeError(
        `${path}: JSON value is ${actual}, but generated type is ${schema}`,
      );
    }

    return;
  }

  // Just a helper function for better typing
  const isArraySchema = (
    val: typeof schema,
  ): val is readonly RuntimeSchema[] => Array.isArray(val);

  if (isArraySchema(schema)) {
    if (!Array.isArray(value)) {
      throw new TypeError(
        `${path}: JSON value is ${getRuntimeType(value)}, but generated type is array`,
      );
    }

    for (let i = 0; i < value.length; i++) {
      let matched = false;
      let lastError: unknown;

      for (const elementSchema of schema) {
        try {
          assertType(value[i], elementSchema, `${path}[${i}]`);
          matched = true;
          break;
        } catch (error) {
          lastError = error;
        }
      }

      if (!matched) {
        if (lastError instanceof Error) throw lastError;
        throw new TypeError(
          `${path}[${i}]: JSON value does not match any allowed element type`,
        );
      }
    }

    return;
  }

  if (
    value === null
    || typeof value !== 'object'
    || Array.isArray(value)
  ) {
    throw new TypeError(
      `${path}: JSON value is ${getRuntimeType(value)}, but generated type is object`,
    );
  }

  // String and array schemas returned earlier, so only an object schema
  // remains here.
  const classSchema = schema as RuntimeClass;

  for (const [field, fieldSchema] of Object.entries(classSchema.schema)) {
    if (!(field in value)) {
      throw new TypeError(
        `${path}.${field}: property is defined by the generated type but is missing from the JSON value`,
      );
    }

    assertType(
      value[field as keyof typeof value],
      fieldSchema,
      `${path}.${field}`,
    );
  }
}

export function validateJSON(
  value: unknown,
  rootSchema: RuntimeSchema,
  path?: string,
): void {
  assertType(value, rootSchema, path ?? '$');
}

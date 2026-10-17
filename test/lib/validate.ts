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
  ): val is readonly [RuntimeSchema] => Array.isArray(val);

  if (isArraySchema(schema)) {
    if (!Array.isArray(value)) {
      throw new TypeError(
        `${path}: JSON value is ${getRuntimeType(value)}, but generated type is array`,
      );
    }

    let currentIndex = 0;
    for (let i = 0; i < schema.length; i++) {
      const elementSchema = schema[i];
      const currentJsonValue = value[i];
      assertType(currentJsonValue, elementSchema, `${path}[${i}]`);

      currentIndex = i;
    }

    if (value.length > schema.length) {
      const elementSchema = schema[currentIndex];
      for (let i = currentIndex; i < value.length; i++) {
        assertType(value[i], elementSchema, `${path}[${i}]`);
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

  for (const [field, fieldSchema] of Object.entries(schema.schema)) {
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
  rootClass: RuntimeClass,
  path?: string,
): void {
  assertType(value, rootClass, path ?? '$');
}

type Primitive =
  | string
  | number
  | bigint
  | boolean
  | symbol
  | null
  | undefined
;
type PrimitiveName =
  | "string"
  | "number"
  | "bigint"
  | "boolean"
  | "symbol"
  | "null"
  | "undefined"
;

type JSONPrimitiveValue = Exclude<Primitive, JSONUnsupportedPrimitiveValue>;
type JSONPrimitiveValueName = Exclude<PrimitiveName, JSONUnsupportedPrimitiveValueName>;
type JSONUnsupportedPrimitiveValue = bigint | symbol | undefined;
type JSONUnsupportedPrimitiveValueName = "bigint" | "symbol" | "undefined";

type JSONValue =
  | JSONPrimitiveValue
  | JSONPrimitiveValue[]
  | { [key: string]: JSONPrimitiveValue }
;
type JSONValueName =
  | JSONPrimitiveValueName
  // This type "object" can be an array or object,
  // and because of `typeof null` is also returns "object",
  // we need to check nullish first before primitive types.
  | "object"
  | "array"  // Note: This is a custom type created by a function
;
type JSONUnsupportedValue =
  | Exclude<Primitive, JSONValue>
  | ((...args: unknown[]) => any)
  | (new (...args: unknown[]) => any)
;
type JSONUnsupportedValueName = Exclude<PrimitiveName, JSONValueName> | "function"

type RuntimeType = JSONPrimitiveValueName | "any";
type RuntimeSchema =
  | RuntimeType
  | readonly [RuntimeSchema]
  | RuntimeClass
;

interface RuntimeClass {
  readonly schema: Record<string, RuntimeSchema>;
}

interface Field {
  readonly name: string;
  readonly type: string;
}

interface InterfaceDefinition {
  readonly name: string;
  readonly fields: readonly Field[];
}

interface GeneratedModule {
  readonly root: RuntimeClass;
  readonly [field: PropertyKey]: any;
}


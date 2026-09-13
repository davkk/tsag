import def, { named } from "./m.js";
import type { TOnly } from "./t.js";
import * as ns from "./ns.js";

export const NUM: number = 1;
export let flag: boolean = true;
export var old: any = null;
const local_c: string = "s";
let [tx, ty]: [number, number] = [1, 2];
const { tname, tage = 3 }: { tname: string; tage?: number } = val;
const [d0, ...drest]: number[] = nums;

function foo(a: string, b?: number, ...rest: string[]): void {}
export function with_types(a: number, b: string = "d"): number { return a; }
export default function defaultFn(): void {}
async function fetchIt(url: string): Promise<string> { return url; }
function* gen(): Generator<number> { yield 1; }
function overloaded(x: string): void;
function overloaded(x: number): void;
function overloaded(x: unknown): void {}
function generic_id<T>(x: T): T { return x; }
function constrained<T extends { len: number }>(x: T): number { return x.len; }
declare function ambient_fn(x: string): void;

const arrow_t = (x: number): number => x;
const async_arrow_t = async (x: number): Promise<number> => x;
let fn_var: (a: number) => string;
type FnAlias = (a: number) => string;

interface MyInterface {
  prop: string;
  readonly ro: number;
  opt?: boolean;
  method(a: string): void;
  generic_m<T>(x: T): T;
  [key: string]: unknown;
  (call: string): void;
}

interface Extends extends MyInterface {
  extra: number;
}
interface GenericI<T, U = string> {
  first: T;
  second: U;
}

type MyType = string | number;
type GenericBox<T> = { value: T };
type Cond<T> = T extends string ? number : never;
type Mapped<T> = { [K in keyof T]: T[K] };
type Tpl<A extends string> = `get-${A}`;
type Union = "a" | "b" | 42;
type FnT = (x: number) => void;
type InferEx<T> = T extends (infer U)[] ? U : T;

enum MyEnum { A, B, C }
enum StrEnum { Up = "UP", Down = "DOWN" }
enum Mixed { N = 1, S = "s", Computed = 1 + 2 }
const enum ConstEnum { X, Y }
declare enum AmbientEnum { P, Q }

class MyClass {
  field: number = 42;
  static sfield = 1;
  private priv = 0;
  protected prot?: string;
  readonly rid = 7;
  #js_private = 1;
  optional?: number;

  constructor(private ctor_param: string, public count = 0) {}
  method(): void {}
  async am(): Promise<void> {}
  *gm(): Generator<number> { yield 1; }
  get v(): number { return 1; }
  set v(x: number) {}
  static make(): MyClass { return new MyClass("a"); }
  overloaded_m(x: string): void;
  overloaded_m(x: number): void;
  overloaded_m(x: unknown): void {}
}

abstract class AbstractBase {
  abstract render(): void;
  concrete(): number { return 1; }
  abstract get label(): string;
}

class Impl extends AbstractBase implements MyInterface {
  prop = "p";
  ro = 1;
  method(a: string): void {}
  render(): void {}
  get label(): string { return "l"; }
  [key: string]: unknown;
}

class Box<T> {
  constructor(public value: T) {}
  get(): T { return this.value; }
}

function takes_box(b: Box<string>): string { return b.get(); }

namespace MyNamespace {
  export const nv = 1;
  export function inner(): void {}
  export class InNs {
    m(): void {}
  }
  export namespace Deep {
    export type T = number;
  }
}

module MyModule {
  export function inner(): void {}
}

declare module "untyped-mod" {
  const x: number;
  export default x;
}
declare global {
  interface Window {
    __flag?: boolean;
  }
  var __global_thing: number;
}

abstract class WithAbstractSig {
  abstract foo(): void;
}

class WithIndex {
  [k: string]: number;
  normal = 1;
}

enum Single { Only }

export { foo, MyClass as Renamed };
export type { MyType };
export * from "./re.js";
export default foo;

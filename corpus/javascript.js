import def from "./def.js";
import { a, b as bee, c } from "./mod.js";
import * as ns from "./ns.js";

export const PI = 3.14;
export let counter = 0;
export var legacy = "old";
const local_const = 1;
let mutable = 2;
var hoisted = 3;
let multi_a = 1, multi_b = 2;
const [first, second, ...rest_arr] = [1, 2, 3, 4];
const { name, age: years, nested: { deep } = {} } = person;
const { with_default = 42, ...rest_obj } = opts;
var { v_destr } = thing;
let [v_x, v_y] = pair;

function foo() {}
export function exported_fn(x, y = 2, ...rest) { return x; }
export default function defaultFn() {}
async function fetchData(url) { return url; }
function* gen() { yield 1; yield 2; }
async function* agen() { yield 1; }
function shadowed(a, a) { return a; }

const arrow = () => {};
const async_arrow = async (x) => x * 2;
const expr_paren = (a, b) => a + b;
let fn_expr = function () {};
var named_expr = function helper() {};
const cond_fn = true ? () => 1 : function () { return 2; };
const obj_arrow_holder = { run: async () => 1 };

class MyClass {
  field = 42;
  static static_field = 1;
  #private_field = 0;
  static #priv_static = 2;
  bound = () => this.field;

  constructor(name) { this.name = name; }
  method() {}
  async asyncMethod() {}
  *genMethod() {}
  async *asyncGenMethod() {}
  get value() { return this._v; }
  set value(v) { this._v = v; }
  static create() { return new MyClass(); }
  #hidden() {}
  ["computed"]() {}
  static { counter += 1; }
}

export class Exported extends MyClass {
  overridden() {}
}
export default class DefaultExported {}
class Generic_like {}

const obj = {
  plain: 1,
  "quoted": 2,
  shorthand,
  method() {},
  async async_m() {},
  *gen_m() {},
  get g() { return 1; },
  set s(v) {},
  nested: { deep_method() {} },
  arrow_prop: () => {},
  0: "zero-key",
};

const instance = new MyClass("n");
const { extracted } = obj;

export { foo, bar as baz_export };
export * from "./re.js";
export * as all from "./all.js";

if (counter > 0) {
  function block_fn() {}
  let block_scoped = 1;
  const block_const = 2;
}

for (let i = 0; i < 3; i++) {}
for (const item of list) {}
for (var k in obj) {}

try {
  failable();
} catch (e) {
  const caught = e;
} finally {}

(function iife() {})();
(() => {})();

const dynamic = import("./lazy.js");
const with_template = `hello ${name}!`;
const re = /ab+c/gi;

module.exports = { foo };
exports.extra = 1;
globalThis.poly = true;

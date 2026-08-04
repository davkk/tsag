let x: number = 1;
const y = 2;
var z = 3;

function foo(): void {}
function* bar() {}

class MyClass {
  method(): void {}
  *genMethod() {}
  field: number = 42;
}

const arrow = (): void => {};

type MyType = string | number;

interface MyInterface {
  prop: string;
  method(): void;
}

enum MyEnum {
  A,
  B,
  C,
}

abstract class AbstractClass {
  abstract foo(): void;
}

namespace MyNamespace {
  export function inner() {}
}

module MyModule {
  export function inner() {}
}

export default function defaultFn() {}

"""Variate Python corpus: every def/class/assign/import shape."""
import os
import sys
import math as math_alias
import numpy as np
from collections import defaultdict, namedtuple as nt_alias
from . import sibling
from .sub import thing as thing_alias
from pkg.mod import func as func_alias

try:
    import ujson as json_impl
except ImportError:
    import json as json_impl

CONSTANT = 1
_underscored = 2
annotated: int = 3
unannotated_str = "hello"
a, b = 1, 2
first, *middle, last = range(5)
x = y = chained = 0
tpl = (1, 2)
lst = [1, 2, 3]
dct = {"a": 1}
st = {1, 2}
obj_var = SomeClass(1)
matrix = [[0] * 3 for _ in range(3)]
EXPECTED = [m for m in [1, 2] if m]


def plain_function():
    pass


def with_params(a, b=1, *args, mode="x", **kwargs):
    pass


def with_annotations(a: int, b: str = "d") -> bool:
    return True


def pos_only(a, b, /, c, *, d, e=1):
    pass


async def async_function(url: str):
    pass


def one_liner():
    return 1


def outer():
    local_var = 10

    def inner():
        return local_var

    def inner_with_params(x, y=2):
        return x + y

    lam = lambda q: q + 1  # lambda must NOT tag
    return inner


@decorator
def decorated_function():
    pass


@decorator_with_args("route", methods=["GET"])
@other.marks.slow
async def async_decorated():
    pass


def with_global():
    global CONSTANT
    CONSTANT = 99


def with_nonlocal():
    token = 1

    def child():
        nonlocal token
        token = 2

    return child


def with_walrus(items):
    if (n := len(items)) > 2:
        return n
    while (line := items.pop() if items else None):
        pass
    return 0


def with_control(x):
    for i in range(x):
        pass
    for k, v in dct.items():
        pass
    while x > 0:
        x -= 1
        if x == 2:
            break
        elif x == 1:
            continue
        else:
            pass
    with open("f") as fh:
        data = fh.read()
    try:
        1 / 0
    except ZeroDivisionError as exc:
        raise ValueError("bad") from exc
    except (KeyError, IndexError):
        pass
    else:
        pass
    finally:
        pass
    assert x >= 0, "neg"
    del data
    return x


class PlainClass:
    CLASS_ATTR = 1
    annotated_attr: str = "s"

    def instance_method(self, x):
        self.cached = x  # self-attr: currently NOT tagged (edge)
        return x

    def __dunder__(self):
        pass

    def _private(self):
        pass

    @classmethod
    def class_method(cls, name):
        return name

    @staticmethod
    def static_method(a, b):
        return a + b

    @property
    def prop(self):
        return self._v

    @prop.setter
    def prop(self, value):
        self._v = value

    @decorator_with_args(1, key="v")
    async def mixed(self, q: int = 0):
        async with open_async(q) as fh:
            return fh

    class NestedClass:
        NESTED_CONST = 5

        def nested_method(self):
            pass

    def makes_closure(self):
        def inner():
            pass

        return inner


class WithBases(Base1, Base2, metaclass=Meta):
    pass


class MultiLine(
    Base1,
    Base2,
    Mixin,
):
    pass


@decorator
@other_decorator(arg=1)
class DecoratedClass:
    pass


@dataclasses.dataclass
class DataLike:
    name: str
    count: int = 0


class WithControlDefs:
    if True:
        def in_if(self):
            pass

    if False:
        pass
    else:
        def in_else(self):
            pass

    for _i in range(1):
        def in_for(self):
            pass

    while False:
        def in_while(self):  # never true at runtime, still syntax
            pass

    with open("x"):
        pass

    try:
        def in_try(self):
            pass
    except ImportError:
        def in_except(self):
            pass


if True:
    def conditional_fn():
        pass


if TYPE_CHECKING:
    def type_only_fn(x: int) -> int:
        ...


try:
    def guarded_fn():
        pass
except Exception:
    pass


async def main_async():
    await async_function("u")
    async with ctx() as c:
        pass
    async for row in stream():
        pass


def generator_fn():
    yield 1
    yield from [2, 3]


async def async_gen():
    yield 1


if __name__ == "__main__":
    main_async()

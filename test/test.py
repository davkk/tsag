import os
import sys
from typing import List


def plain_function():
    pass


@decorator
def decorated_function():
    pass


def with_params(a, b=1, *args, **kwargs):
    pass


async def async_function():
    pass


@decorator
async def async_decorated():
    pass


class PlainClass:
    def instance_method(self):
        pass

    @classmethod
    def class_method(cls):
        pass

    @staticmethod
    def static_method():
        pass

    class NestedClass:
        pass

    def nested_function(self):
        def inner():
            pass

        return inner


@decorator
class DecoratedClass:
    pass


class MultiLine(
    Base1,
    Base2,
):
    pass


module_var = 1
module_var2 = "hello"
a, b = 1, 2
c = d = 3
tuple_var = (1, 2)
list_var = [1, 2, 3]
dict_var = {"a": 1}
obj_var = SomeClass()


def shadowed():
    local_var = 10
    global_var = 20
    return local_var


import math
import numpy as np
from collections import defaultdict
from . import sibling


if __name__ == "__main__":
    main()

-- Variate Lua corpus: function shapes + locals/globals/table edge cases.
local x = 1
local a, b = 1, 2
local s, t, u = 1, "two", {}
y = 2 -- global (edge: currently untagged)
M = {}
local MOD = {}
local _M = {}

function foo()
end

function with_params(a, b, c)
  return a
end

function vargs(a, ...)
  local collected = { ... }
  return a, collected
end

function multi_ret()
  return 1, "two", nil
end

local function qux()
end

local function with_body(n)
  local total = 0
  for i = 1, n do
    total = total + i
  end
  return total
end

function M.bar()
end

function M.baz(a, b)
end

function M:baz_method(a)
  self.val = a
end

function MOD:colon_only(x, y)
  return (x or 0) + (y or 0)
end

function a.b.c.deep()
end

function obj.sub:chained()
end

function outer_fn()
  local function inner_fn()
  end

  local function inner_with_args(p)
    return p
  end

  local anon = function(q)
    return q
  end

  local arrow_like = function(...)
    return ...
  end
  return inner_fn, anon, arrow_like
end

-- assigned anonymous functions (edge: anonymous, name from LHS)
named_assign = function()
end

local local_assign = function()
end

M.field_fn = function(z)
  return z
end

-- table constructors + module patterns
local T = {
  name = "widget",
  count = 3,
  ["quoted-key"] = true,
  [10] = "ten",
  nested = { x = 1, y = 2 },
  list = { 1, 2, 3 },
  fn_field = function() end,
}

local Point = {}
Point.__index = Point

function Point.new(x, y)
  local self = setmetatable({}, Point)
  self.x = x
  self.y = y
  return self
end

function Point:norm()
  return (self.x * self.x + self.y * self.y) ^ 0.5
end

-- require / loops / control with defs inside
local m = require("mod")
local ok, res = pcall(require, "maybe")

for i = 1, 10 do
end

for idx, val in ipairs(T.list) do
end

for k, v in pairs(T) do
  local loop_local = k
end

local n = 0
while n < 3 do
  n = n + 1
end

repeat
  n = n - 1
until n <= 0

if x == 1 then
  function in_if_branch()
  end
elseif x == 2 then
  function in_elseif_branch()
  end
else
  function in_else_branch()
  end
end

local str = [[
  long bracket string
]]
local qstr = [==[ level-2 bracket ]==]

_M.version = "1.0"
_M.new = Point.new

return _M

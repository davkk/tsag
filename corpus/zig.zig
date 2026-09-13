const std = @import("std");
const builtin = @import("builtin");
const C = @import("c.zig");

const x: u32 = 1;
var y: u32 = 2;
pub const PUB_CONST: u8 = 7;
pub var pub_counter: usize = 0;
threadlocal var tls_count: u32 = 0;
var comptime_counter: u32 = 0;
const inferred = 42;
const typed_arr: [4]u8 = .{ 1, 2, 3, 4 };
const with_align: u32 align(16) = 0;
const with_link: u8 linksection(".meta") = 0;
const maybe_null: ?u32 = null;
const any_arr = [_]u8{ 1, 2, 3 };
var uninit: u32 = undefined;

fn foo() void {}
fn bar(v: u32) u32 {
    return v + x + y;
}
pub fn pub_fn(a: []const u8) !usize {
    return a.len;
}
export fn export_fn(v: c_int) void {
    _ = v;
}
inline fn inl(v: u32) u32 {
    return v;
}
fn variadic_like(args: anytype) void {
    _ = args;
}
fn generic_max(comptime T: type, a: T, b: T) T {
    return if (a > b) a else b;
}
fn with_err() anyerror!u32 {
    return 1;
}
fn no_ret() noreturn {
    while (true) {}
}
fn with_fn_ptr(cb: *const fn (u32) u32, v: u32) u32 {
    return cb(v);
}
fn defer_demo() void {
    defer y += 1;
    errdefer y += 2;
}

const Point = struct {
    x: u32,
    y: u32 = 0,
    name: []const u8 = "p",
    cb: ?*const fn (u32) void = null,

    pub fn norm(self: Point) f32 {
        _ = self;
        return 0;
    }
    pub fn set(self: *Point, nx: u32, ny: u32) void {
        self.x = nx;
        self.y = ny;
    }
    fn hidden(self: Point) void {
        _ = self;
    }
};

const Packed = packed struct {
    a: u4,
    b: u4,
    flag: bool = true,
};

const Ext = extern struct {
    fd: c_int,
    buf: [256]u8,
};

const Status = enum(u8) {
    ok,
    err,
    pending = 5,
};

const Tagged = union(enum) {
    int: u32,
    float: f64,
    none,
    pair: struct { a: u32, b: u32 },

    pub fn is_int(self: Tagged) bool {
        return self == .int;
    }
};

const Bare = union {
    a: u32,
    b: f32,
};

const Errors = error{
    NotFound,
    Denied,
    Io,
};

const OpaqueH = opaque {};
const WrapInt = struct { v: u32 };
const AliasInt = u32;
const PtrAlias = *Point;
const SliceAlias = []const u8;

const NS = struct {
    pub const inner_const = 1;
    pub fn helper() void {}
    pub const Deep = struct {
        pub fn deep_fn() void {}
    };
};

pub fn uses_ns() void {
    _ = NS.inner_const;
    NS.helper();
}

const std_alias = std;

test "my test" {
    try std.testing.expect(x == 1);
}

test "named_test_fn" {
    var t = Point{ .x = 1 };
    t.set(2, 3);
}

test "errors" {
    const e: Errors = error.NotFound;
    _ = e;
}

comptime {
    comptime_counter = 1;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const n = bar(41);
    std.debug.print("{}\n", .{n});
}

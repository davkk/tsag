const x: u32 = 1;
var y: u32 = 2;

fn foo() void {}
fn bar(x: u32) u32 { return x; }

const Point = struct {
    x: u32,
    y: u32,
};

const Status = enum {
    ok,
    err,
};

const Tag = union(enum) {
    int: u32,
    float: f64,
};

test "my test" {}
test myNamedTest {}

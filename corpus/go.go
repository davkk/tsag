//go:build linux && amd64

package main

import (
	"C"
	"fmt"
	"math"
	alias "strings"
	. "strconv"
	_ "net/http/pprof"
	multi1 "a"
	multi2 "b"
)

const maxCount = 100
const typedConst int = 7
const (
	_ = iota
	Ok Status = iota
	Err
	Fatal
)
const (
	StrA = "a"
	StrB StrAlias = "b"
)
const BlockTyped string = "x"

var globalVar = 1
var typedVar string = "s"
var (
	VarA, VarB = 1, 2
	VarC int
	ignored, kept = f(), 2
)
var _ = 0

type Point struct {
	X float64 `json:"x"`
	Y float64 `json:"y,omitempty"`
	name string
	hidden *int `json:"-"`
	io.Reader
	*Base
	Tags [4]string
	Next *Point
}

type Status int

type Shape interface {
	Area() float64
	String() string
	io.Reader
	~int | ~float64
}

type Simple interface {
	M()
}

type Id = int64
type StrAlias = string
type Handler func(req *Request) error
type Ints []int
type Mapping map[string]int
type ChanAlias chan int
type StatusAlias = Status
type Ordered interface {
	~int | ~float64 | ~string
}
type Box[T any] struct {
	Value T `json:"value"`
	Next  *Box[T]
}
type Pair[A, B any] struct {
	First  A
	Second B
}
type List[T any] []T

func add(a, b int) int {
	return a + b
}

func multi(a, b int, s string) (int, error) {
	return a + b, nil
}

func namedRet(a int) (x, y int) {
	x, y = a, a*2
	return
}

func variadicSum(xs ...int) int {
	s := 0
	for _, v := range xs {
		s += v
	}
	return s
}

func genericMap[T, U any](in []T, f func(T) U) []U {
	out := make([]U, len(in))
	for i, v := range in {
		out[i] = f(v)
	}
	return out
}

func withConstraint[T Ordered](v T) T { return v }

func (p Point) Norm() float64 {
	return math.Sqrt(p.X*p.X + p.Y*p.Y)
}

func (p *Point) Set(x, y float64) {
	p.X, p.Y = x, y
}

func (b Box[T]) Get() T {
	return b.Value
}

func (s Status) String() string { return "s" }

func init() {
	setup()
}

func setup() {}

func usesLocals() int {
	short := 1 // short-decl edge: query targets top-level var only
	a, b := 1, 2
	var shadow = short + a + b
	{
		block_scoped := 3
		shadow += block_scoped
	}
	for i := 0; i < 3; i++ {
		shadow += i
	}
	switch shadow {
	case 1:
		shadow = 10
	case 2, 3:
		shadow = 20
	default:
		shadow = 30
	}
	select {
	default:
	}
	go shadow_fn()
	defer cleanup()
	f := func(x int) int { return x } // anon func: must NOT tag
	return f(shadow)
}

func main() {
	p := Point{X: 1, Y: 2}
	fmt.Println(p.Norm(), add(1, 2), Itoa(3), alias.ToUpper("a"))
}

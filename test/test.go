package main

import (
	"fmt"
	"math"
)

const maxCount = 100

var globalVar = 1

type Point struct {
	X float64
	Y float64
}

type Status int

const (
	Ok Status = iota
	Err
)

type Shape interface {
	Area() float64
}

type Id = int64

func add(a, b int) int {
	return a + b
}

func (p Point) Norm() float64 {
	return math.Sqrt(p.X*p.X + p.Y*p.Y)
}

func main() {
	p := Point{X: 1, Y: 2}
	fmt.Println(p.Norm())
}

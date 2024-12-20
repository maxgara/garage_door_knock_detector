package main

import (
	"fmt"
	"math"
)

var size int = 1024

func main() {
	var coeff = (2.0 * math.Pi / float64(size)) * 10
	for x := range size {
		y := float64(500.0) + 100*math.Sin(coeff*float64(x))
		fmt.Printf("%v\n", y)
	}
}

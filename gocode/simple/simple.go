package main

import (
	"fmt"
	"log"
	"math"
	"os"
	"strconv"
	"strings"
)

func graphContributions(filename string) {
	data := loadFile(filename)
	linec := len(data)
	var dataAvg float64
	for _, y := range data {
		dataAvg += y
	}
	dataAvg /= float64(linec)
	//normalize data to 0
	for i := range data {
		data[i] = data[i] - dataAvg
	}
	fmt.Printf("%v Data Normalized to zero\n", filename)
	fmt.Println("data")
	for _, y := range data {
		fmt.Println(y)
	}
	fmt.Println("zero")
	for range data {
		fmt.Println(0)
	}
	fmt.Printf("-n\nContributions\n")
	var avgmatch float64
	for cidx := range data[:linec/2] { //linec[:1] {
		var smatch float64
		var cmatch float64
		for x, y := range data {
			sterm := sinf(coeff(cidx, linec), x) * y
			cterm := cosf(coeff(cidx, linec), x) * y
			smatch += sterm
			cmatch += cterm
		}
		smatch = smatch * smatch
		cmatch = cmatch * cmatch
		matchterm := math.Sqrt(smatch + cmatch)
		fmt.Printf("%f\n", matchterm)
		avgmatch += matchterm
	}
}
func main() {
	fmt.Println("Fourier Transforms")
	graphContributions("input.txt")
	fmt.Printf("-n\n")
	graphContributions("input2.txt")

}
func loadFile(name string) (data []float64) {

	fbytes, _ := os.ReadFile(name)
	file := string(fbytes)
	lines := strings.Fields(file)
	// pcount := len(lines)
	for idx, line := range lines {
		y, err := strconv.ParseFloat(line, 64)
		if err != nil {
			log.Fatalf("ParseFloat error line %v:[%v]\n", idx, line)
		}
		data = append(data, y)
	}
	return
}
func coeff(i, count int) float64 {
	return 2 * math.Pi * float64(i) / float64(count)
}
func sinf(coeff float64, x int) float64 {
	return math.Sin(float64(x) * coeff)
}
func cosf(coeff float64, x int) float64 {
	return math.Cos(float64(x) * coeff)
}

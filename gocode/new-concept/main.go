package main

import (
	"fmt"
	"log"
	"math"
	"os"
	"strconv"
	"strings"
)

//analyze streaming data for knock-like pattern without need to save and reprocess almost any data (hopefully).

func main() {
	data1 := loadFile("/Users/maxgara/Desktop/al-code/garage/filesdareknocksandearenonknocks/knock1.txt")
	data2 := loadFile("/Users/maxgara/Desktop/al-code/garage/filesdareknocksandearenonknocks/knock2.txt")
	data3 := loadFile("/Users/maxgara/Desktop/al-code/garage/filesdareknocksandearenonknocks/knock3.txt")
	series0 := [][]float64{data1, data2, data3}
	//new knocks with much cleaner signature, taken from garage wifi comms
	datac1 := loadFile("/Users/maxgara/Desktop/al-code/garage/filesdareknocksandearenonknocks/new_clean_knock1.txt")
	datac2 := loadFile("/Users/maxgara/Desktop/al-code/garage/filesdareknocksandearenonknocks/new_clean_knock2.txt")
	datac3 := loadFile("/Users/maxgara/Desktop/al-code/garage/filesdareknocksandearenonknocks/new_clean_knock3.txt")
	datac4 := loadFile("/Users/maxgara/Desktop/al-code/garage/filesdareknocksandearenonknocks/new_clean_knock4.txt")
	datac5 := loadFile("/Users/maxgara/Desktop/al-code/garage/filesdareknocksandearenonknocks/new_clean_knock5.txt")
	series1 := [][]float64{datac1, datac2, datac3, datac4, datac5}
	_ = series0
	_ = series1

	EachSeries(series1, func(data []float64) {
		fmt.Printf("New Signatures\n\n")
		fmt.Println("raw")
		printarr(data)
		fmt.Println("Integral")
		fmt.Printf("\n-n\n")
		cleanF1 := fint(data)
		printarr(cleanF1)
		fmt.Println()
		fmt.Println("-n")
		fmt.Println("F2")
		cleanF2 := fint(cleanF1)
		printarr(cleanF2)
		fmt.Println()
		fmt.Println("-n")
		fmt.Println("F3")
		cleanF3 := fint(cleanF2)
		printarr(cleanF3)
		fmt.Println()
		fmt.Println("-n")
		fmt.Println("fourier transform")
		printfour(fourier(datac1))
		fmt.Printf("\n-n")

	})
	// fmt.Println()
	// fmt.Println("-n")
	// fmt.Println("raw")
	// printarr(datac2)
	// fmt.Println()
	// fmt.Println("-n")
	// fmt.Println("fourier transform")
	// // printfour(fourier(datac2))
	// fmt.Println()
	// fmt.Println("-n")
	// fmt.Println("raw")
	// printarr(datac3)
	// fmt.Println()
	// fmt.Println("-n")
	// fmt.Println("fourier transform")
	// printfour(fourier(datac3))
	// // fmt.Println()
	// // fmt.Println("-n")
	// // fmt.Println("raw")
	// // printarr(datac4)
	// // fmt.Println()
	// // fmt.Println("-n")
	// // fmt.Println("fourier transform")
	// // printfour(fourier(datac4))
	// // fmt.Println("raw")
	// // fmt.Println()
	// // fmt.Println("-n")
	// // printarr(datac5)
	// // fmt.Println()
	// // fmt.Println("-n")
	// // fmt.Println("fourier transform")
	// // printfour(fourier(datac5))

	// fmt.Printf("-n\n")
	// cleanF3 := fint(datac3)
	// printarr(cleanF3)

}

// apply f to each series in sers
func EachSeries(sers [][]float64, f func([]float64)) {
	for _, ser := range sers {
		f(ser)
	}
}

// print data formatted as fourier-transform output data
func printfour(data []float64) {
	n := len(data)
	var m float64
	var midx int
	for i, v := range data[1 : n/2] {
		if v >= m {
			m = v
			midx = i
		}
		fmt.Printf("%v\n", v)
	}
	//translate max freq to be independent of number of window size
	midxtrans := float64(midx) / float64(n) * 1000
	fmt.Printf("maximum at x=%v: (%v) [real freq=%v / 1000]", midx, m, midxtrans)
	fmt.Println()
}

// get fourier transform of data
func fourier(data []float64) []float64 {
	conts := make([]float64, len(data))
	for k := range data {
		conts[k] = cont(k, data)
	}
	return conts
}

// contribution of fourier component k/n for data (data length = n)
func cont(k int, data []float64) float64 {
	fs := func(x int) float64 {
		n := len(data)
		c := 2 * math.Pi * float64(k) / float64(n)
		return math.Sin(c * float64(x))
	}
	fc := func(x int) float64 {
		n := len(data)
		c := 2 * math.Pi * float64(k) / float64(n)
		return math.Cos(c * float64(x))
	}
	var stotal, ctotal float64
	for x, p := range data {
		stotal += fs(x) * p
		ctotal += fc(x) * p
		// fmt.Printf("data[%v]=%v; sin(%v)=%v; cos(%v)=%v; sterm,cterm = (%v, %v)\n", x, p, x, fs(x), x, fc(x), fs(x)*p, fc(x)*p)
	}
	return math.Sqrt(stotal*stotal + ctotal*ctotal)
}

// print f([xmin,xmax])
func printfunc(f func(float64) float64, xmin, xmax int) {
	for i := xmin; i < xmax; i++ {
		fmt.Println(f(float64(i)))
	}
}

// get f(data)
func evalfunc(f func(float64) float64, data []float64) []float64 {
	out := make([]float64, len(data))
	for i, p := range data {
		out[i] = f(p)
	}
	return out
}

// print f(data)
func printarr(data []float64) {
	for _, p := range data {
		fmt.Println(p)
	}
}

// load + parse data file
func loadFile(name string) (data []float64) {

	fbytes, err := os.ReadFile(name)
	if err != nil {
		fmt.Println(err)
		return nil
	}
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

// F(data); where F(x) = Sum of f(p) over p in [0,x] (discrete integral)
func fint(data []float64) []float64 {
	avg := Avg(data)
	var Farr = make([]float64, len(data))
	var sum float64
	for i, p := range data {
		term := (p - avg) / min(avg, 1.0)
		sum += term
		Farr[i] = sum
	}
	return Farr
}

// average of data
func Avg(data []float64) float64 {
	var sum float64
	for _, v := range data {
		sum += v
	}
	return sum / float64(len(data))
}

// print first 5 integrals of data
func print_ints(data []float64) {
	av := Avg(data)
	fmt.Printf("AVG=%v\n", av)
	for _, p := range data {
		fmt.Println(p)
	}
	F := fint(data)
	av = Avg(F)
	fmt.Printf("\n-n\n")
	fmt.Printf("AVG=%v\n", av)
	printarr(F)
	for range 5 {
		F = fint(F)
		av = Avg(F)
		fmt.Printf("\n-n\n")
		fmt.Printf("AVG=%v\n", av)
		printarr(F)
	}
}

// sin func coefficient for fourier component out of n
func fcoeff(i, n int) float64 {
	return 2 * math.Pi * float64(i) / float64(n)
}
func sinf(coeff float64, x int) float64 {
	return math.Sin(float64(x) * coeff)
}
func cosf(coeff float64, x int) float64 {
	return math.Cos(float64(x) * coeff)
}

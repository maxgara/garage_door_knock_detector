package main

import (
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
)

// find a parabola that best fits the data given as an argument
// we assume no big peaks in training data outside of knock

var printBuff []byte

func main() {
	// if len(os.Args) != 2 {
	// 	log.Fatal("not enough args")
	// }
	// for _, arg := range os.Args {
	// 	data := loadFile(arg)
	// 	arrPrint("normal", data, false)
	// 	D, E, w := fitCurve(data)
	// 	f2Print("fit", D, E, w, data)
	// 	sdata := smooth(data, 10)
	// 	arrPrint("smooth", sdata, false)
	// 	D, E, w = fitCurve(sdata)
	// 	f2Print("fit-smooth", D, E, w, sdata)
	// }

	// ksdata := loadFileNewFormat("eightrealknocks.txt")
	// dcsdata := loadFileNewFormat("eightdoorcloses.txt")
	data := loadFile()
	printBuff = append(printBuff, "knocks vs door closes"...)
	// fmt.Println(len(mdata))
	// arrPrint("normal", mdata[0], true)
	for x := range ksdata {
		kdata := ksdata[x]
		dcdata := dcsdata[x]
		ksmooth := smooth(kdata, 50)
		dcsmooth := smooth(dcdata, 50)
		arrPrint("knock", kdata, true)
		D, E, w := fitCurve(kdata)
		fmt.Printf("k fit:D=%v, E=%v, w=%v\n", D, E, w)
		f2Print("fit", D, E, w, kdata)

		arrPrint("door close", dcdata, true)
		D, E, w = fitCurve(dcdata)
		fmt.Printf("dc fit:D=%v, E=%v, w=%v\n", D, E, w)
		f2Print("fit", D, E, w, dcdata)

		arrPrint("knock-smooth", ksmooth, true)
		D, E, w = fitCurve(ksmooth)
		fmt.Printf("ksm fit:D=%v, E=%v, w=%v\n", D, E, w)
		f2Print("fit", D, E, w, ksmooth)

		arrPrint("door close-smooth", dcsmooth, true)
		D, E, w = fitCurve(dcsmooth)
		fmt.Printf("dcsm fit:D=%v, E=%v, w=%v\n", D, E, w)
		f2Print("fit", D, E, w, dcsmooth)
	}
	fmt.Println("door closes")
	// arrPrint("normal", mdata[0], true)

	// D, E, w = fitCurve(sdata, ymaxidx, ymax)
	// arrPrint("fit", sdata, true)

	printFlush()
}

func f(A, B, C, x float64) float64 {
	val := A*x*x + B*x + C
	// fmt.Printf("val [%v]: %v => ", x, val)
	if val < 0 {
		val = 0
	}
	// fmt.Printf("%v\n", val)
	return val
}
func loadFile(name string) (data []float64) {

	fbytes, _ := os.ReadFile(name)
	file := string(fbytes)
	lines := strings.Split(file, "\n")
	l := len(lines)
	// pcount := len(lines)
	for idx, line := range lines[:l-1] {
		y, err := strconv.ParseFloat(line, 64)
		if err != nil {
			log.Fatalf("ParseFloat error line %v:%v\n", idx, line)
		}
		data = append(data, y)
	}
	return
}
func f2(D, E, w, x float64) float64 {
	val := D*(x-w)*(x-w) + E
	if val < 0 {
		val = 0
	}
	return val
}
func error2(D, E, w float64, data []float64) float64 {
	f2_app := func(x int) float64 { return f2(D, E, w, float64(x)) }
	var errsum float64
	for x := range data {
		errsum += (f2_app(x) - data[int(x)]) * (f2_app(x) - data[int(x)])
	}
	return errsum
}
func f2Print(label string, D, E, w float64, data []float64) {
	printBuff = append(printBuff, []byte("\n"+label+"\n")...)
	for x := range data {
		p := f2(D, E, w, float64(x))
		pstr := fmt.Sprintf("%v\n", p)
		printBuff = append(printBuff, []byte(pstr)...)
	}
}
func printFlush() {
	os.WriteFile("fit_output_data.txt", printBuff, 0b111111111)
}

func smooth(data []float64, times int) []float64 {
	l := len(data)
	var temp = make([]float64, l) //hold last smooth result
	var newData = make([]float64, l)
	copy(temp, data)
	for range times {
		if l == 0 {
			fmt.Println("no data for smooth...")
			return nil
		}
		newData[0] = temp[0]
		newData[l-1] = temp[l-1]
		//3 point smooth:
		for x := 1; x < l-1; x++ {
			newData[x] = (temp[x-1] + temp[x] + temp[x+1]) / 3
		}
		copy(temp, newData)
	}
	return newData
}
func arrPrint(name string, arr []float64, newchart bool) {
	header := fmt.Sprintf("%v\n", name) //default
	if newchart {
		header = fmt.Sprintf("\n-n\n%v\n", name)
	}
	printBuff = append(printBuff, []byte(header)...)
	for _, v := range arr {
		s := fmt.Sprintf("%v\n", v)
		printBuff = append(printBuff, []byte(s)...)
	}
}

// find
func fitCurve(data []float64) (D, E, w float64) {
	var ymaxidx int
	var ymax float64
	for x, y := range data {
		if y > ymax {
			ymax = y
			ymaxidx = x
		}
	}
	// initially take w = -ymaxIdx, D=-1, E = ymax (center on peak)
	// f(x) = D(x+w)^2 + E  = Dx^2 + 2Dxw + Dw^2 + E ->
	// D = A; 2Dw = B; E + Dw^2 = C

	// A = D = -1
	// B = 2Dw = 2*(-1)*(-ymaxidx)
	// C = E + Dw^2 = ymax + (-1)(-ymaxidx)^2

	// f(x) = Ax^2 + Bx + C
	// A := float64(-1)
	// B := float64(2 * ymaxidx)
	// C := ymax - float64(ymaxidx)*float64(ymaxidx)
	D = float64(-1) * 1000
	E = ymax
	w = float64(ymaxidx)
	err := error2(D, E, w, data)
	step := D / 2
	var stepidx int
	for range 1000 {
		stepidx++
		// fmt.Printf("step %v\t", stepidx)
		if newerr := error2(D-step, E, w, data); newerr <= err {
			D -= step
			// fmt.Printf("-- Reducing D to %-15v\t\tdelta=%-10v\tERR=%v\n", D, step, newerr)
			err = newerr
		} else if newerr = error2(D+step, E, w, data); newerr <= err {
			D += step
			// fmt.Printf("++ Increasing D to %-15v\t\tdelta=%-10v\tERR=%v\n", D, step, newerr)
			err = newerr
		} else {
			// fmt.Println("\tfailed to improve err; reducing step")
			step /= 2
		}
	}
	// fmt.Println(err)
	return
}
func loadFileNewFormat(name string) (data [][]float64) {
	fbytes, _ := os.ReadFile(name)
	file := string(fbytes)
	lines := strings.Split(file, "\r\n")
	larr0 := strings.Split(lines[0], "\t")
	sct := len(larr0)
	data = make([][]float64, sct)
	for idx, line := range lines[1:] {
		larr := strings.Split(line, "\t")
		for sidx, ystr := range larr {
			y, err := strconv.ParseFloat(ystr, 64)
			if err != nil {
				// fmt.Printf("ParseFloat error line %v:[%v]\n", idx, ystr)
				log.Fatalf("ParseFloat error line %v:[%v]\n", idx, ystr)
			}
			data[sidx] = append(data[sidx], y)
		}
	}
	return
}

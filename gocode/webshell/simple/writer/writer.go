package main

//write file contents to named pipe
import (
	"fmt"
	"os"
)

func main() {
	var file string
	p, err := os.OpenFile("./../stoc", os.O_WRONLY, os.ModeNamedPipe)
	fmt.Println("stoc opened for write.")
	if len(os.Args) == 1 {
		file = "./../testinput.txt"
		// fmt.Println("missing input file")
		// return
	} else {
		file = os.Args[1]
	}
	f, err := os.ReadFile(file)
	f = []byte("1\n")
	if err != nil {
		fmt.Println("couldn't read input file")
		return
	}
	for _, c := range f {
		n, err := fmt.Fprintf(p, "%c", c)
		fmt.Printf("stoc <- %c\n", c)
		fmt.Printf("[%c] wrote %v bytes. err=%v\n", c, n, err)
	}
}

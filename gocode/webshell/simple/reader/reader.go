package main

//read from named pipe and send to stdout
import (
	"fmt"
	"os"
	"os/exec"
)

// read from stdin pipe
func main() {
	runproc()
	p, err := os.OpenFile("./../ctos", os.O_RDONLY, os.ModeNamedPipe)
	fmt.Println("ctos opened for read")
	if err != nil {
		fmt.Println(err)
		return
	}
	readbychar(p)
}

func readbychar(p *os.File) {
	fmt.Println("reading...")
	b := make([]byte, 1)
	for {
		got, err := p.Read(b)
		if err != nil {
			fmt.Printf("reader got err: %v\n", err)
			return
		}
		if got == 1 {
			fmt.Printf("ctos -> %s\n", b)
		}
	}
}

func runproc() {
	com := exec.Command("./startclientproc.sh")
	com.Start()
}

package main

//notes: the below is working!!!
//read from named pipe and send to stdout
import (
	"fmt"
	"os"
	"os/exec"
)

// read from stdin pipe
func main() {
	// runproc()
	// fmt.Println("proc started")
	p, err := os.OpenFile("/Users/maxgara/Desktop/al-code/garage/gocode/webshell/simple/test/ctos", os.O_RDONLY, os.ModeNamedPipe)
	fmt.Println("ctos opened for read")
	if err != nil {
		fmt.Println(err)
		return
	}
	read(p)
}

func read(p *os.File) {
	fmt.Println("reading...")
	b := make([]byte, 256)
	for {
		got, err := p.Read(b)
		if got != 0 {
			fmt.Printf("ctos -> %s\n", b[:got])
		}
		if err != nil {
			fmt.Printf("reader got err: %v\n", err)
			return
		}
	}
}

func runproc() {
	com := exec.Command("./startclientproc.sh")
	com.Start()
}

package main

//notes: the below is working!!!
//read from named pipe and send to stdout
import (
	"fmt"
	"os"
	"os/exec"
)

var r, w *os.File

// read from stdin pipe
func main() {
	var counter int
	// runproc()
	// fmt.Println("proc started")
	var err error
	r, err = os.OpenFile("/Users/maxgara/Desktop/al-code/garage/gocode/webshell/simple/test/ctos", os.O_RDONLY|os.O_SYNC, os.ModeNamedPipe)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println("ctos opened for read")
	w, err = os.OpenFile("/Users/maxgara/Desktop/al-code/garage/gocode/webshell/simple/test/stoc", os.O_WRONLY|os.O_SYNC, os.ModeNamedPipe)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println("stoc opened for write")
	read(r, &counter)
	write(w, counter)
}

func read(p *os.File, c *int) {
	fmt.Println("reading...")
	b := make([]byte, 256)
	for {
		got, err := p.Read(b)
		if got != 0 {
			fmt.Printf("ctos -> %#q\n", b[:got])
			write(w, *c)
			*c++
		}
		if err != nil {
			fmt.Printf("reader got err: %v\n", err)
			return
		}
	}
}
func write(p *os.File, c int) {
	fmt.Fprintf(p, "count=%v\n", c)
}
func runproc() {

	com := exec.Command("./startclientproc.sh")
	com.Start()
}

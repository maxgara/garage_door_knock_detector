package main

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"time"
)

var log bytes.Buffer

func main() {
	go savelog()
	done := make(chan bool)
	go listen(done) //read from stdin
	go tick()
	<-done
}
func savelog() {
	for {
		time.Sleep(time.Second)
		b := make([]byte, 256)
		n, _ := log.Read(b)
		fmt.Printf("savelog:%v\n", string(b[:n]))
		// if err != nil {
		// 	fmt.Println(err)
		// 	return
		// }
		// fmt.Println("log saved")
	}
}
func tick() {
	t := time.NewTicker(time.Second)
	for {
		fmt.Println(<-t.C) //write to stdout

	}
}
func listen(done chan bool) {
	// b := make([]byte, 4)
	r := bufio.NewReader(os.Stdin)
	for {
		b, err := r.ReadByte()
		if err != nil {
			fmt.Fprintf(&log, "client: error on read: %v\n", err)
			done <- true
			return
		}
		fmt.Printf("read: [%c]!\n", b)
	}

}

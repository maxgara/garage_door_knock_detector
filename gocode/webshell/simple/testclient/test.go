package main

import (
	"bufio"
	"fmt"
	"os"
	"time"
)

func main() {
	go listen() //read from stdin
	t := time.NewTicker(time.Millisecond)
	for {
		fmt.Println(<-t.C) //write to stdout
	}
}
func listen() {
	// b := make([]byte, 4)
	r := bufio.NewReader(os.Stdin)
	for {
		b, err := r.ReadByte()
		if err != nil {
			fmt.Fprintf(os.Stderr, "client: error on read: %v\n", err)
			os.Exit(1)
		}
		fmt.Fprintf(os.Stderr, "read: [%c]!\n", b)
	}
}

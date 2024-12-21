package main

import (
	"bufio"
	"fmt"
	"os"
	"time"
)

func main() {
	t := time.NewTicker(time.Millisecond)
	go listen()
	for {
		fmt.Println(<-t.C)
	}
}
func listen() {
	// b := make([]byte, 4)
	s := bufio.NewScanner(os.Stdin)
	for {
		s.Scan()
		if len(s.Bytes()) == 0 {
			continue
		}
		fmt.Printf("read: [%s]!\n", s.Bytes())
	}
}

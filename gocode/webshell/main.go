package main

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"sync"
)

var b bytes.Buffer
var mu sync.Mutex

func main() {
	go read() //constantly read from client.c
	http.HandleFunc("/", defaultHandler)
	http.HandleFunc("/read", readHandler)
	http.HandleFunc("/input", inputHandler)
	log.Fatal(http.ListenAndServe(":8080", nil))
}

// read from client.c stdout into buffer
func read() {
	r := bufio.NewScanner(os.Stdin)
	for {
		r.Scan()
		mu.Lock()
		b.Write(r.Bytes())
		mu.Unlock()
	}
}

// send user the default webpage
func defaultHandler(w http.ResponseWriter, r *http.Request) {
	f, err := os.ReadFile("site.html")
	if err != nil {
		panic("couldn't read site.html")
	}
	w.Write(f)
}

// read buffer, send to back user
func readHandler(w http.ResponseWriter, r *http.Request) {
	mu.Lock()
	s, err := io.ReadAll(&b)
	mu.Unlock()
	if err != nil {
		fmt.Println("bad read from buffer")
		return
	}
	w.Write([]byte(s))
}

// write input from user to client.c (stdout)
func inputHandler(w http.ResponseWriter, r *http.Request) {
	r.ParseForm()
	s := r.Form.Get("data")
	fmt.Print(s)
}

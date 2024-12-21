package main

import (
	"bufio"
	"fmt"
	"log"
	"net/http"
	"os"
)

var stdin *bufio.Scanner
var f []byte

func main() {
	stdin = bufio.NewScanner(os.Stdin)
	http.HandleFunc("/", defaultHandler)
	http.HandleFunc("/read", readHandler)
	http.HandleFunc("/input", inputHandler)
	log.Fatal(http.ListenAndServe("localhost:8000", nil))
}

// site -> user
func defaultHandler(w http.ResponseWriter, r *http.Request) {
	var err error
	f, err = os.ReadFile("/Users/maxgara/Desktop/al-code/garage/gocode/webshell/simple/site.html")
	if err != nil {
		log.Fatal(err)
	}
	w.Write(f)
}

// client.c -> user
func readHandler(w http.ResponseWriter, r *http.Request) {
	ok := stdin.Scan() //<- client
	if !ok {
		fmt.Fprintf(os.Stderr, "scanner done.\n")
	}
	fmt.Fprintf(os.Stderr, "scanner scanned: [%s]\n", stdin.Bytes())
	w.Write(stdin.Bytes())
}

// user input -> client.c
func inputHandler(w http.ResponseWriter, r *http.Request) {
	err := r.ParseForm()
	if err != nil {
		fmt.Fprintf(os.Stderr, "err parsing input form:%v\n", err)
		return
	}
	fmt.Fprintf(os.Stderr, "input data:%v\n", r.Form.Get("data"))
	fmt.Print(r.Form.Get("data")) //-> client
}

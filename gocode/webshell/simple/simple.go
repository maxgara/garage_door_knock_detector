package main

import (
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"os/exec"
)

var cReader *os.File
var cWriter *os.File
var f []byte

func main() {
	//set up client.c and pipeline
	runproc()
	fmt.Println("started client.c")
	r, rerr := os.OpenFile("ctos", os.O_RDONLY, os.ModeNamedPipe)
	w, werr := os.OpenFile("stoc", os.O_WRONLY, os.ModeNamedPipe)
	fmt.Println("opened files")
	if rerr != nil || werr != nil {
		log.Fatal(errors.Join(rerr, werr))
	}
	_ = w
	_ = r
	for {
		// cWriter = w
		cReader = r
		s := readclient()
		if s != "" {
			fmt.Println(s)
		}
	}
	// //set up + run server
	// http.HandleFunc("/", defaultHandler)
	// http.HandleFunc("/read", readHandler)
	// http.HandleFunc("/input", inputHandler)
	// fmt.Println("setup complete: running ListenAndServe()")
	// log.Fatal(http.ListenAndServe("localhost:8000", nil))
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

func readclient() string {
	var b []byte
	n, err := cReader.Read(b)
	if err == io.EOF {
		fmt.Println(err)
		return string(b[:n])
	}
	if err != nil {
		fmt.Println(err)
		return ""
	}
	return string(b[:n])
}

// client.c -> user
func readHandler(w http.ResponseWriter, r *http.Request) {
	var b []byte
	n, err := cReader.Read(b)
	if err != nil && err != io.EOF {
		fmt.Printf("error: %v\n", err)
	}
	if n != 0 {
		fmt.Println("reading from creader.")
		w.Write(b)
	}
	// c := readbychar()
	// w.Write()
}

// user input -> client.c
func inputHandler(w http.ResponseWriter, r *http.Request) {
	err := r.ParseForm()
	if err != nil {
		fmt.Fprintf(os.Stderr, "err parsing input form:%v\n", err)
		return
	}
	fmt.Fprintf(os.Stderr, "input data:%v\n", r.Form.Get("data"))
	fmt.Fprintf(cWriter, "%s\n", r.Form.Get("data")) //write client
}

// text -> HTML
func format(arr []byte) []byte {
	nl := []byte("<br>")
	out := make([]byte, 0, len(arr))
	for _, c := range arr {
		if c == '\n' {
			out = append(out, nl...)
		}
	}
	return out
}

func runproc() {
	com := exec.Command("./startclientproc.sh")
	com.Start()
}

package main

import (
	"errors"
	"fmt"
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
	r, rerr := os.OpenFile("/Users/maxgara/Desktop/al-code/garage/gocode/webshell/simple/ctos", os.O_RDONLY|os.O_SYNC, os.ModeNamedPipe)

	w, werr := os.OpenFile("/Users/maxgara/Desktop/al-code/garage/gocode/webshell/simple/stoc", os.O_WRONLY|os.O_SYNC, os.ModeNamedPipe)
	fmt.Println("opened files")
	if rerr != nil || werr != nil {
		log.Fatal(errors.Join(rerr, werr))
	}
	cWriter = w
	cReader = r

	// //set up + run server
	http.HandleFunc("/", defaultHandler)
	http.HandleFunc("/read", readHandler)
	http.HandleFunc("/input", inputHandler)
	fmt.Println("setup complete: running ListenAndServe()")
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
	b := make([]byte, 256)
	n, err := cReader.Read(b)
	if n != 0 {
		fmt.Printf("read creader <- [%s]\n", b[:n])
		w.Write(b[:n])
	}
	if err != nil {
		fmt.Printf("error: %v\n", err)
	}
}

// user input -> client.c
func inputHandler(w http.ResponseWriter, r *http.Request) {
	err := r.ParseForm()
	if err != nil {
		fmt.Fprintf(os.Stderr, "err parsing input form:%v\n", err)
		return
	}
	data := r.Form.Get("data")
	fmt.Fprintf(os.Stderr, "input data:%v\n", data)
	fmt.Fprintf(cWriter, "%s\n", format([]byte(data))) //write client
}

// text -> HTML
func format(arr []byte) []byte {
	nl := []byte("<br>")
	out := make([]byte, 0, len(arr))
	for _, c := range arr {
		if c == '\n' {
			out = append(out, nl...)
		} else {
			out = append(out, c)
		}
	}
	return out
}

func runproc() {
	com := exec.Command("/bin/zsh", "./startclientproc.sh")
	com.Start()
}

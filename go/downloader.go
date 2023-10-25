package main

/*
	#include "downloader.h"
	#include <stdlib.h>
	#include <string.h>
*/
import "C"
import (
	"io"
	"net/http"
	"time"
	"unsafe"
)

//export hydra_download
func hydra_download(url *C.cchar_t) C.hydra_buffer_t {
	var buffer C.hydra_buffer_t
	buffer.data = nil
	buffer.size = 0

	goURL := C.GoString(url)
	client := http.Client{
		Timeout: 5 * time.Second,
	}
	response, err := client.Get(goURL)
	if err != nil {
		println("Go: " + err.Error())
		return buffer
	}
	defer response.Body.Close()

	b, err := io.ReadAll(response.Body)
	if err != nil {
		println("Go: " + err.Error())
		return buffer
	}

	if len(b) == 0 {
		println("Go: empty response")
		return buffer
	}

	buffer.data = C.malloc(C.size_t(len(b)))
	C.memcpy(buffer.data, unsafe.Pointer(&b[0]), C.size_t(len(b)))
	buffer.size = C.ulonglong(C.size_t(len(b)))
	return buffer
}

// For whatever reason, this is required to build with -buildmode=c-archive
func main() {}

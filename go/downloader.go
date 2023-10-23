package main

/*
	#include "downloader.h"
	#include <stdlib.h>
	#include <string.h>

	inline void* my_malloc(size_t size) { return malloc(size); }
*/
import "C"
import (
	"io"
	"net/http"
	"unsafe"
)

//export hydra_download
func hydra_download(url *C.cchar_t) C.hydra_buffer_t {
	var buffer C.hydra_buffer_t
	buffer.data = nil
	buffer.size = 0

	goURL := C.GoString(url)
	response, err := http.Get(goURL)
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

	buffer.data = C.my_malloc(C.size_t(response.ContentLength))
	C.memcpy(buffer.data, unsafe.Pointer(&b[0]), C.size_t(response.ContentLength))
	buffer.size = C.size_t(response.ContentLength)
	return buffer
}

// For whatever reason, this is required to build with -buildmode=c-archive
func main() {}

#include "io_manager.h"
#include "file_handle.h"
#include "page_handle.h"

#include <iostream>

int main() {
    io_manager* mgr = new io_manager();
    mgr->create_file("school");
    file_handle handle = mgr->open_file("school");

    handle.get_this_page(0);

    std::cout << *mgr << std::endl;

    // mgr->close_file(handle);
    // mgr->destroy_file("school");

}


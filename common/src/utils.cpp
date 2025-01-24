#include <utils/utils.h>
#include <fstream>
#include <cstdio>       
#include <cstdlib>      
#include <cstring>      
#include <sys/stat.h>   
#include <cmath>        
#include <vector>       
#include <cassert>      
#include <iostream>     

char* ReadBinaryFile(const char* pFilename, int& size) {
    FILE* f = fopen(pFilename, "rb");
    if (!f) {
        std::cerr << "Error opening '" << pFilename << "': " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    struct stat stat_buf;
    if (stat(pFilename, &stat_buf) != 0) {
        std::cerr << "Error getting file stats: " << strerror(errno) << std::endl;
        fclose(f);
        return nullptr;
    }

    size = stat_buf.st_size;

    char* p = static_cast<char*>(malloc(size));
    if (!p) {
        std::cerr << "Memory allocation failed!" << std::endl;
        fclose(f);
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(p, 1, size, f);
    if (bytes_read != size) {
        std::cerr << "Error reading file: " << strerror(errno) << std::endl;
        free(p);
        fclose(f);
        exit(EXIT_FAILURE);
    }

    fclose(f);

    return p;
}

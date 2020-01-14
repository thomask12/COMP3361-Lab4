/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: kt
 *
 * Created on April 29, 2019, 6:44 PM
 */

#include "BitMapAllocator.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::string;

namespace { // unnamed namespace
    // Convert bool to T/F
    static char BoolTF(bool x) {
        return x ? 'T' : 'F';
    }
}

int main(int argc, char* argv[]) {
    // Use command line argument as file name
    if (argc != 2) {
        std::cerr << "usage: lab3 input_file\n";
        exit(1);
    }
    // Open the input file.  Abort program if can't open.
    std::fstream input_file;
    input_file.open(argv[1], std::ios_base::in);
    if (!input_file.is_open()) {
        cerr << "ERROR: failed to open input file: " << argv[1] << "\n";
        exit(2);
    }

    // Read memory size in page frames (hexadecimal)
    string line;
    if (!std::getline(input_file, line)) {
        cerr << "ERROR: could not read first line of input file: " << argv[1] << "\n";
        exit(2);
    }
    cout << "+" << line << "\n";
    std::istringstream first_line_stream(line);
    uint32_t page_frame_count;
    first_line_stream >> std::hex >> page_frame_count;

    // Create memory and empty vector of allocated frames
    mem::MMU mem(page_frame_count);
    BitMapAllocator allocator(mem);
    std::vector<std::vector < uint32_t >> processes(4);

    // Read and execute additional allocate/deallocate lines
    while (std::getline(input_file, line)) {
        std::istringstream line_stream(line);
        cout << "+" << line << "\n";
        string operation;
        uint32_t count = 0;
        uint32_t process = 0xFFFFFFFF;
        line_stream >> std::hex >> operation;
        if (operation == "G" || operation == "F") {
            line_stream >> process;
            line_stream >> std::hex >> count;
        } else if (operation != "B" || line_stream.fail()) {
            cerr << "ERROR: could not decode input line\n";
            exit(2);
        }

        // Get page frames for process
        if (operation == "G") {
            bool op_status = allocator.GetFrames(count, processes.at(process));
            cout << " " << BoolTF(op_status) << " " << std::hex
                    << allocator.get_free_count() << "\n";
        } else if (operation == "F") {
            bool op_status = allocator.FreeFrames(count, processes.at(process));
            cout << " " << BoolTF(op_status) << " " << std::hex
                    << allocator.get_free_count() << "\n";
        } else {
            cout << allocator.get_bit_map_string() << "\n";
        }
    }

    if (!input_file.eof()) {
        cerr << "ERROR: could not read input file: " << argv[1] << "\n";
    }
}


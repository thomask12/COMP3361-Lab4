/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BitMapAllocator.cpp
 * Author: kt
 * 
 * Created on April 29, 2019, 6:56 PM
 */

#include <MMU.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "BitMapAllocator.h"

using namespace std;
static int frameSize = 0x400;

BitMapAllocator::BitMapAllocator(mem::MMU &mem) : memory(mem) {
    //Max page frames allowed
    for (int i = 1; i < memory.get_frame_count(); i++) {
        uint8_t memVal;
        mem::Addr address = i / 8;
        memory.movb(&memVal, address, sizeof (uint8_t));
        memVal |= 0x1 << i % 8;
        memory.movb(address, &memVal, sizeof (uint8_t));
    }
}

bool BitMapAllocator::GetFrames(uint32_t count, vector<uint32_t> &page_frames) {
    if (count > get_free_count()) {
        return false;
    }
    //first bit in memory that is available
    for (int i = 0; i < count; i++) {
        uint8_t memVal;
        mem::Addr address = first_free_bit();
        memory.movb(&memVal, address / 8, sizeof (uint8_t));
        memVal ^= 0x1 << (address % 8);
        memory.movb(address / 8, &memVal, sizeof (uint8_t));
        //Handles case for non-consecutive available memory
        page_frames.push_back((address) * frameSize);
    }
    return true;
}

bool BitMapAllocator::FreeFrames(uint32_t count, vector<uint32_t> &page_frames) {
    if (count > page_frames.size()) {
        return false;
    }
    for (int i = 0; i < count; i++) {
        //Finds where the specific page frame is located inside of the bitmap
        uint8_t memVal;
        mem::Addr address = page_frames[page_frames.size() - 1] / frameSize;
        memory.movb(&memVal, address / 8, sizeof (uint8_t));
        memVal |= 0x1 << (address % 8);
        memory.movb(address / 8, &memVal, sizeof (uint8_t));
        page_frames.pop_back();
    }
    return true;
}

uint32_t BitMapAllocator::get_free_count() {
    uint32_t totalFrames = (memory.get_frame_count());
    uint32_t count = 0;
    for (int i = 1; i < totalFrames; i++) {
        uint8_t memVal;
        mem::Addr address = i / 8;
        memory.movb(&memVal, address, sizeof (uint8_t));
        memVal = (memVal >> (i % 8)) & 0x1;
        if (memVal == 0x1) {
            count++;
        }
    }
    return count;
}

string BitMapAllocator::get_bit_map_string() {
    stringstream ss;
    for (int i = 0; i < 0x20; i++) {
        uint8_t memVal;
        mem::Addr address = i;
        memory.movb(&memVal, address, sizeof (uint8_t));
        ss << setfill('0') << setw(2) << hex << (int) memVal << " ";
    }
    return ss.str();
}

uint32_t BitMapAllocator::first_free_bit() {
    uint32_t totalFrames = memory.get_frame_count();
    uint32_t bit = 0;
    //Run through all frames
    for (int i = 1; i < totalFrames; i++) {
        uint8_t memVal;
        mem::Addr address = i / 8;
        memory.movb(&memVal, address, sizeof (uint8_t));
        memVal = (memVal >> (i % 8)) & 0x1;
        //bit is marked as available
        if (memVal == 0x1) {
            //free bit 
            bit = i;
            i = totalFrames;
        }
    }
    return bit;
}


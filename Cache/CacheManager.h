#ifndef CACHE_H
#define CACHE_H

#pragma once

#include <windows.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <algorithm>

class CacheManager {
public:
    CacheManager(size_t blockSize, size_t maxBlocks);

    void readBlock(HANDLE fileHandle, size_t blockIndex, char *buffer, size_t size);

    void writeBlock(HANDLE fileHandle, size_t blockIndex, const char *buffer, size_t size);

    void syncToDisk(HANDLE fileHandle);

    size_t blockSize;

private:
    struct Block {
        std::unique_ptr<char[]> data;
        size_t frequency;
    };

    size_t maxBlocks;
    std::unordered_map<HANDLE, std::unordered_map<size_t, Block>> fileCacheMap;

    void evictBlock(HANDLE fileHandle);

    static bool compareBlocks(const std::pair<const size_t, Block> &a, const std::pair<const size_t, Block> &b);
};

#endif // CACHE_H

#include "CacheManager.h"
#include <iostream>
#include <algorithm>
#include <cstring>

CacheManager::CacheManager(size_t blockSize, size_t maxBlocks)
        : blockSize(blockSize), maxBlocks(maxBlocks) {}

void CacheManager::readBlock(HANDLE fileHandle, size_t blockIndex, char *buffer, size_t size) {
    if (fileCacheMap[fileHandle].find(blockIndex) != fileCacheMap[fileHandle].end()) {
        auto &block = fileCacheMap[fileHandle][blockIndex];
        std::memcpy(buffer, block.data.get(), size);
        block.frequency++;
    } else {
        LARGE_INTEGER offset;
        offset.QuadPart = blockIndex * blockSize;

        SetFilePointerEx(fileHandle, offset, nullptr, FILE_BEGIN);

        DWORD bytesRead;
        if (!ReadFile(fileHandle, buffer, size, &bytesRead, nullptr)) {
            std::cerr << "Failed to read block from disk. Error: " << GetLastError() << std::endl;
        }

        writeBlock(fileHandle, blockIndex, buffer, bytesRead);
    }
}

void CacheManager::writeBlock(HANDLE fileHandle, size_t blockIndex, const char *buffer, size_t size) {
    if (fileCacheMap[fileHandle].size() >= maxBlocks) {
        evictBlock(fileHandle);
    }

    Block block;
    block.data = std::make_unique<char[]>(blockSize);
    std::memcpy(block.data.get(), buffer, size);
    block.frequency = 1;

    fileCacheMap[fileHandle][blockIndex] = std::move(block);
}

void CacheManager::syncToDisk(HANDLE fileHandle) {
    for (const auto &pair: fileCacheMap[fileHandle]) {
        size_t blockIndex = pair.first;
        const Block &block = pair.second;

        LARGE_INTEGER offset;
        offset.QuadPart = blockIndex * blockSize;

        SetFilePointerEx(fileHandle, offset, nullptr, FILE_BEGIN);

        DWORD bytesWritten;
        if (!WriteFile(fileHandle, block.data.get(), blockSize, &bytesWritten, nullptr)) {
            std::cerr << "Failed to write block to disk. Error: " << GetLastError() << std::endl;
        }
    }

    fileCacheMap[fileHandle].clear();
}

bool CacheManager::compareBlocks(const std::pair<const size_t, Block> &a, const std::pair<const size_t, Block> &b) {
    return a.second.frequency < b.second.frequency;
}

void CacheManager::evictBlock(HANDLE fileHandle) {
    auto &blocks = fileCacheMap[fileHandle];
    auto minFreqBlock = std::min_element(blocks.begin(), blocks.end(), compareBlocks);

    if (minFreqBlock != blocks.end()) {
        size_t blockIndex = minFreqBlock->first;
        const Block &block = minFreqBlock->second;

        LARGE_INTEGER offset;
        offset.QuadPart = blockIndex * blockSize;

        SetFilePointerEx(fileHandle, offset, nullptr, FILE_BEGIN);

        DWORD bytesWritten;
        if (!WriteFile(fileHandle, block.data.get(), blockSize, &bytesWritten, nullptr)) {
            std::cerr << "Failed to write block to disk. Error: " << GetLastError() << std::endl;
        }

        blocks.erase(minFreqBlock);
    }
}

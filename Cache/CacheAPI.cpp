#include "CacheAPI.h"
#include "CacheManager.h"
#include <unordered_map>
#include <iostream>
#include <cstring>

static std::unordered_map<int, HANDLE> fdToFileHandleMap;
static std::unordered_map<int, off_t> fdToCurrentOffsetMap;
static CacheManager cache(4096, 100);

CACHE_API int lab2_open(const char *path) {
    HANDLE fileHandle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                    OPEN_ALWAYS, FILE_FLAG_NO_BUFFERING, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file. Error: " << GetLastError() << std::endl;
        return -1;
    }

    int fd = static_cast<int>(fdToFileHandleMap.size()) + 1;
    fdToFileHandleMap[fd] = fileHandle;
    fdToCurrentOffsetMap[fd] = 0;
    return fd;
}

CACHE_API int lab2_close(int fd) {
    if (fdToFileHandleMap.find(fd) == fdToFileHandleMap.end()) {
        std::cerr << "Invalid file descriptor: " << fd << std::endl;
        return -1;
    }

    HANDLE fileHandle = fdToFileHandleMap[fd];
    cache.syncToDisk(fileHandle);
    CloseHandle(fileHandle);
    fdToFileHandleMap.erase(fd);
    fdToCurrentOffsetMap.erase(fd);
    return 0;
}

CACHE_API ssize_t lab2_read(int fd, void *buf, size_t count) {
    if (fdToFileHandleMap.find(fd) == fdToFileHandleMap.end()) {
        std::cerr << "Invalid file descriptor: " << fd << std::endl;
        return -1;
    }

    HANDLE fileHandle = fdToFileHandleMap[fd];
    off_t &currentOffset = fdToCurrentOffsetMap[fd];

    size_t bytesToRead = count;
    size_t blockIndex = currentOffset / cache.blockSize;
    size_t blockOffset = currentOffset % cache.blockSize;
    char tempBuffer[cache.blockSize];

    while (bytesToRead > 0) {
        size_t bytesFromBlock = (bytesToRead < cache.blockSize - blockOffset) ? bytesToRead : cache.blockSize -
                                                                                              blockOffset;
        cache.readBlock(fileHandle, blockIndex, tempBuffer, cache.blockSize);
        std::memcpy(static_cast<char *>(buf) + (count - bytesToRead), tempBuffer + blockOffset, bytesFromBlock);

        blockIndex++;
        blockOffset = 0;
        bytesToRead -= bytesFromBlock;
    }

    currentOffset += count;
    return count;
}

CACHE_API ssize_t lab2_write(int fd, const void *buf, size_t count) {
    if (fdToFileHandleMap.find(fd) == fdToFileHandleMap.end()) {
        std::cerr << "Invalid file descriptor: " << fd << std::endl;
        return -1;
    }

    HANDLE fileHandle = fdToFileHandleMap[fd];
    off_t &currentOffset = fdToCurrentOffsetMap[fd];

    size_t bytesToWrite = count;
    size_t blockIndex = currentOffset / cache.blockSize;
    size_t blockOffset = currentOffset % cache.blockSize;
    char tempBuffer[cache.blockSize];

    while (bytesToWrite > 0) {
        size_t bytesToBlock = (bytesToWrite < cache.blockSize - blockOffset) ? bytesToWrite : cache.blockSize -
                                                                                              blockOffset;
        cache.readBlock(fileHandle, blockIndex, tempBuffer, cache.blockSize);
        std::memcpy(tempBuffer + blockOffset, static_cast<const char *>(buf) + (count - bytesToWrite), bytesToBlock);
        cache.writeBlock(fileHandle, blockIndex, tempBuffer, cache.blockSize);

        blockIndex++;
        blockOffset = 0;
        bytesToWrite -= bytesToBlock;
    }

    currentOffset += count;
    return count;
}

CACHE_API off_t lab2_lseek(int fd, off_t offset, int whence) {
    if (fdToFileHandleMap.find(fd) == fdToFileHandleMap.end()) {
        std::cerr << "Invalid file descriptor: " << fd << std::endl;
        return -1;
    }

    HANDLE fileHandle = fdToFileHandleMap[fd];
    off_t &currentOffset = fdToCurrentOffsetMap[fd];
    off_t newOffset = 0;

    switch (whence) {
        case SEEK_SET:
            newOffset = offset;
            break;
        case SEEK_CUR:
            newOffset = currentOffset + offset;
            break;
        case SEEK_END:
            LARGE_INTEGER move;
            LARGE_INTEGER newPointer;
            move.QuadPart = 0;
            if (!SetFilePointerEx(fileHandle, move, &newPointer, FILE_END)) {
                std::cerr << "Failed to lseek. Error: " << GetLastError() << std::endl;
                return -1;
            }
            newOffset = newPointer.QuadPart + offset;
            break;
        default:
            std::cerr << "Invalid whence value" << std::endl;
            return -1;
    }

    currentOffset = newOffset;
    return newOffset;
}

CACHE_API int lab2_fsync(int fd) {
    if (fdToFileHandleMap.find(fd) == fdToFileHandleMap.end()) {
        std::cerr << "Invalid file descriptor: " << fd << std::endl;
        return -1;
    }

    HANDLE fileHandle = fdToFileHandleMap[fd];
    cache.syncToDisk(fileHandle);
    return 0;
}

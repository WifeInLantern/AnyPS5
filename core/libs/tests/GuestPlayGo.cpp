#include "SceTypes.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>

extern "C" {
int APS5_VABI scePlayGoInitialize(const PlayGoInitParams*);
int APS5_VABI scePlayGoOpen(int*, const void*);
int APS5_VABI scePlayGoClose(int);
int APS5_VABI scePlayGoGetLanguageMask(int, std::uint64_t*);
int APS5_VABI scePlayGoSetToDoList(int, const PlayGoToDo*, std::uint32_t);
int APS5_VABI scePlayGoGetToDoList(int, PlayGoToDo*, std::uint32_t, std::uint32_t*);
int APS5_VABI scePlayGoGetLocus(int, const std::uint16_t*, std::uint32_t, std::int8_t*);
int APS5_VABI scePlayGoGetChunkId(int, std::uint16_t*, std::uint32_t, std::uint32_t*);
int APS5_VABI scePlayGoGetInstallChunkId(int, std::uint16_t*, std::uint32_t, std::uint32_t*);
int APS5_VABI scePlayGoGetEta(int, const std::uint16_t*, std::uint32_t, std::int64_t*);
int APS5_VABI scePlayGoGetProgress(int, const std::uint16_t*, std::uint32_t, PlayGoProgress*);
int APS5_VABI scePlayGoGetInstallSpeed(int, std::int32_t*);
int APS5_VABI scePlayGoSetInstallSpeed(int, std::int32_t);
int APS5_VABI scePlayGoGetOptionalChunk(int, std::int32_t, PlayGoOptionalChunk*);
int APS5_VABI scePlayGoGetSupportedOptionalChunk(int, std::int32_t, PlayGoOptionalChunk*);
int APS5_VABI scePlayGoPrefetch(int, const std::uint16_t*, std::uint32_t, std::int8_t);
int APS5_VABI scePlayGoPrefetchOptionalChunk(int, std::int32_t, const PlayGoOptionalChunk*);
int APS5_VABI scePlayGoTerminate(void);
}

static void Require(bool value) { if (!value) std::abort(); }

int main() {
    constexpr int badHandle = static_cast<int>(0x80B20009);
    constexpr int badPointer = static_cast<int>(0x80B2000A);
    constexpr int badSize = static_cast<int>(0x80B2000B);
    constexpr int badChunkId = static_cast<int>(0x80B2000C);
    constexpr int badLocus = static_cast<int>(0x80B20010);

    PlayGoInitParams init{};
    int handle = 0;
    Require(scePlayGoInitialize(nullptr) == badPointer);
    Require(scePlayGoInitialize(&init) == 0);
    Require(scePlayGoOpen(nullptr, nullptr) == badPointer);
    Require(scePlayGoOpen(&handle, nullptr) == 0);

    std::uint64_t mask = 0;
    Require(scePlayGoGetLanguageMask(handle + 1, &mask) == badHandle);
    Require(scePlayGoGetLanguageMask(handle, nullptr) == badPointer);
    Require(scePlayGoGetLanguageMask(handle, &mask) == 0 && mask == ~0ull);

    PlayGoToDo todo[2] = {{0, 3, 0}, {0, 0, 0}};
    Require(scePlayGoSetToDoList(handle + 1, todo, 2) == badHandle);
    Require(scePlayGoSetToDoList(handle, nullptr, 2) == badPointer);
    Require(scePlayGoSetToDoList(handle, todo, 0) == badSize);
    Require(scePlayGoSetToDoList(handle, todo, 2) == 0);
    todo[1].locus = 1;
    Require(scePlayGoSetToDoList(handle, todo, 2) == badLocus);
    todo[1] = {0xFFFF, 3, 0};
    Require(scePlayGoSetToDoList(handle, todo, 2) == badChunkId);

    std::uint32_t entries = 1;
    Require(scePlayGoGetToDoList(handle, todo, 2, &entries) == 0 && entries == 0);
    const std::uint16_t chunk = 0;
    std::int8_t locus = 0;
    Require(scePlayGoGetLocus(handle, &chunk, 0, &locus) == badSize);
    Require(scePlayGoGetLocus(handle, &chunk, 1, &locus) == 0 && locus == 3);
    const std::uint16_t unknownChunk = 0xFFFF;
    Require(scePlayGoGetLocus(handle, &unknownChunk, 1, &locus) == badChunkId);
    Require(scePlayGoGetLocus(handle, nullptr, 1, &locus) == badPointer);
    Require(scePlayGoGetLocus(handle, &chunk, 1, nullptr) == badPointer);

    std::uint16_t chunkIds[4] = {0xAAAA, 0xAAAA, 0xAAAA, 0xAAAA};
    entries = 99;
    Require(scePlayGoGetChunkId(handle + 1, chunkIds, 4, &entries) == badHandle);
    Require(scePlayGoGetChunkId(handle, chunkIds, 4, nullptr) == badPointer);
    Require(scePlayGoGetChunkId(handle, nullptr, 0, &entries) == 0 && entries == 1);
    Require(scePlayGoGetChunkId(handle, chunkIds, 0, &entries) == 0 && entries == 0 && chunkIds[0] == 0xAAAA);
    Require(scePlayGoGetChunkId(handle, chunkIds, 4, &entries) == 0 && entries == 1 && chunkIds[0] == 0 && chunkIds[1] == 0xAAAA);
    chunkIds[0] = 0xAAAA;
    Require(scePlayGoGetInstallChunkId(handle + 1, chunkIds, 4, &entries) == badHandle);
    Require(scePlayGoGetInstallChunkId(handle, chunkIds, 4, &entries) == 0 && entries == 1 && chunkIds[0] == 0);

    std::int64_t eta = -1;
    Require(scePlayGoGetEta(handle + 1, &chunk, 1, &eta) == badHandle);
    Require(scePlayGoGetEta(handle, &chunk, 1, nullptr) == badPointer);
    Require(scePlayGoGetEta(handle, nullptr, 1, &eta) == badPointer);
    Require(scePlayGoGetEta(handle, &chunk, 0, &eta) == badSize);
    Require(scePlayGoGetEta(handle, &unknownChunk, 1, &eta) == badChunkId);
    Require(eta == -1);
    Require(scePlayGoGetEta(handle, &chunk, 1, &eta) == 0 && eta == 0);

    PlayGoProgress progress{0, 0};
    Require(scePlayGoGetProgress(handle + 1, &chunk, 1, &progress) == badHandle);
    Require(scePlayGoGetProgress(handle, &chunk, 1, nullptr) == badPointer);
    Require(scePlayGoGetProgress(handle, nullptr, 1, &progress) == badPointer);
    Require(scePlayGoGetProgress(handle, &chunk, 0, &progress) == badSize);
    Require(scePlayGoGetProgress(handle, &unknownChunk, 1, &progress) == badChunkId);
    Require(scePlayGoGetProgress(handle, &chunk, 1, &progress) == 0 && progress.progress_size == progress.total_size && progress.total_size != 0);

    std::int32_t speed = -1;
    Require(scePlayGoGetInstallSpeed(handle + 1, &speed) == badHandle);
    Require(scePlayGoGetInstallSpeed(handle, nullptr) == badPointer);
    Require(scePlayGoGetInstallSpeed(handle, &speed) == 0 && speed == 2);
    Require(scePlayGoSetInstallSpeed(handle + 1, 1) == badHandle);
    Require(scePlayGoSetInstallSpeed(handle, 1) == 0);
    Require(scePlayGoGetInstallSpeed(handle, &speed) == 0 && speed == 1);
    Require(scePlayGoSetInstallSpeed(handle, 2) == 0);
    Require(scePlayGoGetInstallSpeed(handle, &speed) == 0 && speed == 2);

    PlayGoOptionalChunk optional{};
    optional.bitmask = ~0ull;
    Require(scePlayGoGetOptionalChunk(handle + 1, 0, &optional) == badHandle);
    Require(scePlayGoGetOptionalChunk(handle, 0, nullptr) == badPointer);
    Require(scePlayGoGetOptionalChunk(handle, 0, &optional) == 0 && optional.bitmask == 0);
    optional.bitmask = ~0ull;
    Require(scePlayGoGetSupportedOptionalChunk(handle + 1, 0, &optional) == badHandle);
    Require(scePlayGoGetSupportedOptionalChunk(handle, 0, nullptr) == badPointer);
    Require(scePlayGoGetSupportedOptionalChunk(handle, 0, &optional) == 0 && optional.bitmask == 0);
    Require(scePlayGoPrefetchOptionalChunk(handle + 1, 0, &optional) == badHandle);
    Require(scePlayGoPrefetchOptionalChunk(handle, 0, &optional) == 0);

    Require(scePlayGoPrefetch(handle + 1, &chunk, 1, 3) == badHandle);
    Require(scePlayGoPrefetch(handle, nullptr, 1, 3) == badPointer);
    Require(scePlayGoPrefetch(handle, &chunk, 0, 3) == badSize);
    Require(scePlayGoPrefetch(handle, &unknownChunk, 1, 3) == badChunkId);
    Require(scePlayGoPrefetch(handle, &chunk, 1, 3) == 0);

    Require(scePlayGoClose(handle + 1) == badHandle);
    Require(scePlayGoClose(handle) == 0);
    Require(scePlayGoTerminate() == 0);
}

//
// Created by Stepan Usatiuk on 12.08.2023.
//

#include "memman.hpp"
#include "LockGuard.hpp"
#include "Spinlock.hpp"
#include "assert.h"
#include "misc.h"
#include "mutex.hpp"
#include "paging.hpp"
#include <stddef.h>


#define MAXGB       32ULL
#define BITMAP_SIZE (((MAXGB) * 1024ULL * 1024ULL) / (16ULL))
#define MAX_PID     (((BITMAP_SIZE) * 4) - 4)
// Expected to be nulled by the bootloader
static struct FourPages used_bitmap[BITMAP_SIZE];

static Mutex memman_lock;

static uint64_t maxPid   = 0; // Past the end
static uint64_t minPid   = 0;
static uint64_t totalMem = 0; // Past the end

static uint64_t roundup4k(uint64_t addr) {
    if ((addr & 0xFFF) == 0) return addr;
    else {
        return (addr + 0x1000) & (~(0xFFFULL));
    }
}

static uint64_t rounddown4k(uint64_t addr) {
    if ((addr & 0xFFF) == 0) return addr;
    else {
        return (addr) & (~(0xFFFULL));
    }
}

void setSts(uint64_t pid, enum PageStatus sts) {
    uint64_t rounddown = pid & (~(0b11ULL));
    uint64_t idx       = rounddown >> 2;
    switch (pid & 0b11ULL) {
        case 0:
            used_bitmap[idx].first = sts;
            break;
        case 1:
            used_bitmap[idx].second = sts;
            break;
        case 2:
            used_bitmap[idx].third = sts;
            break;
        case 3:
            used_bitmap[idx].fourth = sts;
            break;
    }
}

enum PageStatus getSts(uint64_t pid) {
    uint64_t rounddown = pid & (~(0b11ULL));
    uint64_t idx       = rounddown >> 2;
    switch (pid & 0b11ULL) {
        case 0:
            return used_bitmap[idx].first;
        case 1:
            return used_bitmap[idx].second;
        case 2:
            return used_bitmap[idx].third;
        case 3:
            return used_bitmap[idx].fourth;
    }
    assert2(0, "Error");
}

void parse_limine_memmap(struct limine_memmap_entry *entries, unsigned int num, uint64_t what_is_considered_free) {
    struct limine_memmap_entry *entry = entries;
    for (unsigned int i = 0; i < num; i++, entry++) {
        if (entry->type != what_is_considered_free) continue;
        uint64_t roundbase = roundup4k(entry->base);
        if (roundbase >= (entry->base + entry->length)) continue;
        if (entry->length <= (roundbase - entry->base)) continue;
        uint64_t len = rounddown4k(entry->length - (roundbase - entry->base));
        if (len == 0) continue;

        uint64_t pid = roundbase >> 12;
        if (minPid == 0 || pid < minPid) minPid = pid;
        uint64_t pidend = (roundbase + len) >> 12;
        if (pidend >= MAX_PID) pidend = MAX_PID - 1;
        for (uint64_t cp = pid; cp < pidend; cp++)
            if (getSts(cp) != MEMMAN_STATE_USED)
                setSts(cp, MEMMAN_STATE_FREE);
        totalMem += (pidend - pid) * 4;
        if (pidend > maxPid) maxPid = pidend;
    }
}

void *get4k() {
    LockGuard l(memman_lock);
    if (totalMem == 0) return NULL;

    uint64_t curPid = minPid;
    while (getSts(curPid) != MEMMAN_STATE_FREE && curPid < maxPid)
        minPid = curPid++;

    if (curPid >= maxPid) return NULL;

    totalMem -= 4;
    assert2(getSts(curPid) == MEMMAN_STATE_FREE, "Sanity check");
    setSts(curPid, MEMMAN_STATE_USED);
    return (void *) (HHDM_P2V(curPid << 12));
}

void free4k(void *page) {
    LockGuard l(memman_lock);
    if ((uint64_t) page >= HHDM_BEGIN) page = (void *) HHDM_V2P(page);
    else
        assert2(0, "Tried to free memory not in HHDM!");
    uint64_t roundbase = rounddown4k((uint64_t) page);
    assert2(((uint64_t) page == roundbase), "Tried to free unaligned memory!");

    uint64_t pid = (uint64_t) page >> 12;
    assert2(getSts(pid) == MEMMAN_STATE_USED, "Tried to free memory not allocated by the allocator!");
    setSts(pid, MEMMAN_STATE_FREE);
    totalMem += 4;
    if (minPid > pid) minPid = pid;
}

uint64_t get_free() {
    return totalMem;
}

/*
 * nth_prime_wasm.cpp
 *
 * WASM module for computing the nth prime using a precomputed pi(x) table.
 *
 * Algorithm:
 *   1. Load pi.dat — a table of cumulative prime counts at regular intervals
 *   2. Binary search the table to find which block contains the nth prime
 *   3. Segmented sieve within that block to locate the exact prime
 *
 * Exported functions:
 *   init_table(ptr, len) — parse pi.dat from a memory buffer
 *   query_nth_prime(n_lo, n_hi) — find the nth prime (n passed as two u32s)
 *   get_table_max_n()    — max supported n
 *   get_table_max_x()    — max x covered by the table
 *   get_result_lo/hi()   — retrieve 64-bit result as two u32s
 *   get_time_us()        — query time in microseconds
 */

#include <cstdint>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>

extern "C" {

using u64 = uint64_t;
using u32 = uint32_t;

// ---------- Table state ----------
static u64 g_max_x   = 0;
static u64 g_stride   = 0;
static u64 g_records  = 0;
static u64* g_pi      = nullptr;

// ---------- Result state ----------
static u64 g_result   = 0;
static u32 g_time_us  = 0;
static int g_error    = 0;  // 0 = ok, 1 = n=0, 2 = n too large, 3 = not found

// ---------- Small primes cache ----------
static std::vector<u32> g_small_primes;
static u64 g_small_primes_limit = 0;

void build_small_primes(u64 limit) {
    if (limit <= g_small_primes_limit) return;
    g_small_primes.clear();
    std::vector<char> sieve(limit + 1, 1);
    sieve[0] = sieve[1] = 0;
    for (u64 i = 2; i * i <= limit; i++) {
        if (sieve[i]) {
            for (u64 j = i * i; j <= limit; j += i)
                sieve[j] = 0;
        }
    }
    for (u64 i = 2; i <= limit; i++) {
        if (sieve[i]) g_small_primes.push_back((u32)i);
    }
    g_small_primes_limit = limit;
}

// ---------- Exported functions ----------

int init_table(const uint8_t* ptr, int len) {
    if (len < 64) return -1;

    // Verify magic
    if (memcmp(ptr, "PIFIN01\0", 8) != 0) return -2;

    memcpy(&g_max_x,   ptr + 8,  8);
    memcpy(&g_stride,  ptr + 16, 8);
    memcpy(&g_records, ptr + 24, 8);

    u64 expected = 64 + g_records * 8;
    if ((u64)len < expected) return -3;

    // Point directly into the buffer (caller must keep it alive)
    g_pi = (u64*)(ptr + 64);

    // Precompute small primes up to sqrt(max_x)
    u64 root = (u64)(sqrt((double)g_max_x) + 2);
    build_small_primes(root);

    return 0;
}

u32 get_table_max_n_lo()  { return (u32)(g_pi ? g_pi[g_records - 1] : 0); }
u32 get_table_max_n_hi()  { return (u32)((g_pi ? g_pi[g_records - 1] : 0) >> 32); }
u32 get_table_max_x_lo()  { return (u32)g_max_x; }
u32 get_table_max_x_hi()  { return (u32)(g_max_x >> 32); }
u32 get_stride()           { return (u32)g_stride; }
u32 get_num_records()      { return (u32)g_records; }
u32 get_result_lo()        { return (u32)g_result; }
u32 get_result_hi()        { return (u32)(g_result >> 32); }
u32 get_time_us()          { return g_time_us; }
int get_error()            { return g_error; }

u64 find_nth_inside_block(u64 L, u64 R, u64 need) {
    u64 len = R - L + 1;
    std::vector<char> is_prime(len, 1);

    if (L == 0) {
        if (len > 0) is_prime[0] = 0;
        if (len > 1) is_prime[1] = 0;
    } else if (L == 1) {
        is_prime[0] = 0;
    }

    // Use precomputed small primes
    for (u32 p : g_small_primes) {
        u64 pp = (u64)p * p;
        if (pp > R) break;
        u64 start = (pp >= L) ? pp : ((L + p - 1) / p) * p;
        for (u64 x = start; x <= R; x += p) {
            is_prime[x - L] = 0;
        }
    }

    u64 count = 0;
    for (u64 i = 0; i < len; i++) {
        if (is_prime[i]) {
            count++;
            if (count == need)
                return L + i;
        }
    }
    return 0; // not found
}

void query_nth_prime(u32 n_lo, u32 n_hi) {
    u64 n = ((u64)n_hi << 32) | n_lo;

    g_result = 0;
    g_time_us = 0;
    g_error = 0;

    if (n == 0) {
        g_error = 1;
        return;
    }

    u64 max_n = g_pi[g_records - 1];
    if (n > max_n) {
        g_error = 2;
        return;
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    // Binary search: find first index where pi[idx] >= n
    u64 lo = 0, hi = g_records - 1;
    while (lo < hi) {
        u64 mid = lo + (hi - lo) / 2;
        if (g_pi[mid] < n)
            lo = mid + 1;
        else
            hi = mid;
    }

    u64 idx = lo;
    u64 prev_pi = (idx == 0) ? 0 : g_pi[idx - 1];
    u64 L = idx * g_stride + 1;
    u64 R = (idx + 1) * g_stride;
    u64 need = n - prev_pi;

    u64 ans = find_nth_inside_block(L, R, need);

    auto t2 = std::chrono::high_resolution_clock::now();
    g_time_us = (u32)std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    if (ans == 0) {
        g_error = 3;
        return;
    }

    g_result = ans;
}

} // extern "C"

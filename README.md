# p(n) — Nth Prime Calculator

A fast, browser-based nth prime calculator powered by WebAssembly. Given any `n` from 1 to 73,302,249,005, it returns the exact nth prime number — covering all primes up to 2 trillion.

**[Live Demo →](https://p-santhiagu.github.io/nth-prime/)**

## How it works

```
 Input: n
   │
   ▼
 ┌─────────────────────────────────────────┐
 │  pi.dat — precomputed π(x) table        │
 │  66,667 entries at 30M intervals         │
 │  π(30M), π(60M), π(90M), ... π(2×10¹²) │
 └────────────────┬────────────────────────┘
                  │ binary search
                  ▼
        ┌─────────────────┐
        │  Block [L, R]   │
        │  width = 30M    │
        │  need = n - π(L)│
        └────────┬────────┘
                 │ segmented sieve
                 ▼
          ┌──────────────┐
          │  p(n) found  │
          └──────────────┘
```

1. **Precomputed table** (`pi.dat`, 521 KB) — stores cumulative prime counts at every 30,000,000 interval up to 2×10¹².
2. **Binary search** — locates which 30M-wide block contains the nth prime. O(log 66667) = ~17 comparisons.
3. **Segmented sieve** — runs Eratosthenes within that single 30M block to find the exact prime. Small primes up to √(2×10¹²) ≈ 1.41M are precomputed once at startup.

Typical query time: **under 1 second** in the browser, **under 100ms** via the CLI.

## Project structure

```
nth-prime/
├── nth_prime.cpp          # CLI tool (C++)
├── nth_prime_wasm.cpp     # WASM module (C++, Emscripten)
├── build_wasm.sh          # Build script for WASM
├── docs/
│   ├── index.html         # Web UI
│   ├── pi.dat             # Precomputed table (521 KB)
│   ├── nth_prime.js       # WASM glue (generated)
│   └── nth_prime.wasm     # WASM binary (generated)
├── README.md
├── LICENSE
└── .gitignore
```

## Quick start

### Web (GitHub Pages)

1. **Build the WASM module:**

```bash
# Install Emscripten (one-time)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source emsdk_env.sh

# Build
cd nth-prime
chmod +x build_wasm.sh
./build_wasm.sh
```

2. **Test locally:**

```bash
cd docs && python3 -m http.server 8080
# Open http://localhost:8080
```

3. **Deploy to GitHub Pages:**

```bash
git init && git add . && git commit -m "nth prime calculator"
gh repo create nth-prime --public --push
```

Then: Settings → Pages → Source → Deploy from branch → `main` → `/docs`.

### CLI

```bash
g++ -O2 -o nth_prime nth_prime.cpp
./nth_prime 1000000000 docs/pi.dat
# n:         1000000000
# nth prime: 22801763489
# time:      0.085234 sec
```

## pi.dat format

| Offset | Size | Field     | Description                          |
|--------|------|-----------|--------------------------------------|
| 0      | 8    | magic     | `PIFIN01\0`                          |
| 8      | 8    | max_x     | Maximum x covered (2,000,009,999,999)|
| 16     | 8    | stride    | Interval between records (30,000,000)|
| 24     | 8    | records   | Number of entries (66,667)           |
| 32     | 32   | reserved  | Padding to 64-byte header            |
| 64     | 8×N  | pi[]      | Cumulative prime counts (u64 array)  |

Total file size: 64 + 66,667 × 8 = 533,400 bytes (521 KB).

## Known values for verification

| n               | p(n)              |
|-----------------|-------------------|
| 1               | 2                 |
| 1,000           | 7,919             |
| 1,000,000       | 15,485,863        |
| 1,000,000,000   | 22,801,763,489    |
| 10,000,000,000  | 252,097,800,623   |
| 50,000,000,000  | 1,371,580,099,837 |
| 73,302,249,005  | 1,999,999,999,993 |

The last entry confirms the table boundary: the 73,302,249,005th prime is 1,999,999,999,993 — the largest prime below 2×10¹².

## License

MIT License. See [LICENSE](LICENSE) for details.

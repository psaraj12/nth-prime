# p(n) — Nth Prime Calculator

Given any `n` from 1 to 73,302,249,005, it returns the exact nth prime number — covering all primes up to 2 trillion.

**[Live Demo →](https://psaraj12.github.io/nth-prime/)**

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
                 │ sub-block segmented sieve
                 │ (2M chunks, odd-only)
                 ▼
          ┌──────────────┐
          │  p(n) found  │
          └──────────────┘
```

1. **Precomputed table** (`pi.dat`, 521 KB) — stores cumulative prime counts at every 30,000,000 interval up to 2×10¹².
2. **Binary search** — locates which 30M-wide block contains the nth prime. O(log 66667) = ~17 comparisons.
3. **Sub-block segmented sieve** — runs Eratosthenes in 2M chunks (odd-only representation) within the 30M block. Small primes up to √(2×10¹²) ≈ 1.41M are precomputed once at startup.

Typical query time: **under 300ms** in the browser, **under 100ms** via the CLI.

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

Settings → Pages → Source → Deploy from branch → `main` → `/docs`

### CLI

```bash
g++ -O3 -o nth_prime nth_prime.cpp
./nth_prime 1000000000 pi.dat
```

## Example queries

| n | p(n) | Time (WASM) |
|---|------|-------------|
| 1,000 | 7,919 | < 1ms |
| 1,000,000 | 15,485,863 | ~5ms |
| 1,000,000,000 | 22,801,763,489 | ~150ms |
| 10,000,000,000 | 252,097,800,623 | ~270ms |
| 73,302,249,005 | 1,999,999,999,981 | ~280ms |

## Changes

- **v2**: Sub-block segmented sieve (2M chunks, odd-only representation) for better L2/L3 cache utilisation
- **v1**: Initial release with full-block segmented sieve

## License

MIT

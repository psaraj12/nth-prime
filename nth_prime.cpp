#include <bits/stdc++.h>
using namespace std;

using u64 = uint64_t;

struct PiData {
    u64 max_x;
    u64 stride;
    u64 records;
    vector<u64> pi;
};

PiData load_pi_dat(const string& file) {
    ifstream in(file, ios::binary);
    if (!in) {
        cerr << "Cannot open file\n";
        exit(1);
    }

    char magic[8];
    PiData d;

    in.read(magic, 8);
    if (memcmp(magic, "PIFIN01\0", 8) != 0) {
        cerr << "Invalid pi.dat file\n";
        exit(1);
    }

    in.read((char*)&d.max_x, 8);
    in.read((char*)&d.stride, 8);
    in.read((char*)&d.records, 8);

    // skip remaining header bytes up to 64
    in.seekg(64, ios::beg);

    d.pi.resize(d.records);
    in.read((char*)d.pi.data(), d.records * sizeof(u64));

    return d;
}

vector<int> simple_primes(u64 limit) {
    vector<char> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;

    for (u64 i = 2; i * i <= limit; i++) {
        if (is_prime[i]) {
            for (u64 j = i * i; j <= limit; j += i)
                is_prime[j] = false;
        }
    }

    vector<int> primes;
    for (u64 i = 2; i <= limit; i++) {
        if (is_prime[i]) primes.push_back((int)i);
    }

    return primes;
}

u64 find_nth_inside_block(u64 L, u64 R, u64 need) {
    u64 len = R - L + 1;
    vector<char> is_prime(len, true);

    if (L == 0) {
        if (len > 0) is_prime[0] = false;
        if (len > 1) is_prime[1] = false;
    } else if (L == 1) {
        is_prime[0] = false;
    }

    u64 root = sqrt((long double)R) + 1;
    auto primes = simple_primes(root);

    for (u64 p : primes) {
        u64 start = max(p * p, ((L + p - 1) / p) * p);

        for (u64 x = start; x <= R; x += p) {
            is_prime[x - L] = false;
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

    cerr << "Prime not found inside block\n";
    exit(1);
}

u64 nth_prime(u64 n, const PiData& d) {
    if (n == 0) {
        cerr << "n must be >= 1\n";
        exit(1);
    }

    if (n > d.pi.back()) {
        cerr << "n is beyond this pi.dat range\n";
        cerr << "Max supported n = " << d.pi.back() << "\n";
        exit(1);
    }

    auto it = lower_bound(d.pi.begin(), d.pi.end(), n);
    size_t idx = it - d.pi.begin();

    u64 prev_pi = (idx == 0) ? 0 : d.pi[idx - 1];

    u64 L = idx * d.stride + 1;
    u64 R = (idx + 1) * d.stride;

    u64 need = n - prev_pi;

    return find_nth_inside_block(L, R, need);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Usage:\n";
        cerr << "  " << argv[0] << " <n> <pi.dat>\n\n";
        cerr << "Example:\n";
        cerr << "  " << argv[0] << " 5040197381 pi.dat\n";
        return 1;
    }

    u64 n = stoull(argv[1]);
    string file = argv[2];

    auto d = load_pi_dat(file);

    auto t1 = chrono::high_resolution_clock::now();

    u64 ans = nth_prime(n, d);

    auto t2 = chrono::high_resolution_clock::now();
    double sec = chrono::duration<double>(t2 - t1).count();

    cout << "n:         " << n << "\n";
    cout << "nth prime: " << ans << "\n";
    cout << "time:      " << fixed << setprecision(6) << sec << " sec\n";

    return 0;
}

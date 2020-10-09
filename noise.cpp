#include <bits/stdc++.h>
#include <x86intrin.h>
#include<immintrin.h>
using namespace std;

namespace Additional_SIMD_Operator{
    // Barrett Reduction : https://arxiv.org/ftp/arxiv/papers/1407/1407.3383.pdf
    struct Mod {
        int m, t, s, n;
        Mod(int MOD): m(MOD), t(32-__builtin_clz(m)+1), s(std::max(31-__builtin_clz(m-1), 0)), n((1ULL<<s+t)/m) {}
    };
    inline __m256i _mm256_mulmod_epi32(__m256i const x, __m256i const y, Mod const md) {
        auto const p=_mm256_set1_epi32(md.m);
        auto const q=_mm256_set1_epi64x(md.n);
        auto const al=_mm256_mul_epu32(x, y);
        auto const bl=_mm256_srli_epi64(al, md.s);
        auto const cl=_mm256_srli_epi64(_mm256_mul_epi32(bl, q), md.t);
        auto const ah=_mm256_mul_epu32(_mm256_srli_si256(x, 4), _mm256_srli_si256(y, 4));
        auto const bh=_mm256_srli_epi64(ah, md.s);
        auto const ch=_mm256_srli_epi64(_mm256_mul_epi32(bh, q), md.t);
        auto const dl=_mm256_sub_epi64(al, _mm256_mul_epi32(cl, p));
        auto const dh=_mm256_sub_epi64(ah, _mm256_mul_epi32(ch, p));
        auto const d=_mm256_or_si256(dl, _mm256_slli_si256(dh, 4));
        return _mm256_min_epu32(d, _mm256_sub_epi32(d, p));
    }
    // bitwise shift : https://stackoverflow.com/questions/17610696/shift-a-m128i-of-n-bits
    inline __m256i _mm256_lshift_epi32(__m256i const &v, int const n){
        __m256i v1, v2;
        if(n >= 32){
            v1 = _mm256_slli_si256(v, 8);
            v1 = _mm256_slli_epi32(v1, n-32);
        }
        else{
            v1 = _mm256_slli_epi32(v, n);
            v2 = _mm256_slli_si256(v, 8);
            v2 = _mm256_srli_epi32(v2, 32-n);
            v1 = _mm256_or_si256(v1, v2);
        }
        return v1;
    }
    inline __m256i _mm256_rshift_epi32(__m256i const &v, int const n){
        __m256i v1, v2;
        if(n >= 32){
            v1 = _mm256_srli_si256(v, 8);
            v1 = _mm256_srli_epi64(v1, n-32);
        }
        else{
            v1 = _mm256_srli_epi32(v, n);
            v2 = _mm256_srli_si256(v, 8);
            v2 = _mm256_slli_epi32(v2, 32-n);
            v1 = _mm256_or_si256(v1, v2);
        }
        return v1;
    }
    // type casting
    inline __m256i _mm256_convert_ps_epi32(__m256 v){
        float t1[8]; int t2[8];
        v = _mm256_floor_ps(v);
        _mm256_storeu_ps(t1, v);
        for(int i=0; i<8; i++) t2[i] = t1[i];
        return _mm256_load_si256((__m256i*)t2);
    }
}
using namespace Additional_SIMD_Operator;

namespace NoiseOperation{
    inline __m256 fade(__m256 const &vec){
        __m256 res = vec;
        res = _mm256_mul_ps(res, _mm256_set1_ps(6));
        res = _mm256_add_ps(res, _mm256_set1_ps(-15));
        res = _mm256_mul_ps(res, vec);
        res = _mm256_add_ps(res, _mm256_set1_ps(10));
        res = _mm256_mul_ps(res, vec);
        res = _mm256_mul_ps(res, vec);
        res = _mm256_mul_ps(res, vec);
        return res;
    }
    inline __m256 lerp(__m256 const &t, __m256 const &a, __m256 const &b){
        __m256 res = _mm256_sub_ps(b, a);
        res = _mm256_mul_ps(res, t);
        return _mm256_add_ps(res, a);
    }
    inline __m256i hashing(__m256i seed, __m256i a, __m256i b){
        seed = _mm256_xor_si256(seed, a);
        seed = _mm256_xor_si256(seed, b);
        seed = _mm256_mul_epi32(seed, _mm256_set1_epi32(0x27d4eb2d));
        seed = _mm256_xor_si256(_mm256_rshift_epi32(seed, 15), seed);
        return seed;
    }
    inline __m256 grad(__m256i const &h, __m256 const &x, __m256 const &y){
        const float RT2 = 1.4142135;
        __m256 gx = _mm256_permutevar8x32_ps(_mm256_setr_ps(1+RT2, -1-RT2, 1+RT2, -1-RT2, 1, -1, 1, -1), h);
        __m256 gy = _mm256_permutevar8x32_ps(_mm256_setr_ps(1, 1, -1, -1, 1+RT2, 1+RT2, -1-RT2, -1-RT2), h);
        __m256 X = _mm256_mul_ps(x, gx);
        __m256 Y = _mm256_mul_ps(y, gy);
        return _mm256_add_ps(X, Y);
    }
    inline __m256 generateNoise(__m256i seed, __m256 x, __m256 y){
        __m256 xs = _mm256_floor_ps(x);
        __m256 ys = _mm256_floor_ps(y);
        const __m256i xHash = _mm256_set1_epi32(0x1dde90c9);
        const __m256i yHash = _mm256_set1_epi32(0x43c42e4d);
        __m256i x0 = _mm256_mul_epi32(_mm256_convert_ps_epi32(xs), xHash);
        __m256i y0 = _mm256_mul_epi32(_mm256_convert_ps_epi32(ys), yHash);
        __m256i x1 = _mm256_add_epi32(x0, xHash);
        __m256i y1 = _mm256_add_epi32(y0, yHash);
        __m256 xf0 = xs = _mm256_sub_ps(x, xs);
        __m256 yf0 = ys = _mm256_sub_ps(y, ys);
        __m256 xf1 = _mm256_sub_ps(xs, _mm256_set1_ps(1));
        __m256 yf1 = _mm256_sub_ps(ys, _mm256_set1_ps(1));
        xs = fade(xs); ys = fade(ys);
        __m256 ret = lerp(
                lerp(grad(hashing(seed, x0, y0), xf0, yf0), grad(hashing(seed, x1, y0), xf1, yf0), xs),
                lerp(grad(hashing(seed, x0, y1), xf0, yf1), grad(hashing(seed, x1, y1), xf1, yf1), xs),
                ys
        );
        ret = _mm256_mul_ps(ret, _mm256_set1_ps(0.579106986522674560546875f));
        return ret;
    }
}
using namespace NoiseOperation;

int main(){
    float arr[80], tmp[8];
    __m256i seed; _mm256_set1_epi32((unsigned)chrono::steady_clock::now().time_since_epoch().count());
    __m256 x_pos, y_pos = _mm256_set1_ps(1);
    for(int i=0; i<80; i++) arr[i] = 0.0000001 * i;
    for(int i=0; i<80; i+=8){
        x_pos = _mm256_load_ps(tmp+i);
        _mm256_store_ps(tmp, generateNoise(seed, x_pos, y_pos));
        for(int j=0; j<8; j++) cout << fixed << setprecision(10) << tmp[j] << "\n";
    }
}

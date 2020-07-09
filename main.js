class Noise{
    constructor(){
        this.p = new Array(512);
            this.perm = new Array(256);
            for(let i=0; i<256; i++) this.perm[i] = i;
            for(let i=0; i<256; i++){
                let j = Math.floor(Math.random()*(256-i))+i;
                let t = this.perm[i]; this.perm[i] = this.perm[j]; this.perm[j] = t;
        }
        for(let i=0; i<256; i++) this.p[i] = this.p[i+256] = this.perm[i];
    }

    hash(x){
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        x = (x ^ (x >> 31));
        return x & 255;
    }

    fade(t){ return t*t*t*(t*(t*6-15)+10); }
    lerp(t, a, b){ return a + t * (b - a); }
    grad(hash, x, y, z){
        switch(hash & 0xF){
            case 0x0: return  x + y;
            case 0x1: return -x + y;
            case 0x2: return  x - y;
            case 0x3: return -x - y;
            case 0x4: return  x + z;
            case 0x5: return -x + z;
            case 0x6: return  x - z;
            case 0x7: return -x - z;
            case 0x8: return  y + z;
            case 0x9: return -y + z;
            case 0xA: return  y - z;
            case 0xB: return -y - z;
            case 0xC: return  y + x;
            case 0xD: return -y + z;
            case 0xE: return  y - x;
            case 0xF: return -y - z;
        }
    }

    noise(x, y, z){
        let X = Math.floor(x) & 255, Y = Math.floor(y) & 255, Z = Math.floor(z) & 255;
        x -= Math.floor(x); y -= Math.floor(y); z -= Math.floor(z);
        let u = this.fade(x), v = this.fade(y), w = this.fade(z);

        let hash_base1 = this.p[X]+Y, hash_base2 = this.p[X+1]+Y;
        let h11 = this.p[hash_base1]+Z, h12 = this.p[hash_base1+1]+Z;
        let h21 = this.p[hash_base2]+Z, h22 = this.p[hash_base2+1]+Z;

        let t11 = this.lerp(u, this.grad(this.p[h11], x, y, z), this.grad(this.p[h21], x-1, y, z));
        let t12 = this.lerp(u, this.grad(this.p[h12], x, y-1, z), this.grad(this.p[h22], x-1, y-1, z));
        let t1 = this.lerp(v, t11, t12);

        let t21 = this.lerp(u, this.grad(this.p[h11+1], x, y, z-1), this.grad(this.p[h21+1], x-1, y, z-1));
        let t22 = this.lerp(u, this.grad(this.p[h12+1], x, y-1, z-1), this.grad(this.p[h22+1], x-1, y-1, z-1));
        let t2 = this.lerp(v, t21, t22);
        return this.lerp(w, t1, t2);
    }

    noiseWithOctave(x, y, z, octave, persistence){
        let sum = 0, frequency = 1, amplitude = 1, mx = 0;
        for(let i=0; i<octave; i++){
            sum += this.noise(x*frequency, y*frequency, z*frequency) * amplitude;
            mx += amplitude;
            amplitude *= persistence;
            frequency *= 2;
        }
        return sum / mx;
    }
}
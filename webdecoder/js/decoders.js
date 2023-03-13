let algoBits = 8, quantization = 16;

function sgn(val) {
    return (0 < val) - (val < 0);
}

function transposeFromHilbertCoords(col) {
    let X = [ col[0], col[1], col[2] ];
    let N = 2 << (algoBits - 1), P, Q, t;

    // Gray decode by H ^ (H/2)
    t = X[3 - 1] >> 1;
    // Corrected error in Skilling's paper on the following line. The appendix had i >= 0 leading to negative array index.
    for (let i = 3 - 1; i > 0; i--) 
        X[i] ^= X[i - 1];
    X[0] ^= t;

    // Undo excess work
    for (Q = 2; Q != N; Q <<= 1) {
        P = Q - 1;
        for (let i = 3 - 1; i >= 0; i--)
            if (X[i] & Q) // Invert
                X[0] ^= P;
            else { // Exchange
                t = (X[0] ^ X[i]) & P;
                X[0] ^= t;
                X[i] ^= t;
            }
    }

    col[0] = X[0]; col[1] = X[1]; col[2] = X[2];
    return col;
}

function transposeToHilbertCoords(col) {
    let X = [ col[0], col[1], col[2] ];
    let M = 1 << (algoBits - 1), P, Q, t;

    // Inverse undo
    for (Q = M; Q > 1; Q >>= 1) {
        P = Q - 1;
        for (let i = 0; i < 3; i++) {
            if (X[i] & Q) // Invert
                X[0] ^= P;
            else { // Exchange
                t = (X[0] ^ X[i]) & P;
                X[0] ^= t;
                X[i] ^= t;
            }
        }
    }

    // Gray encode
    for (let i = 1; i < 3; i++) 
        X[i] ^= X[i - 1];
    t = 0;
    for (Q = M; Q > 1; Q >>= 1)
        if (X[3 - 1] & Q) 
            t ^= Q - 1;
    for (let i = 0; i < 3; i++) 
        X[i] ^= t;

    col[0] = X[0]; col[1] = X[1]; col[2]= X[2];
    return col;
}

function hilbertDecode(col1) {
    let segmentBits = 8 - algoBits;
    let side = 1 << segmentBits;
    let currColor;

    let fract = [];
    for (let i = 0; i < 3; i++) {
        fract.push(col1[i] & ((1 << segmentBits) - 1));
        if (fract[i] >= side / 2) {
            fract[i] -= side;
            col1[i] += side / 2; //round to the closest one
        }
    }

    for (let i = 0; i < 3; i++)
        col1[i] >>= segmentBits;

    currColor = col1;
    col1 = transposeToHilbertCoords(col1);
    [col1[0], col1[2]] = [col1[2], col1[0]];
    let v1 = mortonDecode(col1, true);

    let v2 = Math.min(v1 + 1, 1 << quantization);
    let nextCol = mortonEncode(v2, true);
    [nextCol[0], nextCol[2]] = [nextCol[2], nextCol[0]];
    nextCol = transposeFromHilbertCoords(nextCol);

    let v3 = Math.max(v1 - 1, 0);

    let prevCol = mortonEncode(v3, true);
    [prevCol[0], prevCol[2]] = [prevCol[2], prevCol[0]];
    prevCol = transposeFromHilbertCoords(prevCol);

    v1 <<= segmentBits;
    for (let i = 0; i < 3; i++)
        v1 += fract[i] * sgn(nextCol[i] - prevCol[i]);
    return v1 << (16 - quantization);
}

function mortonDecode(col, forHilbert = false) {
    let codex = 0, codey = 0, codez = 0;
    let nbits2;
    if (forHilbert) nbits2 = 2 * algoBits;
    else nbits2 = 12;

    for (let i = 0, andbit = 1; i < nbits2; i += 2, andbit <<= 1) {
        codex |= (col[0] & andbit) << i;
        codey |= (col[1] & andbit) << i;
        codez |= (col[2] & andbit) << i;
    }

    return ((codez << 2) | (codey << 1) | codex);
}

function mortonEncode(val, forHilbert = false) {
    let ret = [0,0,0];
    ret[0] = 0; ret[1] = 0; ret[2] = 0;

    let mortBits;
    if (forHilbert)
        mortBits = algoBits;
    else
    {
        mortBits = 6;
        val <<= (16 - m_Quantization);
    }

    for (let i = 0; i <= mortBits; ++i) {
        let selector = 1;
        let shift_selector = 3 * i;
        let shiftback = 2 * i;

        ret[0] |= (val & (selector << shift_selector)) >> (shiftback);
        ret[1] |= (val & (selector << (shift_selector + 1))) >> (shiftback + 1);
        ret[2] |= (val & (selector << (shift_selector + 2))) >> (shiftback + 2);
    }

    return ret;
}

function phaseDecode(col) {
    const w = 65535.0;
    const P = 16384.0;
    const beta = P / 2.0;

    let gamma, phi, PHI, K, Z;
    let i1 = col[0] / 255.0, i2 = col[1] / 255.0;

    phi = Math.abs(Math.acos(2.0 * i1 - 1.0));
    gamma = Math.floor((i2 * w) / beta);

    if (gamma % 2)
        phi *= -1;

    K = Math.round((i2 * w) / P);
    PHI = phi + 2 * Math.PI * K;

    Z = PHI * (P / (Math.PI * 2.0));
    return Math.min(Math.max(0, Z), 65535);
}

function triangleDecode(col) {
    const w = 65536;
    // Function data
    let np = 512;
    let p = np / w;
    let Ld = col[0] / 255.0;
    let Ha = col[1] / 255.0;
    let Hb = col[2] / 255.0;
    let m = Math.floor(4.0 * (Ld / p) - (Ld > 0 ? 0.5 : 0.0)) % 4;
    let L0 = (Ld - (Ld - p / 8.0 % p) + (p / 4.0) * m - p / 8.0);
    let delta = 0;

    switch (m) {
    case 0:
        delta = (p / 2.0) * Ha;
        break;
    case 1:
        delta = (p / 2.0) * Hb;
        break;
    case 2:
        delta = (p / 2.0) * (1.0 - Ha);
        break;
    case 3:
        delta = (p / 2.0) * (1.0 - Hb);
        break;
    }

    return (L0 + delta) * w;
}

function hueDecode(col) {
    let r = col[0], g = col[1], b = col[2];
    let ret = 0;

    if (b + g + r < 255)
        ret = 0;
    else if (r >= g && r >= b)
    {
        if (g >= b)
            ret = g - b;
        else
            ret = (g - b) + 1529;
    }
    else if (g >= r && g >= b)
        ret = b - r + 510;
    else if (b >= g && b >= r)
        ret = r - g + 1020;

    let q = Math.round((ret / 1529.0) * (1 << quantization));
    q = q * (1 << (16 - quantization));

    return q;
}

function packedDecode(col) {
    let highPart, lowPart;
    let right = quantization - algoBits;

    col[0] >>= (8 - algoBits);
    col[1] >>= (8 - right);

    highPart = col[0] << right;
    lowPart = col[1];

    return (highPart + lowPart) << (16 - quantization);
}

function splitDecode(col) {
    let highPart, lowPart;
    let right = quantization - algoBits;

    col[0] >>= (8 - algoBits);
    col[1] >>= (8 - right);

    if((col[0] & 0x1) == 1)
        col[1] = (1<<right) - col[1]- 1;

    highPart = col[0] << right;
    lowPart = col[1];

    return (highPart + lowPart) << (16 - quantization);
}
function generateShrinkTable(coder) {

}

function generateDecodingTable(coder) {
    let ret = new Uint8Array(255 * 255 * 255 * 3);
    
    for (let i=0; i<255; i++) {
        for (let j=0; j<255; j++) {
            for (let k=0; k<255; k++) {
                let col = [i, j, k];
                let depth;

                switch (coder) {
                    case "hilbert": depth = hilbertDecode(col); break;
                    case "morton": depth = mortonDecode(col); break;
                    case "packed": depth = packedDecode(col); break;
                    case "split": depth = splitDecode(col); break;
                    case "phase": depth = phaseDecode(col); break;
                    case "hue": depth = hueDecode(col); break;
                    case "triangle": depth = triangleDecode(col); break;
                }
                let idx = 255 * 255 * 3 * i + 255 * 3 *j + k;
                ret[idx] = depth >> 8;
                ret[idx+1] = depth & 255;
                ret[idx+2] = 0;
            }
        }
    }

    return ret;
}
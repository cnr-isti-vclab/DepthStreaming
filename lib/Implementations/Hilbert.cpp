#include <Implementations/Hilbert.h>

#include <utility>
#include <cassert>
#include <iostream>
#include <cmath>

// Credits for basic Hilbert convertions: https://github.com/davemc0/DMcTools/blob/main/Math/SpaceFillCurve.h

namespace DStream
{

    Hilbert::Hilbert(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistributions) : Coder(quantization, algoBits, channelDistributions)
	{
		m_AlgoBits = algoBits;
        m_Morton = Morton(quantization, algoBits, { 8,8,8 }, true);
	}

	Color Hilbert::EncodeValue(uint16_t val)
	{
		Color v = m_Morton.EncodeValue(val);
		std::swap(v[0], v[2]);
		TransposeFromHilbertCoords(v);
        
        return v;
	}

    uint16_t Hilbert::DecodeValue(Color col1)
    {
        Color col = col1;
        TransposeToHilbertCoords(col);
        std::swap(col[0], col[2]);
        return m_Morton.DecodeValue(col);
    }

    void Hilbert::TransposeFromHilbertCoords(Color& col)
    {
        int X[3] = { col.x, col.y, col.z };
        uint32_t N = 2 << (m_AlgoBits - 1), P, Q, t;

        // Gray decode by H ^ (H/2)
        t = X[3 - 1] >> 1;
        // Corrected error in Skilling's paper on the following line. The appendix had i >= 0 leading to negative array index.
        for (int i = 3 - 1; i > 0; i--) X[i] ^= X[i - 1];
        X[0] ^= t;

        // Undo excess work
        for (Q = 2; Q != N; Q <<= 1) {
            P = Q - 1;
            for (int i = 3 - 1; i >= 0; i--)
                if (X[i] & Q) // Invert
                    X[0] ^= P;
                else { // Exchange
                    t = (X[0] ^ X[i]) & P;
                    X[0] ^= t;
                    X[i] ^= t;
                }
        }

        col.x = X[0]; col.y = X[1]; col.z = X[2];
    }

    void Hilbert::TransposeToHilbertCoords(Color& col)
    {
        int X[3] = { col.x, col.y, col.z };
        uint32_t M = 1 << (m_AlgoBits - 1), P, Q, t;

        // Inverse undo

        for (Q = M; Q > 1; Q >>= 1) {
            P = Q - 1;
            for (int i = 0; i < 3; i++) {
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
        for (int i = 1; i < 3; i++) X[i] ^= X[i - 1];
        t = 0;
        for (Q = M; Q > 1; Q >>= 1)
            if (X[3 - 1] & Q) t ^= Q - 1;
        for (int i = 0; i < 3; i++) X[i] ^= t;

        col.x = X[0]; col.y = X[1]; col.z = X[2];
    }
}
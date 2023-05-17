#include <Implementations/Hilbert.h>

#include <utility>
#include <cassert>
#include <iostream>
#include <cmath>

// Credits for basic Hilbert convertions: https://github.com/davemc0/DMcTools/blob/main/Math/SpaceFillCurve.h

namespace DStream
{
    static float BilinearInterpolation(float p00, float p10, float p01, float p11, float tx, float ty)
    {
        float  a = p00 * (1 - tx) + p10 * tx;
        float  b = p01 * (1 - tx) + p11 * tx;
        return a * (1 - ty) + b * ty;
    }


    template <typename T>
    static inline int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

    uint16_t Hilbert::MortonDecode(Vec3& c)
    {
        Color col1 = { (uint8_t)c.x, (uint8_t)c.y, (uint8_t)c.z };
        TransposeToHilbertCoords(col1);
        std::swap(col1[0], col1[2]);
        return m_Morton.DecodeValue(col1) << m_SegmentBits;
    }

    Hilbert::Hilbert(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistributions) : Coder(quantization, algoBits, channelDistributions)
	{
#ifdef _WIN32
		assert(algoBits * 3 < quantization);
#else
		assert(algoBits * 3 < quantization);
#endif

		m_AlgoBits = algoBits;
		m_SegmentBits = quantization - 3 * m_AlgoBits;

#ifdef _WIN32
		assert(m_AlgoBits + m_SegmentBits <= 8);
#else
		assert(m_AlgoBits + m_SegmentBits <= 8);
#endif
        
        m_Morton = Morton(quantization, algoBits, { 8,8,8 }, true);
	}

	Color Hilbert::EncodeValue(uint16_t val)
	{
		int frac = val & ((1 << m_SegmentBits) - 1);
		val >>= m_SegmentBits;

		Color v = m_Morton.EncodeValue(val);
		std::swap(v[0], v[2]);
		TransposeFromHilbertCoords(v);
        
        if (val >= (1 << (m_AlgoBits * 3)))
            val = (1 << (m_AlgoBits * 3)) - 1;

        if (val + 1 < (1 << (m_AlgoBits * 3)))
        {
            // SUBDIVISION
            Color v2 = m_Morton.EncodeValue(val + 1);
            std::swap(v2[0], v2[2]);
            TransposeFromHilbertCoords(v2);

            // Divide in segments
            for (uint32_t i = 0; i < 3; i++)
            {
                uint8_t side = 1 << m_SegmentBits;
                uint8_t mult = sgn(v2[i] - v[i]) * frac;
                uint8_t overflown = mult + side;

                v[i] <<= m_SegmentBits;

                if (overflown < mult)
                    v[i] -= side - overflown;
                else
                    v[i] += mult;
            }
        }
        else
        {
            Color v2 = m_Morton.EncodeValue(val + 1);
            std::swap(v2[0], v2[2]);
            TransposeFromHilbertCoords(v2);

            for (uint32_t i = 0; i < 3; i++)
                v[i] = (v[i] << m_SegmentBits);
        }
        return v;
	}

    uint16_t Hilbert::DecodeValue(Color col1)
    {
        uint16_t ret;
        float divisor = 1 << (8 - m_AlgoBits);

        std::vector<Color> corners;
        std::vector<uint16_t> values;

        Vec3 col = Vec3((float)col1.x / divisor, (float)col1.y / divisor, (float)col1.z / divisor);

        /*    
              G---H  
            C---D |
            | E-|-F
            A---B
        */
        float U, V, W;
        float u = modf(col.x, &U);
        float v = modf(col.y, &V);
        float w = modf(col.z, &W);

        // BACK COLORS
        Vec3 c111 = { U + 1,    V + 1,  W + 1 };
        Vec3 c011 = { U,        V + 1,  W + 1 };
        Vec3 c101 = { U + 1,    V,      W + 1};
        Vec3 c001 = { U,        V,      W + 1 };

        // FRONT COLORS
        Vec3 c110 = { U + 1,    V + 1,  W };
        Vec3 c010 = { U,        V + 1,  W };
        Vec3 c100 = { U + 1,    V,      W };
        Vec3 c000 = { U,        V,      W };

        // Depth values
        uint16_t A = MortonDecode(c000), E = MortonDecode(c001), C = MortonDecode(c010), G = MortonDecode(c011), B = MortonDecode(c100), 
            F = MortonDecode(c101), D = MortonDecode(c110), H = MortonDecode(c111);

        /*
        if (W == 1)
            std::cout << "stop" << std::endl;
            */
        // Interpolation values
        float threshold = 1 << (9);

        float uAB = std::abs(A - B) > threshold ? std::round(u) : u;
        float uCD = std::abs(C - D) > threshold ? std::round(u) : u;
        float uEF = std::abs(E - F) > threshold ? std::round(u) : u;
        float uGH = std::abs(G - H) > threshold ? std::round(u) : u;

        float vAC = std::abs(A - C) > threshold ? std::round(v) : v;
        float vBD = std::abs(B - D) > threshold ? std::round(v) : v;
        float vEG = std::abs(E - G) > threshold ? std::round(v) : v;
        float vFH = std::abs(F - H) > threshold ? std::round(v) : v;
        
        float wAE = std::abs(A - E) > threshold ? std::round(w) : w;
        float wBF = std::abs(B - F) > threshold ? std::round(w) : w;
        float wCG = std::abs(C - G) > threshold ? std::round(w) : w;
        float wDH = std::abs(D - H) > threshold ? std::round(w) : w;

        // Interpolate values
        float val = A * (1 - uAB) * (1 - vAC) * (1 - wAE) +
                    B *       uAB * (1 - vBD) * (1 - wBF) +
                    C * (1 - uCD) *       vAC * (1 - wCG) +
                    D *       uCD *       vBD * (1 - wDH) +

                    E * (1 - uEF) * (1 - vEG) * wAE +
                    F *       uEF * (1 - vFH) * wBF +
                    G * (1 - uGH) *       vEG * wCG +
                    H *       uGH *       vFH * wDH;

        return A;// std::round(val);
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
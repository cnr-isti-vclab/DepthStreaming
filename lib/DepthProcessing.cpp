#include <DepthProcessing.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

namespace DStream
{
    static void GetMinMax(uint16_t& min, uint16_t& max, uint16_t* data, uint32_t nElements)
    {
        for (uint32_t i = 0; i < nElements; i++)
        {
            min = std::min(min, data[i]);
            max = std::max(max, data[i]);
        }
    }

	void DepthProcessing::DenoiseMedian(uint16_t* data, uint32_t width, uint32_t height, uint32_t threshold, int halfWind /* = 1*/)
	{
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // Get all neighbors
                std::vector<uint16_t> neighbors;
                int current = data[x + y * width];

                for (int i = -halfWind; i <= halfWind; i++)
                {
                    for (int j = -halfWind; j <= halfWind; j++)
                    {
                        int xCoord = x + j;
                        int yCoord = (y + i);
                        // Don't include current point
                        if (xCoord >= 0 && xCoord < width && yCoord >= 0 && yCoord < height)
                            neighbors.push_back(data[xCoord + yCoord * width]);
                    }
                }

                std::sort(neighbors.begin(), neighbors.end());
                float err = std::abs(current - neighbors[neighbors.size() / 2]);

                if (err > threshold)
                    data[x + y * width] = neighbors[neighbors.size() / 2];
                else
                    data[x + y * width] = current;
            }
        }
	}

    void DepthProcessing::Quantize(uint16_t* dest, float* source, uint8_t q, uint32_t nElements, float minHint, float maxHint)
    {
        float min, max;
        if (minHint < maxHint)
        {
            min = minHint;
            max = maxHint;
        }
        else
        {
            min = std::numeric_limits<float>().max();
            max = std::numeric_limits<float>().min();

            for (uint32_t i = 0; i < nElements; i++)
            {
                min = std::min(min, source[i]);
                max = std::max(max, source[i]);
            }
        }

        for (uint32_t i = 0; i < nElements; i++)
            dest[i] = std::round(((source[i] - min) / max) * ((1 << q)-1));
    }

    void DepthProcessing::Quantize(uint16_t* dest, uint16_t* source, uint8_t q, uint32_t nElements, uint16_t minHint, uint16_t maxHint)
    {
        uint16_t min = 65535, max = 0;
        GetMinMax(min, max, source, nElements);

        for (uint32_t i = 0; i < nElements; i++)
            dest[i] = std::round(((float)(source[i] - min) / max) * ((1 << q)-1));
    }


    void DepthProcessing::Dequantize(uint16_t* dest, uint16_t* source, uint8_t currQ, uint32_t nElements, uint16_t minHint, uint16_t maxHint)
    {
        uint16_t min = 65535, max = 0;
        GetMinMax(min, max, source, nElements);

        for (uint32_t i = 0; i < nElements; i++)
            dest[i] = source[i] << (16 - currQ);
    }
}
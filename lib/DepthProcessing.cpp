#include <DepthProcessing.h>

#include <vector>
#include <algorithm>

namespace DStream
{
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

    void DepthProcessing::Quantize(uint16_t* source, uint16_t* dest, uint8_t q, uint32_t nElements)
    {
        for (uint32_t i = 0; i < nElements; i++)
            dest[i] = (source[i] >> (16 - q)) << (16 - q);
    }
}
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

	void DepthProcessing::DenoiseMedianExclusive(uint16_t* data, uint32_t width, uint32_t height, uint32_t threshold, int halfWind)
	{
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // Compute distance sqrt(matrix size)
                int matSize = (halfWind * 2 + 1) * (halfWind * 2 + 1);
                bool removed = false;
                if (y - halfWind < 0 || y + halfWind >= height)
                {
                    matSize -= halfWind * 2 + 1;
                    removed = true;
                }
                if (x - halfWind < 0 || x + halfWind >= width)
                {
                    if (!removed)
                        matSize -= halfWind * 2 + 1;
                    else
                        matSize -= halfWind * 2;
                }

                // Get all neighbors
                std::vector<int> neighbors;

                int current = data[x + y * width];
                for (int i = -halfWind; i <= halfWind; i++)
                {
                    for (int j = -halfWind; j <= halfWind; j++)
                    {
                        int xCoord = x + j;
                        int yCoord = (y + i);
                        if (xCoord >= 0 && xCoord < width && yCoord >= 0 && yCoord < height)
                            neighbors.push_back(data[xCoord + yCoord * width]);
                    }
                }

                // Compute distance matrix
                int* distanceMatrix = new int[matSize * matSize];

                for (int i = 0; i < matSize; i++)
                    for (int j = 0; j < matSize; j++)
                        distanceMatrix[i*matSize + j] = std::abs(neighbors[i] - neighbors[j]);

                uint32_t outliers = 0;
                // Check if there's an outlier
                for (uint32_t i = 0; i < matSize; i++)
                    if (distanceMatrix[i*matSize] > threshold)
                        outliers++;

                // Find the right outliers (for each neighbor, return the one with the biggest row + column sum)
                std::vector<int> outliersIdx;

                for (uint32_t i = 0; i < outliers; i++)
                {
                    int currOutlier = -1;
                    int maxSum = -1;
                    for (int j = 0; j < matSize; j++)
                    {
                        // Compute row + column
                        int row = 0, col = 0;
                        for (int k = 0; k < matSize; k++)
                        {
                            row += distanceMatrix[j*matSize + k];
                            col += distanceMatrix[k*matSize + j];
                        }

                        if ((row + col) > maxSum && !std::count(outliersIdx.begin(), outliersIdx.end(), j))
                        {
                            currOutlier = j;
                            maxSum = row + col;
                        }
                    }
                    if (currOutlier != -1)
                        outliersIdx.push_back(currOutlier);
                }

                std::vector<uint16_t> goodNeighbors;
                for (int i = 0; i < neighbors.size(); i++)
                    if (!std::count(outliersIdx.begin(), outliersIdx.end(), i))
                        goodNeighbors.push_back(neighbors[i]);


                // Compute the average without maxIdx
                float avg = 0;
                for (uint32_t i = 0; i < goodNeighbors.size(); i++)
                    avg += goodNeighbors[i];
                avg /= goodNeighbors.size();

                float err = std::abs(current - avg);
                std::sort(goodNeighbors.begin(), goodNeighbors.end());

                if (err > threshold)
                    data[x + y * width] = goodNeighbors[goodNeighbors.size() / 2];
                else
                    data[x + y * width] = current;
            }
        }
	}

    uint16_t* DepthProcessing::Quantize(uint16_t* source, uint8_t q, uint32_t nElements)
    {
        uint16_t* ret = new uint16_t[nElements];
        for (uint32_t i = 0; i < nElements; i++)
            ret[i] = (source[i] >> (16 - q)) << (16 - q);
        return ret;
    }
}
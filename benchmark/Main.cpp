#include <StreamCoder.h>
#include <DepthmapReader.h>
#include <DepthProcessing.h>
#include <ImageWriter.h>

#include <Implementations/Hilbert.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed.h>
#include <Implementations/Split.h>
#include <Implementations/Morton.h>
#include <Implementations/Phase.h>
#include <Implementations/Triangle.h>

#include <iostream>
#include <filesystem>

using namespace DStream;

void EncodeDecode(const std::string& coder, uint16_t* originalData, uint8_t* encoded, uint16_t* decoded, uint32_t nElements, 
	uint8_t quantization, bool enlarge, bool useTables)
{
	if (!coder.compare("Packed"))
	{
		StreamCoder<Packed> coder(quantization, enlarge, 8, useTables);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Hue"))
	{
		StreamCoder<Hue> coder(quantization, enlarge, 8, useTables);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Hilbert"))
	{
		StreamCoder<Hilbert> coder(quantization, enlarge, 3, useTables);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Morton"))
	{
		StreamCoder<Morton> coder(quantization, enlarge, 8, useTables);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Split"))
	{
		StreamCoder<Split> coder(quantization, enlarge, 8, useTables);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Phase"))
	{
		StreamCoder<Phase> coder(quantization, enlarge, 8, useTables);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else
	{
		StreamCoder<Triangle> coder(quantization, enlarge, 8, useTables);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
}

template <typename Coder>
void TestCoder(uint32_t q, uint32_t algo)
{
	Coder c(q, algo);
	float avg = 0;
	float max = 0;
	for (uint32_t i = 0; i < 65535; i++)
	{
		uint32_t quantized = (i >> (16 - q));
		quantized <<= 16 - q;

		Color col = c.EncodeValue(quantized);
		uint16_t v = c.DecodeValue(col);

		quantized <<= (16 - q);
		int err = std::abs((int)v - (int)quantized);
		avg += err;
		max = std::max<int>(max, err);

		//std::cout << "ERR: " << err << ", decoded: " << v << ", correct: " << quantized << std::endl;
	}
	avg /= 65536;

	std::cout << "Avg: " << avg << ", max: " << max << std::endl;
}

std::vector<uint8_t> GetAlgoBitsToTest(const std::string& algo, uint8_t q)
{
	std::vector<uint8_t> ret = { 8 };

	if (!algo.compare("Morton"))
		return ret;
	else if (!algo.compare("Hilbert"))
	{
		for (uint8_t i = 1; i <= 8; i++)
		{
			uint8_t algoBits = i;
			uint8_t segmentBits = q - 3 * algoBits;

			if (algoBits * 3 < q && algoBits + segmentBits <= 8)
				ret.push_back(algoBits);
		}
	}
	else if (!algo.compare("Packed"))
	{
		for (uint32_t i = q - 8; i < 8; i++)
			ret.push_back(i);
		return ret;
	}
	else if (!algo.compare("Split"))
		return ret;
	else if (!algo.compare("Triangle"))
		return ret;
	else if (!algo.compare("Phase"))
		return ret;
	else if (!algo.compare("Hue"))
		return ret;
}

int main(int argc, char** argv)
{
	// Benchmark parameters
	std::string coders[7] = { "Morton", "Hue", "Hilbert", "Triangle", "Split", "Phase", "Packed" };
	uint8_t quantizations[4] = {10, 12, 14, 16};
	uint8_t jpegQualities[4] = { 70, 80, 90, 100 };
	bool enlarge[2] = {true, false};
	bool denoising[2] = { false, true };
	bool tabulate[2] = { false, true };
	std::vector<uint8_t> algoBits;

	DepthmapData dmData;
	DepthmapReader reader("envoi_RTI/MNT.asc", DepthmapFormat::ASC, dmData);
	uint32_t nElements = dmData.Width * dmData.Height;

	uint16_t* originalData = reader.GetData();
	uint8_t* encodedData = new uint8_t[nElements * 3];
	uint16_t* decodedData = new uint16_t[nElements];

	//TestCoder<Triangle>(16, 8);

	for (uint32_t c = 0; c < 7; c++)
	{
		for (uint32_t q = 0; q < 4; q++)
		{
			uint16_t* quantizedData = DepthProcessing::Quantize(originalData, quantizations[q], nElements);
			algoBits = GetAlgoBitsToTest(coders[c], q);

			for (uint32_t p = 0; p < algoBits.size(); p++)
			{
				for (uint32_t j = 0; j < 4; j++)
				{

				}
			}
			
		}
		std::cout << "Benchmarking " << coders[c] << std::endl;
		EncodeDecode(coders[c], originalData, encodedData, decodedData, nElements, 10, true, true);
		ImageWriter::WriteEncoded(coders[c] + "encoded.jpg", encodedData, dmData.Width, dmData.Height, ImageFormat::JPG, 90);
		ImageWriter::WriteDecoded(coders[c] + "decoded.png", decodedData, dmData.Width, dmData.Height);
	}

	delete[] encodedData;
	delete[] decodedData;
	//delete[] originalData;

	return 0;
}
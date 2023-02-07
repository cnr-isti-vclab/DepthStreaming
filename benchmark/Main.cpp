#include <StreamCoder.h>
#include <DepthmapReader.h>
#include <ImageWriter.h>

#include <Implementations/Hilbert.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed.h>
#include <Implementations/Split.h>
#include <Implementations/Morton.h>
#include <Implementations/Phase.h>
#include <Implementations/Triangle.h>

#include <iostream>

using namespace DStream;

void EncodeDecode(const std::string& coder, uint16_t* originalData, uint8_t* encoded, uint16_t* decoded, uint32_t nElements, uint8_t quantization, bool enlarge)
{
	if (!coder.compare("Packed"))
	{
		StreamCoder<Packed> coder(quantization, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Hue"))
	{
		StreamCoder<Hue> coder(quantization, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Hilbert"))
	{
		StreamCoder<Hilbert> coder(quantization, enlarge, 3, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Morton"))
	{
		StreamCoder<Morton> coder(quantization, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Split"))
	{
		StreamCoder<Split> coder(quantization, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Phase"))
	{
		StreamCoder<Phase> coder(quantization, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else
	{
		StreamCoder<Triangle> coder(quantization, enlarge, 8, false);
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

int main(int argc, char** argv)
{
	std::string coders[7] = { "Morton", "Hue", "Hilbert", "Triangle", "Split", "Phase", "Packed" };
	DepthmapData dmData;
	DepthmapReader reader("envoi_RTI/MNT.asc", DepthmapFormat::ASC, dmData);
	uint32_t nElements = dmData.Width * dmData.Height;

	uint16_t* originalData = reader.GetData();
	uint8_t* encodedData = new uint8_t[nElements * 3];
	uint16_t* decodedData = new uint16_t[nElements];

	//TestCoder<Triangle>(16, 8);

	for (uint32_t i = 0; i < 7; i++)
	{
		EncodeDecode(coders[i], originalData, encodedData, decodedData, nElements, 10, true);
		ImageWriter::WriteEncoded(coders[i] + "encoded.jpg", encodedData, dmData.Width, dmData.Height, ImageFormat::JPG, 90);
		ImageWriter::WriteDecoded(coders[i] + "decoded.png", decodedData, dmData.Width, dmData.Height);
	}

	delete[] encodedData;
	delete[] decodedData;
	//delete[] originalData;

	return 0;
}
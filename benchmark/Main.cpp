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

/*
	BUG LIST
	- Enlarge not working properly in some cases: 
		- Morton stays unaltered
		- Bug in triangle encoding

*/

using namespace DStream;


void EncodeDecode(const std::string& coder, uint16_t* originalData, uint8_t* encoded, uint16_t* decoded, uint32_t nElements, bool enlarge)
{
	if (!coder.compare("Packed"))
	{
		StreamCoder<Packed> coder(10, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Hue"))
	{
		StreamCoder<Hue> coder(10, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Hilbert"))
	{
		StreamCoder<Hilbert> coder(10, enlarge, 2, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Morton"))
	{
		StreamCoder<Morton> coder(10, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Split"))
	{
		StreamCoder<Split> coder(10, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else if (!coder.compare("Phase"))
	{
		StreamCoder<Phase> coder(10, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
	else
	{
		StreamCoder<Triangle> coder(10, enlarge, 8, false);
		coder.Encode(originalData, (Color*)encoded, nElements);
		coder.Decode((Color*)encoded, decoded, nElements);
	}
}

template <typename Coder>
void TestCoder(uint32_t q, uint32_t algo)
{
	Coder c(q, algo);
	for (uint32_t i = 0; i < 65535; i++)
	{
		uint32_t quantized = (i >> (16 - q));
		quantized <<= 16 - q;

		Color col = c.EncodeValue(quantized);
		uint16_t v = c.DecodeValue(col);

		if (v != quantized)
			std::cout << "ERROR amount: " << std::abs((int)v - (int)quantized) << ", value: " << quantized << ", unquantized: " << i << std::endl;
	}
}

int main(int argc, char** argv)
{
	std::string coders[7] = { "Triangle", "Hue", "Hilbert", "Morton", "Split", "Phase", "Packed" };
	DepthmapData dmData;
	DepthmapReader reader("envoi_RTI/MNT.asc", DepthmapFormat::ASC, dmData);
	uint32_t nElements = dmData.Width * dmData.Height;

	uint16_t* originalData = reader.GetData();
	uint8_t* encodedData = new uint8_t[nElements * 3];
	uint16_t* decodedData = new uint16_t[nElements];

	TestCoder<Triangle>(10, 8);

	for (uint32_t i = 0; i < 7; i++)
	{
		EncodeDecode(coders[i], originalData, encodedData, decodedData, nElements, true);
		ImageWriter::WriteEncoded(coders[i] + "encoded.jpg", encodedData, dmData.Width, dmData.Height, ImageFormat::JPG, 90);
		ImageWriter::WriteDecoded(coders[i] + "decoded.png", decodedData, dmData.Width, dmData.Height);
	}

	delete[] encodedData;
	delete[] decodedData;
	//delete[] originalData;

	return 0;
}
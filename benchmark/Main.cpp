#include <StreamCoder.h>
#include <DepthmapReader.h>
#include <ImageWriter.h>
#include <Implementations/Hilbert.h>

using namespace DStream;

int main(int argc, char** argv)
{
	DepthmapData dmData;
	DepthmapReader reader("envoi_RTI/MNT.asc", DepthmapFormat::ASC, dmData);
	uint32_t nElements = dmData.Width * dmData.Height;

	uint16_t* originalData = reader.GetData();
	uint8_t* encodedData = new uint8_t[nElements * 3];
	uint16_t* decodedData = new uint16_t[nElements];

	StreamCoder<Hilbert> coder(10, true, 2, false);
	coder.Encode(originalData, (Color*)encodedData, nElements);
	coder.Decode((Color*)encodedData, decodedData, nElements);

	ImageWriter::WriteEncoded("encoded.jpg", encodedData, dmData.Width, dmData.Height, ImageFormat::JPG, 70);
	ImageWriter::WriteDecoded("decoded.jpg", decodedData, dmData.Width, dmData.Height);

	delete[] encodedData;
	return 0;
}
#include <StreamCoder.h>
#include <DepthmapReader.h>
#include <DepthProcessing.h>
#include <ImageWriter.h>
#include <JpegDecoder.h>
#include <Timer.h>

#include <Implementations/Hilbert.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed.h>
#include <Implementations/Split.h>
#include <Implementations/Morton.h>
#include <Implementations/Phase.h>
#include <Implementations/Triangle.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

using namespace DStream;

unsigned char turbo_srgb_bytes[256][3] = { {48,18,59},{50,21,67},{51,24,74},{52,27,81},{53,30,88},{54,33,95},{55,36,102},{56,39,109},{57,42,115},{58,45,121},{59,47,128},{60,50,134},{61,53,139},{62,56,145},{63,59,151},{63,62,156},{64,64,162},{65,67,167},{65,70,172},{66,73,177},{66,75,181},{67,78,186},{68,81,191},{68,84,195},{68,86,199},{69,89,203},{69,92,207},{69,94,211},{70,97,214},{70,100,218},{70,102,221},{70,105,224},{70,107,227},{71,110,230},{71,113,233},{71,115,235},{71,118,238},{71,120,240},{71,123,242},{70,125,244},{70,128,246},{70,130,248},{70,133,250},{70,135,251},{69,138,252},{69,140,253},{68,143,254},{67,145,254},{66,148,255},{65,150,255},{64,153,255},{62,155,254},{61,158,254},{59,160,253},{58,163,252},{56,165,251},{55,168,250},{53,171,248},{51,173,247},{49,175,245},{47,178,244},{46,180,242},{44,183,240},{42,185,238},{40,188,235},{39,190,233},{37,192,231},{35,195,228},{34,197,226},{32,199,223},{31,201,221},{30,203,218},{28,205,216},{27,208,213},{26,210,210},{26,212,208},{25,213,205},{24,215,202},{24,217,200},{24,219,197},{24,221,194},{24,222,192},{24,224,189},{25,226,187},{25,227,185},{26,228,182},{28,230,180},{29,231,178},{31,233,175},{32,234,172},{34,235,170},{37,236,167},{39,238,164},{42,239,161},{44,240,158},{47,241,155},{50,242,152},{53,243,148},{56,244,145},{60,245,142},{63,246,138},{67,247,135},{70,248,132},{74,248,128},{78,249,125},{82,250,122},{85,250,118},{89,251,115},{93,252,111},{97,252,108},{101,253,105},{105,253,102},{109,254,98},{113,254,95},{117,254,92},{121,254,89},{125,255,86},{128,255,83},{132,255,81},{136,255,78},{139,255,75},{143,255,73},{146,255,71},{150,254,68},{153,254,66},{156,254,64},{159,253,63},{161,253,61},{164,252,60},{167,252,58},{169,251,57},{172,251,56},{175,250,55},{177,249,54},{180,248,54},{183,247,53},{185,246,53},{188,245,52},{190,244,52},{193,243,52},{195,241,52},{198,240,52},{200,239,52},{203,237,52},{205,236,52},{208,234,52},{210,233,53},{212,231,53},{215,229,53},{217,228,54},{219,226,54},{221,224,55},{223,223,55},{225,221,55},{227,219,56},{229,217,56},{231,215,57},{233,213,57},{235,211,57},{236,209,58},{238,207,58},{239,205,58},{241,203,58},{242,201,58},{244,199,58},{245,197,58},{246,195,58},{247,193,58},{248,190,57},{249,188,57},{250,186,57},{251,184,56},{251,182,55},{252,179,54},{252,177,54},{253,174,53},{253,172,52},{254,169,51},{254,167,50},{254,164,49},{254,161,48},{254,158,47},{254,155,45},{254,153,44},{254,150,43},{254,147,42},{254,144,41},{253,141,39},{253,138,38},{252,135,37},{252,132,35},{251,129,34},{251,126,33},{250,123,31},{249,120,30},{249,117,29},{248,114,28},{247,111,26},{246,108,25},{245,105,24},{244,102,23},{243,99,21},{242,96,20},{241,93,19},{240,91,18},{239,88,17},{237,85,16},{236,83,15},{235,80,14},{234,78,13},{232,75,12},{231,73,12},{229,71,11},{228,69,10},{226,67,10},{225,65,9},{223,63,8},{221,61,8},{220,59,7},{218,57,7},{216,55,6},{214,53,6},{212,51,5},{210,49,5},{208,47,5},{206,45,4},{204,43,4},{202,42,4},{200,40,3},{197,38,3},{195,37,3},{193,35,2},{190,33,2},{188,32,2},{185,30,2},{183,29,2},{180,27,1},{178,26,1},{175,24,1},{172,23,1},{169,22,1},{167,20,1},{164,19,1},{161,18,1},{158,16,1},{155,15,1},{152,14,1},{149,13,1},{146,11,1},{142,10,1},{139,9,2},{136,8,2},{133,7,2},{129,6,2},{126,5,2},{122,4,3} };

struct ErrorData
{
	float MaxError = -1e20;
	float AvgError = 0.0f;

	float DespeckledMaxError = -1e20;
	float DespeckledAvgError = 0.0f;

	uint32_t EncodedTextureSize;
};

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

std::vector<uint8_t> GetAlgoBitsToTest(const std::string& algo, uint8_t q)
{
	std::vector<uint8_t> ret = { 8 };

	if (!algo.compare("Morton"))
		return ret;
	else if (!algo.compare("Hilbert"))
	{
		ret.pop_back();
		for (uint8_t i = 2; i <= 8; i++)
		{
			uint8_t algoBits = i;
			uint8_t segmentBits = q - 3 * algoBits;

			if (algoBits * 3 < q && algoBits + segmentBits <= 8)
				ret.push_back(algoBits);
		}

		return ret;
	}
	else if (!algo.compare("Packed"))
	{
		uint8_t increase = 1;

		if ((16 - q) >= 5) increase = 2;
		for (uint32_t i = q - 8; i < 8; i+=increase)
			ret.push_back(i);
		return ret;
	}
	else if (!algo.compare("Split"))
		return ret;
	else if (!algo.compare("Triangle"))
		return ret;
	else if (!algo.compare("Phase"))
		return ret;
	else
		return ret;
}

std::string GetPathFromComponents(std::vector<std::string> path)
{
	std::string ret = "";
	for (uint32_t i = 0; i < path.size(); i++)
		ret += path[i] + "/";
	return ret;
}

void AddFolderLevel(const std::string& str, int val, std::vector<std::string>& currStructure)
{
	if (val == -1)
		currStructure.push_back(str);
	else
	{
		std::stringstream ss;
		ss << str << " " << val;
		currStructure.push_back(ss.str());
	}

	std::filesystem::create_directory(GetPathFromComponents(currStructure));
}

void AddBenchmarkResult(std::ofstream& file, std::string algo, std::string outFormat, int quantization,
	int jpegQuality, int parameter, const ErrorData& errData)
{
	std::stringstream configName;
	configName << algo << "_Q:" << quantization;
	if (!outFormat.compare("JPG"))
		configName << "_J:" << jpegQuality;
	if (!algo.compare("HILBERT") || !algo.compare("PACKED"))
		configName << "_P:" << parameter;

	//"Configuration, Max Error, Avg Error, Despeckle Max Error, Despeckle Avg Error, Compressed Size\n";
	file << configName.str() << "," << errData.MaxError << "," << errData.AvgError << ",";
	if (errData.DespeckledMaxError >= 0.0f)
		file << errData.DespeckledMaxError << "," << errData.DespeckledAvgError << ",";
	else
		file << "/,/,";
	file << errData.EncodedTextureSize << "\n";

	std::cout << errData.AvgError << std::endl;
}

void SaveError(uint16_t* original, uint16_t* processed, uint8_t* colorBuffer, uint32_t width, uint32_t height, const std::string& path)
{
	DSTR_PROFILE_SCOPE("SaveError");

	uint32_t nElements = width * height;

	for (uint32_t i = 0; i < nElements; i++)
	{
		float err = std::log2(std::abs((int)original[i] - (int)processed[i]));
		uint8_t turboIdx = 256 * (err / std::log2((float)(1 << 16)));

		for (uint32_t j = 0; j < 3; j++)
			colorBuffer[i * 3 + j] = turbo_srgb_bytes[turboIdx][j];
	}

	ImageWriter::WriteError(path + "_error.png", colorBuffer, width, height);
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

template <typename T>
void BenchmarkCoder(uint8_t q, bool enlarge, uint8_t algoBits, uint16_t* src, uint8_t* encoded, uint16_t* decoded, uint8_t* jpegBuffer, uint16_t* original,
	uint32_t width, uint32_t height, std::vector<std::string> path)
{
	uint32_t nElements = width * height;

	std::string currPath = GetPathFromComponents(path);
	StreamCoder<T> coder(q, enlarge, algoBits, true);
	coder.Encode(src, (Color*)encoded, nElements);
	coder.Decode((Color*)encoded, decoded, nElements);
	ImageWriter::WriteDecoded(currPath + "_lossless.png", decoded, width, height);

	// Save with varying jpeg qualities, decode
	for (uint8_t j = 70; j <= 100; j+=10)
	{
		std::stringstream ss;
		ss << "Quality" << (int)j;
		DSTR_PROFILE_SCOPE("Quality");

		{
			DSTR_PROFILE_SCOPE("WriteEncoded");
			ImageWriter::WriteEncoded(currPath + ss.str() + ".jpg", encoded, width, height, ImageFormat::JPG, j);
		}

		{
			DSTR_PROFILE_SCOPE("JpegDecode");

			JpegDecoder decoder; int w, h;
			decoder.setJpegColorSpace(J_COLOR_SPACE::JCS_RGB);
			decoder.decodeNonAlloc((currPath + ss.str() + ".jpg").c_str(), jpegBuffer, w, h);
		}

		{
			DSTR_PROFILE_SCOPE("DStreamDecode");
			coder.Decode((Color*)jpegBuffer, decoded, nElements);
		}

		{
			DSTR_PROFILE_SCOPE("WriteError");
			ImageWriter::WriteDecoded(currPath + ss.str() + "_decoded.png", decoded, width, height);
			SaveError(original, decoded, jpegBuffer, width, height, currPath + ss.str() + "_decoded");
		}

		{
			DSTR_PROFILE_SCOPE("Denoise");
			DepthProcessing::DenoiseMedian(decoded, width, height, 750, 1);
		}

		{
			DSTR_PROFILE_SCOPE("WriteErrorDenoised");
			ImageWriter::WriteDecoded(currPath + ss.str() + "_decoded_denoised.png", decoded, width, height);
			SaveError(original, decoded, jpegBuffer, width, height, currPath + ss.str() + "_decoded_denoised");
		}
	}
}

int main(int argc, char** argv)
{
	DSTR_PROFILE_BEGIN_SESSION("Runtime", "Profile-Runtime.json");

	std::string coders[7] = { "Morton", "Hue", "Hilbert", "Triangle", "Split", "Phase", "Packed" };
	uint8_t quantizations[4] = {10, 12, 14, 16};
	bool enlarge[2] = {true, false};
	bool denoising[2] = { false, true };
	bool tabulate[2] = { false, true };
	std::vector<uint8_t> algoBits;

	// Read raw data
	DepthmapData dmData;
	DepthmapReader reader("envoi_RTI/MNT.asc", DepthmapFormat::ASC, dmData);
	uint32_t nElements = dmData.Width * dmData.Height;

	// Prepare auxiliary buffers
	uint16_t* originalData = reader.GetData();
	uint8_t* encodedData = new uint8_t[nElements * 3];
	uint8_t* jpegBuffer = new uint8_t[nElements * 3];
	uint16_t* decodedData = new uint16_t[nElements];
	uint16_t* quantizedData = new uint16_t[nElements];

	// Folder structures
	std::vector<std::string> folders;
	AddFolderLevel("Output", -1, folders);

	//TestCoder<Triangle>(16, 8);

	for (uint32_t c = 0; c < 1; c++)
	{
		std::cout << "CODER: " << coders[c] << std::endl;
		AddFolderLevel(coders[c], -1, folders);

		for (uint32_t q = 0; q < 4; q++)
		{
			std::cout << "QUANTIZATION: " << (int)quantizations[q] << std::endl;
			AddFolderLevel("Quantization", quantizations[q], folders);

			DepthProcessing::Quantize(originalData, quantizedData, quantizations[q], nElements);
			algoBits = GetAlgoBitsToTest(coders[c], quantizations[q]);

			for (uint32_t p = 0; p < algoBits.size(); p++)
			{
				if (algoBits.size() > 1)
					AddFolderLevel("Parameter", algoBits[p], folders);

				if (!coders[c].compare("Hilbert")) BenchmarkCoder<Hilbert>(quantizations[q], true, algoBits[p], quantizedData, encodedData, decodedData, jpegBuffer, originalData, dmData.Width, dmData.Height, folders);
				if (!coders[c].compare("Morton")) BenchmarkCoder<Morton>(quantizations[q], true, algoBits[p], quantizedData, encodedData, decodedData, jpegBuffer, originalData, dmData.Width, dmData.Height, folders);
				if (!coders[c].compare("Hue")) BenchmarkCoder<Hue>(quantizations[q], true, algoBits[p], quantizedData, encodedData, decodedData, jpegBuffer, originalData, dmData.Width, dmData.Height, folders);
				if (!coders[c].compare("Triangle")) BenchmarkCoder<Triangle>(quantizations[q], true, algoBits[p], quantizedData, encodedData, decodedData, jpegBuffer, originalData, dmData.Width, dmData.Height, folders);
				if (!coders[c].compare("Split")) BenchmarkCoder<Split>(quantizations[q], true, algoBits[p], quantizedData, encodedData, decodedData, jpegBuffer, originalData, dmData.Width, dmData.Height, folders);
				if (!coders[c].compare("Phase")) BenchmarkCoder<Phase>(quantizations[q], true, algoBits[p], quantizedData, encodedData, decodedData, jpegBuffer, originalData, dmData.Width, dmData.Height, folders);
				if (!coders[c].compare("Packed")) BenchmarkCoder<Packed>(quantizations[q], true, algoBits[p], quantizedData, encodedData, decodedData, jpegBuffer, originalData, dmData.Width, dmData.Height, folders);

				if (algoBits.size() > 1)
					folders.pop_back();
			}

			folders.pop_back();
		}
		folders.pop_back();
	}

	delete[] encodedData;
	delete[] decodedData;
	delete[] quantizedData;
	delete[] jpegBuffer;
	//delete[] originalData;

	DSTR_PROFILE_END_SESSION();

	return 0;
}
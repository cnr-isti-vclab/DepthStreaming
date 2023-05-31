#include <StreamCoder.h>
#include <DepthmapReader.h>
#include <DepthProcessing.h>
#include <ImageWriter.h>
#include <ImageReader.h>
#include <JpegDecoder.h>
#include <Timer.h>

#include <Implementations/Hilbert.h>
#include <Implementations/Phase.h>
#include <Implementations/Hue.h>
#include <Implementations/Triangle.h>
#include <Implementations/Packed2.h>
#include <Implementations/Split2.h>
#include <Implementations/Packed3.h>
#include <Implementations/Split3.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <map>
#include <unordered_set>

using namespace DStream;

unsigned char turbo_srgb_bytes[256][3] = { {48,18,59},{50,21,67},{51,24,74},{52,27,81},{53,30,88},{54,33,95},{55,36,102},{56,39,109},{57,42,115},{58,45,121},{59,47,128},{60,50,134},{61,53,139},{62,56,145},{63,59,151},{63,62,156},{64,64,162},{65,67,167},{65,70,172},{66,73,177},{66,75,181},{67,78,186},{68,81,191},{68,84,195},{68,86,199},{69,89,203},{69,92,207},{69,94,211},{70,97,214},{70,100,218},{70,102,221},{70,105,224},{70,107,227},{71,110,230},{71,113,233},{71,115,235},{71,118,238},{71,120,240},{71,123,242},{70,125,244},{70,128,246},{70,130,248},{70,133,250},{70,135,251},{69,138,252},{69,140,253},{68,143,254},{67,145,254},{66,148,255},{65,150,255},{64,153,255},{62,155,254},{61,158,254},{59,160,253},{58,163,252},{56,165,251},{55,168,250},{53,171,248},{51,173,247},{49,175,245},{47,178,244},{46,180,242},{44,183,240},{42,185,238},{40,188,235},{39,190,233},{37,192,231},{35,195,228},{34,197,226},{32,199,223},{31,201,221},{30,203,218},{28,205,216},{27,208,213},{26,210,210},{26,212,208},{25,213,205},{24,215,202},{24,217,200},{24,219,197},{24,221,194},{24,222,192},{24,224,189},{25,226,187},{25,227,185},{26,228,182},{28,230,180},{29,231,178},{31,233,175},{32,234,172},{34,235,170},{37,236,167},{39,238,164},{42,239,161},{44,240,158},{47,241,155},{50,242,152},{53,243,148},{56,244,145},{60,245,142},{63,246,138},{67,247,135},{70,248,132},{74,248,128},{78,249,125},{82,250,122},{85,250,118},{89,251,115},{93,252,111},{97,252,108},{101,253,105},{105,253,102},{109,254,98},{113,254,95},{117,254,92},{121,254,89},{125,255,86},{128,255,83},{132,255,81},{136,255,78},{139,255,75},{143,255,73},{146,255,71},{150,254,68},{153,254,66},{156,254,64},{159,253,63},{161,253,61},{164,252,60},{167,252,58},{169,251,57},{172,251,56},{175,250,55},{177,249,54},{180,248,54},{183,247,53},{185,246,53},{188,245,52},{190,244,52},{193,243,52},{195,241,52},{198,240,52},{200,239,52},{203,237,52},{205,236,52},{208,234,52},{210,233,53},{212,231,53},{215,229,53},{217,228,54},{219,226,54},{221,224,55},{223,223,55},{225,221,55},{227,219,56},{229,217,56},{231,215,57},{233,213,57},{235,211,57},{236,209,58},{238,207,58},{239,205,58},{241,203,58},{242,201,58},{244,199,58},{245,197,58},{246,195,58},{247,193,58},{248,190,57},{249,188,57},{250,186,57},{251,184,56},{251,182,55},{252,179,54},{252,177,54},{253,174,53},{253,172,52},{254,169,51},{254,167,50},{254,164,49},{254,161,48},{254,158,47},{254,155,45},{254,153,44},{254,150,43},{254,147,42},{254,144,41},{253,141,39},{253,138,38},{252,135,37},{252,132,35},{251,129,34},{251,126,33},{250,123,31},{249,120,30},{249,117,29},{248,114,28},{247,111,26},{246,108,25},{245,105,24},{244,102,23},{243,99,21},{242,96,20},{241,93,19},{240,91,18},{239,88,17},{237,85,16},{236,83,15},{235,80,14},{234,78,13},{232,75,12},{231,73,12},{229,71,11},{228,69,10},{226,67,10},{225,65,9},{223,63,8},{221,61,8},{220,59,7},{218,57,7},{216,55,6},{214,53,6},{212,51,5},{210,49,5},{208,47,5},{206,45,4},{204,43,4},{202,42,4},{200,40,3},{197,38,3},{195,37,3},{193,35,2},{190,33,2},{188,32,2},{185,30,2},{183,29,2},{180,27,1},{178,26,1},{175,24,1},{172,23,1},{169,22,1},{167,20,1},{164,19,1},{161,18,1},{158,16,1},{155,15,1},{152,14,1},{149,13,1},{146,11,1},{142,10,1},{139,9,2},{136,8,2},{133,7,2},{129,6,2},{126,5,2},{122,4,3} };
std::string outputFolder = "FastTest";

enum class ImageFormat
{
	JPG = 0
	, PNG
#ifdef DSTREAM_ENABLE_WEBP
	, WEBP, SPLIT_WEBP
#endif
};

struct ErrorData
{
	float MaxError = -1e20;
	float AvgError = 0.0f;

	float DespeckledMaxError = -1e20;
	float DespeckledAvgError = 0.0f;

	uint32_t EncodedTextureSize;
};

struct BenchmarkConfig
{
	std::string CoderName;
	uint8_t AlgoBits;

	bool Enlarge;
	bool Interpolate;

	uint8_t* ColorBuffer;
	uint8_t* EncodedBuffer;
	float* RawData;
	uint16_t* QuantizedData;
	uint16_t* DecodedData;
	std::vector<uint8_t> ChannelDistribution = { 0,0,0 };

	uint32_t Width;
	uint32_t Height;
	ImageFormat OutputFormat;

	std::string CurrentPath;
};

std::vector<uint8_t> GetAlgoBitsToTest(const std::string& algo)
{
	std::vector<uint8_t> ret;

	if (!algo.compare("Morton"))
		return { 6 };
	else if (!algo.compare("Hilbert"))
	{
		for (uint32_t i = 0; i < 5; i++)
			ret.push_back(i + 1);

		return ret;
	}
	else if (!algo.compare("Packed2") || (!algo.compare("Split2")))
	{
		for (uint32_t i = 2; i <= 8; i+=2)
			ret.push_back(i);
		return ret;
	}
	else if (!algo.compare("Triangle"))
		return { 6 };
	else if (!algo.compare("Phase"))
		return { 6 };
	else
		return { 6 };
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

void AddBenchmarkResult(std::ofstream& file, std::string algo, int jpegQuality, int parameter, const ErrorData& errData)
{
	std::stringstream configName;
	std::string cleanAlgo = algo.substr(algo.find_last_of(":") + 1, algo.length());

	configName << cleanAlgo << "_Q:" << 16;
	configName << "_J:" << jpegQuality;
	if (!cleanAlgo.compare("Hilbert") || !cleanAlgo.compare("Packed") || !cleanAlgo.compare("Split"))
		configName << "_P:" << parameter;

	//"Configuration, Max Error, Avg Error, Despeckle Max Error, Despeckle Avg Error, Compressed Size\n";
	file << configName.str() << "," << errData.MaxError << "," << errData.AvgError << ",";
	if (errData.DespeckledMaxError >= 0.0f)
		file << errData.DespeckledMaxError << "," << errData.DespeckledAvgError << ",";
	else
		file << "/,/,";
	file << errData.EncodedTextureSize << "\n";
}

void SaveError(uint16_t* original, uint16_t* processed, uint8_t* colorBuffer, uint32_t width, uint32_t height, const std::string& path, ErrorData& errData)
{
	std::map<float, int, std::less<float>> errorFrequencies;

	std::ofstream histogram(path + "histo.csv");
	int nBars = 20;

	{
		DSTR_PROFILE_SCOPE("ComputeLogErr");

		uint32_t nElements = width * height;
		float errSum = 0;
		float maxErr = -1;
		float log216 = std::log2((float)(1 << 16));
		
		for (uint32_t i = 0; i < nElements; i++)
		{
			float err = std::log2(1.0f + std::abs((int)original[i] - (int)processed[i]));
			uint8_t turboIdx = 256 * (err / log216);

			float errCopy = err;

			if (errorFrequencies.find(errCopy) == errorFrequencies.end()) 
				errorFrequencies[errCopy] = 1;
			else 
				errorFrequencies[errCopy]++;

			errSum += err;
			maxErr = std::max(maxErr, err);

			for (uint32_t j = 0; j < 3; j++)
				colorBuffer[i * 3 + j] = turbo_srgb_bytes[turboIdx][j];

			if (err > 15)
				for (uint32_t j = 0; j < 3; j++)
					colorBuffer[i * 3 + j] = 255;
		}


		// Create histogram
		int advance = std::ceil((float)errorFrequencies.size() / nBars);
		int idx = 0;
		float freqSum = 0;
		float minErrBound = 20;
		float maxErrBound = -1;

		for (auto freq : errorFrequencies)
		{
			if (idx < advance)
			{
				idx++;
				minErrBound = std::min(minErrBound, freq.first);
				maxErrBound = std::max(maxErrBound, freq.first);
				freqSum += freq.second;
			}
			else
			{
				histogram << minErrBound << "-" << maxErrBound << "," << freqSum << std::endl;
				minErrBound = 20;
				maxErrBound = -1;
				idx = 0;
				freqSum = 0;
			}
		}

		// Save error data
		errSum /= nElements;
		if (path.find("denoised") != std::string::npos)
		{
			errData.DespeckledAvgError = errSum;
			errData.DespeckledMaxError = maxErr;
		}
		else
		{
			errData.AvgError = errSum;
			errData.MaxError = maxErr;
		}
	}

	{
		DSTR_PROFILE_SCOPE("WriteErrorTexture");
		ImageWriter::WritePNG(path + "_error.png", colorBuffer, width, height);
	}
}

template <typename Coder>
void TestCoder(uint32_t algo, std::vector<uint8_t> config = { 8,8,8 })
{
	float avg = 0;
	int max = 0;

	bool interpolate = true;
	bool enlarge = true;
	bool tables = false;

	StreamCoder<Coder> sc(enlarge, interpolate, algo, config, tables);
	for (uint16_t i = 0; i < 65535; i++)
	{
		Color c;
		sc.Encode(&i, &c, 1);

		uint16_t v;
		sc.Decode(&c, &v, 1);

		if (v != i)
		{
			int err = std::abs((int)v - (int)i);
			avg += err;
			max = std::max<int>(max, err);
		}
	}

	avg /= 65535;
	std::cout << "Max: " << max << "Avg: " << avg << std::endl;
}

template <typename T>
void BenchmarkCoder(BenchmarkConfig& config)
{
	uint32_t width = config.Width, height = config.Height;
	uint32_t nElements = width * height;
	uint8_t jpegLevels[5] = {70, 80, 90, 95, 100};

	std::string currPath = config.CurrentPath;
	std::ofstream csv(outputFolder+"/results.csv", std::ios::out | std::ios::app);

	DepthProcessing::Quantize(config.QuantizedData, config.RawData, 16, nElements);

	std::cout << "ALGO BITS: " << (int)config.AlgoBits << std::endl;

	// Always use tables, they're free
	StreamCoder<T> coder(config.Enlarge, config.Interpolate, config.AlgoBits, config.ChannelDistribution, true);
	coder.Encode(config.QuantizedData, (Color*)config.EncodedBuffer, nElements);
	coder.Decode((Color*)config.EncodedBuffer, config.DecodedData, nElements);
	
	ImageWriter::WriteDecoded(currPath + "_lossless.png", config.DecodedData, width, height);

	ErrorData dummy;
	SaveError(config.QuantizedData, config.DecodedData, config.ColorBuffer, width, height, currPath + "_lossless_error.png", dummy);

	uint32_t minQuality = 4;
	uint32_t maxQuality = 4;

	if (config.OutputFormat == ImageFormat::JPG 
#ifdef DSTREAM_ENABLE_WEBP
		|| config.OutputFormat == ImageFormat::SPLIT_WEBP || config.OutputFormat == ImageFormat::WEBP
#endif
		)
		minQuality = 0;

	// Save with varying jpeg qualities, decode
	for (uint8_t j = minQuality; j <= maxQuality; j++)
	{
		ErrorData err;

		std::stringstream ss;
		ss << "Quality" << (int)jpegLevels[j];
		DSTR_PROFILE_SCOPE("Quality");
		{
			std::cout << "Encode" << std::endl;
			DSTR_PROFILE_SCOPE("WriteEncoded");
			switch (config.OutputFormat)
			{
			case ImageFormat::JPG: ImageWriter::WriteJPEG(currPath + ss.str() + ".jpg", config.EncodedBuffer, width, height, jpegLevels[j]); break;
			case ImageFormat::PNG: ImageWriter::WritePNG(currPath + ss.str() + ".png", config.EncodedBuffer, width, height); break;
#ifdef DSTREAM_ENABLE_WEBP
			case ImageFormat::WEBP: ImageWriter::WriteWEBP(currPath + ss.str() + ".webp", config.EncodedBuffer, width, height, jpegLevels[j]); break;
			case ImageFormat::SPLIT_WEBP: ImageWriter::WriteSplitWEBP(currPath + ss.str(), config.EncodedBuffer, width, height, jpegLevels[j]); break;
#endif
			}
		}

		{
			std::cout << "Read encoded" << std::endl;
			DSTR_PROFILE_SCOPE("ImageDecode");
			switch (config.OutputFormat)
			{
			case ImageFormat::JPG: ImageReader::ReadJPEG(currPath + ss.str() + ".jpg", config.ColorBuffer); break;
			case ImageFormat::PNG: ImageReader::ReadPNG(currPath + ss.str() + ".png", config.ColorBuffer); break;
#ifdef DSTREAM_ENABLE_WEBP
			case ImageFormat::WEBP: ImageReader::ReadWEBP(currPath + ss.str() + ".webp", config.ColorBuffer, nElements * 3); break;
			case ImageFormat::SPLIT_WEBP: ImageReader::ReadSplitWEBP(currPath + ss.str(), config.ColorBuffer, nElements * 3); break;
#endif
			}
		}

		std::cout << "Decode" << std::endl;
		{
			DSTR_PROFILE_SCOPE("DStreamDecode");
			coder.Decode((Color*)config.ColorBuffer, config.DecodedData, nElements);
		}

		std::cout << "Write Decoded" << std::endl;
		{
			{
				DSTR_PROFILE_SCOPE("WriteDecoded");
				ImageWriter::WriteDecoded(currPath + ss.str() + "_decoded.png", config.DecodedData, width, height);
			}
			SaveError(config.QuantizedData, config.DecodedData, config.ColorBuffer, width, height, currPath + ss.str() + "_decoded", err);
		}

		std::string extension;
		switch (config.OutputFormat)
		{
		case ImageFormat::JPG: extension = ".jpg"; break;
		case ImageFormat::PNG: extension = ".png"; break;
#ifdef DSTREAM_ENABLE_WEBP
		case ImageFormat::WEBP: extension = ".webp"; break;
		case ImageFormat::SPLIT_WEBP: extension = ""; break;
#endif
		}
		
		std::cout << "File size" << std::endl;
		if (config.OutputFormat == ImageFormat::SPLIT_WEBP)
			err.EncodedTextureSize = std::filesystem::file_size(currPath + ss.str() + extension + ".red.splitwebp") +
			std::filesystem::file_size(currPath + ss.str() + extension + ".green.splitwebp");
		else
			err.EncodedTextureSize = std::filesystem::file_size(currPath + ss.str() + extension);

		if (config.CoderName == "Packed3" || config.CoderName == "Split3")
		{
			std::stringstream ss;
			ss << (int)config.ChannelDistribution[0] << (int)config.ChannelDistribution[1] << (int)config.ChannelDistribution[2];
			config.CoderName += ss.str();
		}
		AddBenchmarkResult(csv, config.CoderName, jpegLevels[j], config.AlgoBits, err);
	}

	csv.close();
}

int main(int argc, char** argv)
{
	DSTR_PROFILE_BEGIN_SESSION("Runtime", "Profile-Runtime.json");
	
	// Parameters to test
	std::string coders[7] = { "Triangle", "Hilbert", "Split2", "Hue", "Packed2", "Hue", "Phase" };
	std::vector<uint8_t> algoBits;

	// Read raw data
	DepthmapData dmData;
	DepthmapReader reader("Input/2.tif", DepthmapFormat::TIF, dmData);
	uint32_t nElements = dmData.Width * dmData.Height;

	// Prepare auxiliary buffers
	float* rawData = reader.GetRawData();
	uint16_t* originalData = new uint16_t[nElements];
	uint8_t* encodedData = new uint8_t[nElements * 3];
	uint8_t* colorBuffer = new uint8_t[nElements * 3];
	uint16_t* decodedData = new uint16_t[nElements];
	std::vector<std::vector<uint8_t>> distributions = {
		{2,6,8},{2,7,7},{3,5,8},{3,6,7},{4,4,8},{4,5,7},{4,6,6},{5,5,6}
	};

	// Folder structures
	std::vector<std::string> folders;
	AddFolderLevel(outputFolder, -1, folders);

	// CSV File
	std::ofstream csv(outputFolder + "/results.csv");
	csv.clear();
	csv << "Configuration, Max Error, Avg Error, Despeckle Max Error, Despeckle Avg Error, Compressed Size\n";
	csv.close();

	for (uint32_t c = 0; c < 7; c++)
	{
		std::cout << "CODER: " << coders[c] << std::endl;
		AddFolderLevel(coders[c], -1, folders);

		BenchmarkConfig config;
		config.Width = dmData.Width;
		config.Height = dmData.Height;
		config.ColorBuffer = colorBuffer;
		config.RawData = rawData;
		config.QuantizedData = originalData;
		config.EncodedBuffer = encodedData;
		config.DecodedData = decodedData;
		config.Enlarge = false;
		config.Interpolate = true;
		config.OutputFormat = ImageFormat::JPG;
		config.CoderName = coders[c];

		if (coders[c] == "Packed3" || coders[c] == "Split3")
		{
			for (uint32_t d = 0; d < distributions.size(); d++)
			{
				std::stringstream ss;
				ss << (int)distributions[d][0] << (int)distributions[d][1] << (int)distributions[d][2];
				AddFolderLevel("Distribution" + ss.str(), -1, folders);
					
				config.ChannelDistribution = distributions[d];
				config.CurrentPath = GetPathFromComponents(folders);

				if (coders[c] == "Packed3")	BenchmarkCoder<Packed3>(config);
				if (coders[c] == "Split3")	BenchmarkCoder<Split3>(config);

				folders.pop_back();
			}
		}
		else
		{
			algoBits = GetAlgoBitsToTest(coders[c]);
			for (uint32_t p = 0; p < algoBits.size(); p++)
			{
				if (algoBits.size() > 1)
					AddFolderLevel("Parameter", algoBits[p], folders);

				config.AlgoBits = algoBits[p];
				config.CurrentPath = GetPathFromComponents(folders);

				if (!coders[c].compare("Hilbert"))BenchmarkCoder<Hilbert>(config);
				if (!coders[c].compare("Hue")) BenchmarkCoder<Hue>(config);
				if (!coders[c].compare("Phase")) BenchmarkCoder<Phase>(config);
				if (!coders[c].compare("Triangle")) BenchmarkCoder<Triangle>(config);
				if (!coders[c].compare("Split2")) BenchmarkCoder<Split2>(config);
				if (!coders[c].compare("Packed2")) BenchmarkCoder<Packed2>(config);
				if (algoBits.size() > 1)
					folders.pop_back();
			}
		}

		folders.pop_back();
	}

	delete[] encodedData;
	delete[] decodedData;
	delete[] colorBuffer;
	delete[] originalData;

	DSTR_PROFILE_END_SESSION();

	return 0;
}

#include <cstdio>
#include <cstring>
#include <string>
#include <filesystem>

#include <iostream>
#include <sstream>
#include <fstream>

#include <DepthmapReader.h>
#include <DepthProcessing.h>
#include <ImageReader.h>
#include <ImageWriter.h>

#include <StreamCoder.h>
#include <Implementations/Hilbert.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed2.h>
#include <Implementations/Packed3.h>
#include <Implementations/Phase.h>
#include <Implementations/Split2.h>
#include <Implementations/Split3.h>
#include <Implementations/Morton.h>
#include <Implementations/Triangle.h>

using namespace DStream;

#ifndef _WIN32
#include <unistd.h>
#else

int opterr = 1, optind = 1, optopt, optreset;
const char* optarg;
int getopt(int nargc, char* const nargv[], const char* ostr);
#endif

StreamCoder<Hilbert> hilbertCoder;
StreamCoder<Packed3> packedCoder;
StreamCoder<Split3> splitCoder;
StreamCoder<Triangle> triangleCoder;
StreamCoder<Phase> phaseCoder;
StreamCoder<Hue> hueCoder;
StreamCoder<Morton> mortonCoder;

void Usage()
{
    std::cerr <<
        R"use(Usage: dstream-cmd [OPTIONS] <DIRECTORY>

    DIRECTORY is the path to the folder containing the depth data
      -d <output>: output folder in which final data will be saved
      -f <format>: file format to which data will be encoded or from which it will be decoded. Choose one between JPG, PNG, WEBP, LOSSY_WEBP, SPLIT_WEBP, defaults to WEBP. 
                    When decoding, the format is deduced from the file extension. Specify the format if you only want to decode a given format
      -r <recursive>: navigate the input directory recursively and process all the files contained in it
      -a <algorithm>: algorithm to be used (algorithm names: PACKED, TRIANGLE, MORTON, HILBERT, PHASE, SPLIT, HUE)
      -q <quantization>: quantization level
      -n <no quantize>: don't quantize raw height data between 0 and 65535. This is normally done prior to quantizing to the specified quantization level.
      -b <bits>: number of bits dedicated to the algorithm (only used by Hilbert and Packed). The remaining ones will be used to enlarge / shrink data
      -e <no enlarge>: don't use the whole 8 bit range of colours if encoded colours end up using less
      -j <quality>: quality to use if encoding, only applies to WEBP and PNG
      -m <mode>: program mode, E for encoding, D for decoding
      -p <print>: print the decoded texture in PNG format, 8 bit grayscale
      -?: display this message
      -h: display this message

    )use";
}

int ParseOptions(int argc, char** argv, std::string& inDir, std::string& outDir, std::string& algo, uint8_t& quantization, uint8_t& jpeg, 
    uint8_t& algoBits,  bool& recursive, std::string& mode, std::string& outputFormat, bool& enlarge, bool& quantize, bool& printTexture)
{
    int c;
    recursive = false;
    enlarge = true;
    quantize = true;


    while ((c = getopt(argc, argv, "d:a:q:j:b:m:f:rpenh::")) != -1) {
        switch (c) {
        case 'd':
        {
            if (!std::filesystem::exists(optarg))
                std::filesystem::create_directory(optarg);
            outDir = optarg;
            break;
        }
        case 'a':
        {
            std::string arg(optarg);
            if (arg == "PACKED" || arg == "TRIANGLE" || arg == "MORTON" || arg == "HILBERT" || arg == "PHASE" || arg == "SPLIT")
            {
                algo = optarg;
                break;
            }
            else
            {
                std::cerr << "Unknown algorithm " << arg << std::endl;
                Usage();
                return -1;
            }
        }
        case 'q':
        {
            int q = atoi(optarg);
            if (q >= 10 && q <= 16)
            {
                quantization = q;
                break;
            }
            else
            {
                std::cerr << "Quantization should be in the range of [10, 16]" << std::endl;
                return -2;
            }
            
        }
        case 'j':
        {
            int j = atoi(optarg);
            if (j >= 0 && j <= 100)
                jpeg = j;
            else
            {
                std::cerr << "JPEG quality should be in the range of [0, 100]" << std::endl;
                return -3;
            }
            break;
        }
        case 'b':
        {
            int b = atoi(optarg);
            if (b >= 1 && b <= 8)
            {
                algoBits = b;
                break;
            }
            else
            {
                std::cerr << "Number of bits dedicated to the algorithm should be in the range of [1,8]" << std::endl;
                return -4;
            }
        }
        case 'f':
        {
            outputFormat = optarg;
            if (outputFormat != "PNG" && outputFormat != "JPG" 
#ifdef DSTREAM_ENABLE_WEBP
                && outputFormat != "WEBP" && 
                outputFormat != "LOSSY_WEBP" && outputFormat != "SPLIT_WEBP"
#endif
                )
            {
                std::cerr << "Unknown format \"" << outputFormat << "\"" << std::endl;
                Usage();
                return -5;
            }
            break;
        }
        case 'm':
            mode = optarg;
            if (mode != "D" && mode != "E")
            {
                std::cerr << "Unknown program mode " << mode << std::endl;
                Usage();
                return -6;
            }
            break;
        case 'e':
            enlarge = true;
            break;
        case 'n':
            quantize = false;
            break;
        case 'p':
            printTexture = true;
            break;
        case 'r':
            recursive = true;
            break;
        case 'h':
        case '?': Usage(); return -5;
        default:
            std::cerr << "Unknown option: " << (char)c << std::endl;
            Usage();
            return -6;
        }
    }
    if (optind == argc) {
        std::cerr << "Missing input folder path" << std::endl;
        Usage();
        return -7;
    }

    if (optind != argc - 1) {
#ifdef _WIN32
        std::cerr << "Too many arguments or argument before other options\n";
#else
        std::cerr << "Too many arguments\n";
#endif
        Usage();
        return -8;
    }

    inDir = argv[optind];
    if (!std::filesystem::exists(inDir))
    {
        std::cerr << "Specified input folder does not exist." << std::endl;
        return -9;
    }
    return 0;
}

int ValidateInput(const std::string& algorithm, uint8_t quantization, uint8_t jpeg, uint8_t algoBits, const std::string& mode, const std::string& format)
{
    if (algorithm == "")
    {
        std::cout << "Unspecified coding algorithm." << std::endl;
        return -1;
    }

    if (mode == "")
    {
        std::cout << "Unspecified program mode." << std::endl;
        return -1;
    }

    if (mode == "D" && jpeg <= 100)
        std::cout << "Image quality specified, but DECODING mode is set. The quality parameter will be ignored." << std::endl;
    if (jpeg <= 100 && (format == "WEBP" || format == "PNG"))
        std::cout << "Image quality specified, but selected format is lossless. The quality parameter will be ignored." << std::endl;

    if (algorithm == "Hilbert")
    {
        uint8_t segmentBits = quantization - 3 * algoBits;

        if (!(algoBits * 3 < quantization && algoBits + segmentBits <= 8))
        {
            std::cerr << "The specified amount of bits reserved for the Hilbert algorithm is not enough for the given quantization level." << std::endl;
            return -1;
        }
    }

    return 0;
}

void Encode(uint16_t* input, Color* output, uint32_t nElements, const std::string& coder)
{
    if (coder == "PACKED") packedCoder.Encode(input, (Color*)output, nElements);
    else if (coder == "HUE") hueCoder.Encode(input, (Color*)output, nElements);
    else if (coder == "HILBERT") hilbertCoder.Encode(input, (Color*)output, nElements);
    else if (coder == "MORTON") mortonCoder.Encode(input, (Color*)output, nElements);
    else if (coder == "SPLIT") splitCoder.Encode(input, (Color*)output, nElements);
    else if (coder == "PHASE") phaseCoder.Encode(input, (Color*)output, nElements);
    else triangleCoder.Encode(input, (Color*)output, nElements);
}

void Decode(uint8_t* input, uint16_t* output, uint32_t nElements, const std::string& coder)
{
    if (coder == "PACKED") packedCoder.Decode((Color*)input, output, nElements);
    else if (coder == "HUE") hueCoder.Decode((Color*)input, output, nElements);
    else if (coder == "HILBERT") hilbertCoder.Decode((Color*)input, output, nElements);
    else if (coder == "MORTON") mortonCoder.Decode((Color*)input, output, nElements);
    else if (coder == "SPLIT") splitCoder.Decode((Color*)input, output, nElements);
    else if (coder == "PHASE") phaseCoder.Decode((Color*)input, output, nElements);
    else triangleCoder.Decode((Color*)input, output, nElements);
}

std::vector<std::filesystem::path> GetFiles(const std::filesystem::path& path, bool recursive, const std::string inputPrefix, const std::string outDir, char codingMode)
{
    std::vector<std::filesystem::path> ret;

    for (auto file : std::filesystem::directory_iterator(path))
    {
        if (file.is_directory() && recursive)
        {
            // Create mirror directory in the output folder
            std::filesystem::create_directory(outDir + file.path().string().substr(inputPrefix.length(), file.path().string().length() - inputPrefix.length()));
            std::vector<std::filesystem::path> toAppend = GetFiles(file, recursive, inputPrefix, outDir, codingMode);
            ret.insert(ret.end(), toAppend.begin(), toAppend.end());
        }
        else if (!file.is_directory())
        {
            std::string ext = file.path().extension().string();
            for (uint32_t i = 0; i < ext.length(); i++)
                ext[i] = std::tolower(ext[i]);

            if ((codingMode == 'E') && (ext == ".asc" || ext == ".pgm"
#ifdef DSTREAM_ENABLE_TIFF
                || ext == ".tif" || ext == ".tiff")
#endif
                )
                ret.push_back(file);
            else if ((codingMode == 'D') && (ext == ".png" || ext == ".jpg" || ext == ".jpeg"
#ifdef DSTREAM_ENABLE_WEBP
                || ext == ".webp" || ext == ".splitwebp")
#endif
                )
            {
#ifdef DSTREAM_ENABLE_WEBP
                if (ext == ".splitwebp")
                {
                    // Remove .green / .red, add .splitwebp extension to filename
                    // Parent/file.green or Parent/file.red
                    std::string fileString = file.path().string();
                    std::string mainPart = fileString.substr(0, fileString.find_last_of("."));
                    // Parent/file
                    std::string parentName = mainPart.substr(0, mainPart.find_last_of(".")) + ".splitwebp";

                    if (std::find(ret.begin(), ret.end(), parentName) == ret.end())
                        ret.push_back(parentName);
                }
                else
                    ret.push_back(file);
#else
                ret.push_back(file);
#endif
            }
        }
    }

    return ret;
}

int main(int argc, char** argv)
{
    std::unordered_map<std::string, std::string> inPath2OutPath;
    bool saveDecoded = true, recursive = false, enlarge = true, quantize;
    uint8_t quantization = 14, jpeg = 100, algoBits = 8;
    std::string inDir, outDir = "", algorithm = "-", mode = "-", outputFormat = "JPG";

    if (ParseOptions(argc, argv, inDir, outDir, algorithm, quantization, jpeg, algoBits, recursive, mode, outputFormat, enlarge, quantize, saveDecoded) != 0)
        return -1;
    if (ValidateInput(algorithm, quantization, jpeg, algoBits, mode, outputFormat) != 0)
    {
        Usage();
        return -2;
    }

    if (outDir == "")
    {
        std::cout << "Unspecified output directory, will save data in the input directory. Files won't be overwritten." << std::endl;
        outDir = inDir;
    }

    std::filesystem::path inputDir = std::filesystem::path(inDir);
    // If encoding, add all files supported by the DepthmapReader. If decoding, add all formats supported by the ImageReader
    std::vector<std::filesystem::path> files = GetFiles(inputDir, recursive, inDir, outDir, mode[0]);

    if (algorithm == "HILBERT") hilbertCoder = StreamCoder<Hilbert>(quantization, enlarge, algoBits, { 8,8,8 },false);
    if (algorithm == "PACKED") packedCoder = StreamCoder<Packed3>(quantization, enlarge, algoBits, { 8,8,8 }, true);
    if (algorithm == "SPLIT") splitCoder = StreamCoder<Split3>(quantization, enlarge, algoBits, { 8,8,8 }, true);
    if (algorithm == "TRIANGLE") triangleCoder = StreamCoder<Triangle>(quantization, enlarge, algoBits, { 8,8,8 }, true);
    if (algorithm == "PHASE") phaseCoder = StreamCoder<Phase>(quantization, enlarge, algoBits, { 8,8,8 }, true);
    if (algorithm == "HUE") hueCoder = StreamCoder<Hue>(quantization, enlarge, algoBits, { 8,8,8 }, true);
    if (algorithm == "MORTON") mortonCoder = StreamCoder<Morton>(quantization, enlarge, algoBits, { 8,8,8 }, true);

    for (auto file : files)
    {
        std::string outPath;
        uint32_t inputIdx = file.string().find(inDir);
        outPath = outDir + "/" + file.string().substr(inDir.length(), file.string().length() - inDir.length());

        if (mode == "E")
        {
            DepthmapData dmData;
            DepthmapReader reader(file.string(), dmData);
            uint32_t nElements = dmData.Width * dmData.Height;

            float* rawData = reader.GetRawData();
            uint16_t* depthData = new uint16_t[dmData.Width * dmData.Height];
            DepthProcessing::Quantize(depthData, rawData, quantization, nElements);

            uint8_t* encoded = new uint8_t[nElements * 3];
            if (quantize)
                DepthProcessing::Quantize(depthData, depthData, quantization, nElements);
            Encode(depthData, (Color*)encoded, nElements, algorithm);

            if (outputFormat == "JPG") 
                ImageWriter::WriteJPEG(outPath + "_encoded.jpg", encoded, dmData.Width, dmData.Height, jpeg);
            else if (outputFormat == "PNG") 
                ImageWriter::WritePNG(outPath + "_encoded.png", encoded, dmData.Width, dmData.Height);
#ifdef DSTREAM_ENABLE_WEBP
            else if (outputFormat == "WEBP")
                ImageWriter::WriteWEBP(outPath + "_encoded.webp", encoded, dmData.Width, dmData.Height);
            else if (outputFormat == "LOSSY_WEBP")
                ImageWriter::WriteWEBP(outPath + "_encoded.webp", encoded, dmData.Width, dmData.Height, jpeg);
            else if (outputFormat == "SPLIT_WEBP")
                ImageWriter::WriteSplitWEBP(outPath + "_encoded", encoded, dmData.Width, dmData.Height, jpeg);
#endif
            delete[] encoded;
            delete[] depthData;
        }
        else
        {
            int width, height, comp;
            ImageReader::GetImageSize(file.string(), &width, &height, &comp, file.extension().string());
            uint32_t nElements = width * height;

            uint16_t* decoded = new uint16_t[nElements];
            uint8_t* encoded = new uint8_t[nElements * 3];

            ImageReader::Read(file.string(), encoded, nElements * 3);
            Decode(encoded, decoded, nElements, algorithm);
            DepthProcessing::Dequantize(decoded, decoded, quantization, nElements);

            if (saveDecoded)
                ImageWriter::WriteDecoded(outPath + "_decoded.png", decoded, width, height);

            std::stringstream ss;
            std::vector<std::string> values(nElements);
            for (uint32_t i = 0; i < nElements; i++)
                values[i] = std::to_string(decoded[i]);

            std::ofstream csv(outPath + "_decoded.csv");
            for (uint32_t y = 0; y < height; y++)
            {
                for (uint32_t x=0; x<width; x++)
                    ss << std::to_string(decoded[y * width + x]) + ",";
                ss << '\n';
            }

            csv << ss.str();
            delete[] decoded;
        }
    }

	return 0;
}


#ifdef _WIN32

int getopt(int nargc, char* const nargv[], const char* ostr) {
    static const char* place = "";        // option letter processing
    const char* oli;                      // option letter list index

    if (optreset || !*place) {             // update scanning pointer
        optreset = 0;
        if (optind >= nargc || *(place = nargv[optind]) != '-') {
            place = "";
            return -1;
        }

        if (place[1] && *++place == '-') { // found "--"
            ++optind;
            place = "";
            return -1;
        }
    }                                       // option letter okay?

    if ((optopt = (int)*place++) == (int)':' || !(oli = strchr(ostr, optopt))) {
        // if the user didn't specify '-' as an option,  assume it means -1.
        if (optopt == (int)'-')
            return (-1);
        if (!*place)
            ++optind;
        if (opterr && *ostr != ':')
            std::cout << "illegal option -- " << optopt << "\n";
        return ('?');
    }

    if (*++oli != ':') {                    // don't need argument
        optarg = NULL;
        if (!*place)
            ++optind;

    }
    else {                                // need an argument
        if (*place)                         // no white space
            optarg = place;
        else if (nargc <= ++optind) {       // no arg
            place = "";
            if (*ostr == ':')
                return (':');
            if (opterr)
                std::cout << "option requires an argument -- " << optopt << "\n";
            return (':');
        }
        else                              // white space
            optarg = nargv[optind];
        place = "";
        ++optind;
    }
    return optopt;                          // dump back option letter
}

#endif

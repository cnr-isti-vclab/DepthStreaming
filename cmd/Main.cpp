#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <filesystem>

#include <DepthmapReader.h>
#include <ImageReader.h>
#include <ImageWriter.h>

#include <StreamCoder.h>
#include <Implementations/Hilbert.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed.h>
#include <Implementations/Phase.h>
#include <Implementations/Split.h>
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
StreamCoder<Packed> packedCoder;
StreamCoder<Split> splitCoder;
StreamCoder<Triangle> triangleCoder;
StreamCoder<Phase> phaseCoder;
StreamCoder<Hue> hueCoder;
StreamCoder<Morton> mortonCoder;


void Usage()
{
    std::cerr <<
        R"use(Usage: dstream-cmd [OPTIONS] <DIRECTORY>

    DIRECTORY is the path to the folder containing the depth data
      -d <output>: output folder in which final data will be saved+
      -f <format>: file format to which data will be encoded or from which it will be decoded. Choose one between JPG, PNG, WEBP, LOSSY_WEBP, SPLIT_WEBP, defaults to WEBP
      -r <recursive>: navigate the input directory recursively and process all the files contained in it
      -a <algorithm>: algorithm to be used (algorithm names: PACKED, TRIANGLE, MORTON, HILBERT, PHASE, SPLIT, HUE)
      -q <quantization>: quantization level
      -n <no quantize>: don't quantize raw height data between 0 and 65535. This is normally done prior to quantizing to the specified quantization level.
      -b <bits>: number of bits dedicated to the algorithm (only used by Hilbert and Packed). The remaining ones will be used to enlarge / shrink data
      -e <no enlarge>: don't use the whole 8 bit range of colours if encoded colours end up using less
      -j <quality>: quality to use if encoding, only applies to WEBP and PNG
      -m <mode>: program mode, E for encoding, D for decoding
      -?: display this message
      -h: display this message

    )use";
}

int ParseOptions(int argc, char** argv, std::string& inDir, std::string& outDir, std::string& algo, uint8_t& quantization, uint8_t& jpeg, 
    uint8_t& algoBits,  bool& recursive, std::string& mode, std::string& outputFormat, bool& enlarge, bool& quantize)
{
    int c;
    recursive = false;
    enlarge = true;


    while ((c = getopt(argc, argv, "d:a:q:j::b:m:rfenh::")) != -1) {
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
            if (outputFormat != "PNG" && outputFormat != "JPG" && outputFormat != "WEBP" && 
                outputFormat != "LOSSY_WEBP" && outputFormat != "SPLIT_WEBP")
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

void Encode(uint16_t* input, Color* output, uint32_t nElements, const std::string& coder)
{
    if (coder == "PACKED") packedCoder.Encode(input, (Color*)output, nElements);
    else if (coder == "HUE") hueCoder.Encode(input, (Color*)output, nElements);
    else if (!coder.compare("Hilbert")) hilbertCoder.Encode(input, (Color*)output, nElements);
    else if (!coder.compare("Morton")) mortonCoder.Encode(input, (Color*)output, nElements);
    else if (!coder.compare("Split")) splitCoder.Encode(input, (Color*)output, nElements);
    else if (!coder.compare("Phase")) phaseCoder.Encode(input, (Color*)output, nElements);
    else triangleCoder.Encode(input, (Color*)output, nElements);
}

void Decode(const std::filesystem::path& filePath, const std::string& outputDirectory, const std::string& coder, uint8_t quantization, uint8_t jpeg, uint8_t algoBits)
{

}

std::vector<std::filesystem::path> GetFiles(const std::filesystem::path& path, bool recursive)
{
    std::vector<std::filesystem::path> ret;

    for (auto file : std::filesystem::directory_iterator(path))
    {
        if (file.is_directory() && recursive)
        {
            std::vector<std::filesystem::path> toAppend = GetFiles(file, recursive);
            ret.insert(ret.end(), toAppend.begin(), toAppend.end());
        }
        else
            ret.push_back(file);
    }

    return ret;
}



int main(int argc, char** argv)
{
    bool saveDecoded, recursive, enlarge, quantize;
    uint8_t quantization = 9, jpeg = 101, algoBits = 9;
    std::string inDir, outDir, algorithm, mode, outputFormat = "WEBP";

    if (ParseOptions(argc, argv, inDir, outDir, algorithm, quantization, jpeg, algoBits, recursive, mode, outputFormat, enlarge, quantize) != 0)
        return -1;
    if (ValidateInput(algorithm, quantization, jpeg, algoBits, mode, outputFormat) != 0)
        return -2;

    std::filesystem::path inputDir = std::filesystem::path(inDir);
    std::vector<std::filesystem::path> files = GetFiles(inputDir, recursive);

    if (algorithm == "HILBERT") hilbertCoder = StreamCoder<Hilbert>(quantization, enlarge, algoBits, true);
    if (algorithm == "PACKED") packedCoder = StreamCoder<Packed>(quantization, enlarge, algoBits, true);
    if (algorithm == "SPLIT") splitCoder = StreamCoder<Split>(quantization, enlarge, algoBits, true);
    if (algorithm == "TRIANGLE") triangleCoder = StreamCoder<Triangle>(quantization, enlarge, algoBits, true);
    if (algorithm == "PHASE") phaseCoder = StreamCoder<Phase>(quantization, enlarge, algoBits, true);
    if (algorithm == "HUE") hueCoder = StreamCoder<Hue>(quantization, enlarge, algoBits, true);
    if (algorithm == "MORTON") mortonCoder = StreamCoder<Morton>(quantization, enlarge, algoBits, true);

    for (auto file : files)
    {
        DepthmapData dmData;
        DepthmapReader reader(file.string(), dmData, true);

        uint16_t* depthData = reader.GetData();
        uint32_t nElements = dmData.Width * dmData.Height;

        if (mode == "E")
        {
            uint8_t* encoded = new uint8_t[nElements * 3];
            Encode(depthData, (Color*)encoded, nElements, algorithm);

            if (outputFormat == "JPG") 
                ImageWriter::WriteJPEG(outDir + file.filename().string() + ".jpg", encoded, dmData.Width, dmData.Height, jpeg);
            else if (outputFormat == "PNG") 
                ImageWriter::WritePNG(outDir + file.filename().string() + ".png", encoded, dmData.Width, dmData.Height);
            else if (outputFormat == "WEBP")
                ImageWriter::WriteWEBP(outDir + file.filename().string() + ".webp", encoded, dmData.Width, dmData.Height);
            else if (outputFormat == "LOSSY_WEBP")
                ImageWriter::WriteWEBP(outDir + file.filename().string() + ".jpg", encoded, dmData.Width, dmData.Height, jpeg);
            else if (outputFormat == "SPLIT_WEBP")
                ImageWriter::WriteSplitWEBP(outDir + file.filename().string() + ".jpg", encoded, dmData.Width, dmData.Height, jpeg);

            delete[] encoded;
        }
        else
        {

            Decode(file, outDir, algorithm);
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

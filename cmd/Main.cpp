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

/*
    - Parameter to enlarge
    - Parameter to use tables
    - TODO: quantization parameter optional, extracted from file if not specified
*/

using namespace DStream;

#ifndef _WIN32
#include <unistd.h>
#else

int opterr = 1, optind = 1, optopt, optreset;
const char* optarg;
int getopt(int nargc, char* const nargv[], const char* ostr);
#endif


void Usage()
{
    std::cerr <<
        R"use(Usage: dstream-cmd [OPTIONS] <DIRECTORY>

    DIRECTORY is the path to the folder containing the depth data
      -d <output>: output folder in which final data will be saved
      -r <recursive>: navigate the input directory recursively and process all the files contained in it
      -a <algorithm>: algorithm to be used (algorithm names: PACKED, TRIANGLE, MORTON, HILBERT, PHASE, SPLIT, HUE)
      -q <quantization>: quantization level
      -b <bits>: number of bits dedicated to the algorithm (only used by Hilbert and Packed). The remaining ones will be used to enlarge / shrink data
      -j <quality>: JPEG quality to use if encoding
      -m <mode>: program mode, E for encoding, D for decoding
      -?: display this message
      -h: display this message

    )use";
}

int ParseOptions(int argc, char** argv, std::string& inDir, std::string& outDir, std::string& algo, uint8_t& quantization, uint8_t& jpeg, 
    uint8_t& algoBits,  bool& recursive, std::string& mode)
{
    int c;
    recursive = false;

    while ((c = getopt(argc, argv, "d:a:q:j::b:m:rh::")) != -1) {
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
        case 'm':
            mode = optarg;
            if (mode != "D" && mode != "E")
            {
                std::cerr << "Unknown program mode " << mode << std::endl;
                Usage();
            }
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

int ValidateInput(const std::string& algorithm, uint8_t quantization, uint8_t jpeg, uint8_t algoBits, const std::string& mode)
{
    if (mode == "D" && jpeg <= 100)
        std::cout << "JPEG quality specified, but DECODING mode is set. The quality parameter will be ignored." << std::endl;
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

void Encode(const std::filesystem::path& filePath, const std::string& outputDirectory, const std::string& coder, uint8_t quantization, uint8_t jpeg, uint8_t algoBits)
{
    std::string extension = filePath.extension().string();
    DepthmapData dmData;
    DepthmapReader reader;
    uint16_t* depthData;
    uint8_t* encoded;
    uint32_t nElement;

    bool enlarge = true, useTables = true;
    uint32_t nElements = 0;

    if (extension == ".asc")
        reader = DepthmapReader(filePath.string(), DepthmapFormat::ASC, dmData, true);
    else if (extension == ".tif" || extension == ".tiff")
        reader = DepthmapReader(filePath.string(), DepthmapFormat::TIF, dmData, true);

    nElements = dmData.Width * dmData.Height;
    encoded = new uint8_t[nElements * 3];

    // Read depthmap here
    if (!coder.compare("Packed"))
    {
        StreamCoder<Packed> coder(quantization, enlarge, 8, useTables);
        coder.Encode(depthData, (Color*)encoded, nElements);
    }
    else if (!coder.compare("Hue"))
    {
        StreamCoder<Hue> coder(quantization, enlarge, 8, useTables);
        coder.Encode(depthData, (Color*)encoded, nElements);
    }
    else if (!coder.compare("Hilbert"))
    {
        StreamCoder<Hilbert> coder(quantization, enlarge, 3, useTables);
        coder.Encode(depthData, (Color*)encoded, nElements);
    }
    else if (!coder.compare("Morton"))
    {
        StreamCoder<Morton> coder(quantization, enlarge, 8, useTables);
        coder.Encode(depthData, (Color*)encoded, nElements);
    }
    else if (!coder.compare("Split"))
    {
        StreamCoder<Split> coder(quantization, enlarge, 8, useTables);
        coder.Encode(depthData, (Color*)encoded, nElements);
    }
    else if (!coder.compare("Phase"))
    {
        StreamCoder<Phase> coder(quantization, enlarge, 8, useTables);
        coder.Encode(depthData, (Color*)encoded, nElements);
    }
    else
    {
        StreamCoder<Triangle> coder(quantization, enlarge, 8, useTables);
        coder.Encode(depthData, (Color*)encoded, nElements);
    }

    delete[] depthData;
    delete[] encoded;
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
    bool saveDecoded, recursive;
    uint8_t quantization, jpeg, algoBits;
    std::string inDir, outDir, algorithm, mode;

    if (ParseOptions(argc, argv, inDir, outDir, algorithm, quantization, jpeg, algoBits, recursive, mode) != 0)
        return -1;
    if (ValidateInput(algorithm, quantization, jpeg, algoBits, mode) != 0)
        return -2;

    std::filesystem::path inputDir = std::filesystem::path(inDir);
    std::vector<std::filesystem::path> files = GetFiles(inputDir, recursive);

    for (auto file : files)
    {
        if (mode == "E")
            Encode(file, outDir, algorithm, quantization, jpeg, algoBits);
        else
            Decode(file, outDir, algorithm, quantization, 100, algoBits);
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

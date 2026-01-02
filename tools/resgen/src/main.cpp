//
// Created by Gxin on 2022/12/15.
//

#define USE_GANY_CORE
#include <gx/gany.h>

#include <gx/gfile.h>
#include <gx/gtime.h>

#include <getopt/getopt.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory.h>


static const char *sJsonMagicString = "RESINFO";

static const char *sPackageName = "resources";

static const char *sDeployDir = ".";

static bool sKeepExtension = false;

static bool sAppendNull = false;

static bool sGenerateC = false;

static bool sQuietMode = false;

static bool sEmbedJson = false;

static const char *USAGE = R"TXT(Usage:
RESGEN [options] [input_file_0] [input_file_1] ...

RESGEN aggregates a sequence of binary blobs, each of which becomes a "resource" whose id
is the basename of the input file. It produces the following set of files:

    resources.h ......... declares sizes and offsets for each resource
    resources.c ......... binary data for the resource

Options:
   --help, -h
       Print this message
   --package=string, -p string
       Name of the resource package (defaults to "resources")
       This is used to generate filenames and symbol prefixes
   --deploy=dir, -x dir (defaults to ".")
       Generate everything needed for deployment into [dir]
   --keep, -k
       Keep file extensions when generating symbols
   --text, -t
       Append a null terminator to each data blob
   --cfile, -c
       Generate xxd-style C file
   --json, -j
       Embed a JSON string in the output that provides a summary
       of all resource sizes and names. Useful for size analysis.
    --quiet, -q
        Suppress console output
)TXT";

static void printUsage(const char *name)
{
    std::string execName(GFile(name).fileName());
    const std::string from("RESGEN");
    std::string usage(USAGE);
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    puts(usage.c_str());
}

static int handleArguments(int argc, char *argv[])
{
    constexpr static const char *OPTSTR = "hp:x:ktcqj";
    static const struct option OPTIONS[] = {
            {"help",    no_argument,       nullptr, 'h'},
            {"package", required_argument, nullptr, 'p'},
            {"deploy",  required_argument, nullptr, 'x'},
            {"keep",    no_argument,       nullptr, 'k'},
            {"text",    no_argument,       nullptr, 't'},
            {"cfile",   no_argument,       nullptr, 'c'},
            {"quiet",   no_argument,       nullptr, 'q'},
            {"json",    no_argument,       nullptr, 'j'},
            {nullptr, 0,                   nullptr, 0}  // termination of the option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = gp_getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'p':
                sPackageName = optarg;
                break;
            case 'x':
                sDeployDir = optarg;
                break;
            case 'k':
                sKeepExtension = true;
                break;
            case 't':
                sAppendNull = true;
                break;
            case 'c':
                sGenerateC = true;
                break;
            case 'q':
                sQuietMode = true;
                break;
            case 'j':
                sEmbedJson = true;
                break;
        }
    }

    return optind;
}

int main(int argc, char *argv[])
{
    initGAnyCore();

    const int optionIndex = handleArguments(argc, argv);
    const int numArgs = argc - optionIndex;
    if (numArgs < 1) {
        printUsage(argv[0]);
        return 1;
    }

    std::vector<GFile> inputPaths;
    for (int argIndex = optionIndex; argIndex < argc; ++argIndex) {
        inputPaths.emplace_back(argv[argIndex]);
    }
    if (sEmbedJson) {
        inputPaths.emplace_back(sJsonMagicString);
    }

    std::string packageFile = sPackageName;
    std::string packagePrefix = std::string(sPackageName) + "_";
    transform(packageFile.begin(), packageFile.end(), packageFile.begin(), ::tolower);
    transform(packagePrefix.begin(), packagePrefix.end(), packagePrefix.begin(), ::toupper);
    std::string package = packagePrefix + "PACKAGE";

    GFile deployDir(sDeployDir);
    if (!deployDir.exists()) {
        deployDir.mkdirs();
    }

    const GFile headerPath(deployDir, (packageFile + ".h"));
    const GFile xxdPath(deployDir, (packageFile + ".c"));

    // Open the header file stream for writing.
    std::ostringstream headerStream;
    headerStream << "#ifndef " << packagePrefix << "H" << std::endl
                 << "#define " << packagePrefix << "H" << std::endl << std::endl
                 << "#include <stdint.h>" << std::endl << std::endl
                 << "extern \"C\" {" << std::endl
                 << "    extern const uint8_t " << package << "[];" << std::endl;

    std::ostringstream headerMacros;
    std::ostringstream xxdDefinitions;
    std::ostringstream jsonStream;

    // Open the generated C file for writing.
    std::ofstream xxdStream;
    if (sGenerateC) {
        xxdStream = std::ofstream(xxdPath.filePath());
        if (!xxdStream) {
            std::cerr << "Unable to open " << xxdPath.filePath() << std::endl;
            exit(1);
        }
        xxdStream << "#include <stdint.h>\n\nconst uint8_t " << package << "[] = {\n";
    }

    jsonStream << "{";
    size_t offset = 0;
    for (const auto &inPath: inputPaths) {
        std::vector<uint8_t> content;
        if (inPath.filePath() != sJsonMagicString) {
            std::ifstream inStream(inPath.filePath(), std::ios::binary);
            if (!inStream) {
                std::cerr << "Unable to open " << inPath.filePath() << std::endl;
                exit(1);
            }
            content = std::vector<uint8_t>((std::istreambuf_iterator<char>(inStream)), {});
        } else {
            std::string jsonString = jsonStream.str();
            jsonString[jsonString.size() - 1] = '}';
            std::ostringstream jsonBlob;
            jsonBlob << sJsonMagicString << "\0";
            jsonBlob << jsonString.size() << "\0";
            jsonBlob << jsonString;
            jsonString = jsonBlob.str();
            const uint8_t *jsonPtr = (const uint8_t  *) jsonString.c_str();
            content = std::vector<uint8_t>(jsonPtr, jsonPtr + jsonBlob.str().size());
        }
        if (sAppendNull) {
            content.push_back(0);
        }

        std::string rName = sKeepExtension ? inPath.fileName() : inPath.fileNameWithoutExtension();
        std::replace(rName.begin(), rName.end(), '.', '_');
        std::replace(rName.begin(), rName.end(), '-', '_');
        std::replace(rName.begin(), rName.end(), ' ', '_');
        transform(rName.begin(), rName.end(), rName.begin(), ::toupper);
        const std::string prname = packagePrefix + rName;


        headerMacros
                << "#define " << prname << "_DATA (" << package << " + " << prname << "_OFFSET)\n";

        headerStream
                << "    extern uint32_t " << prname << "_OFFSET;\n"
                << "    extern uint32_t " << prname << "_SIZE;\n";

        // Write the xxd-style ASCII array, followed by a blank line.
        if (sGenerateC) {
            xxdDefinitions
                    << "uint32_t " << prname << "_OFFSET = " << offset << ";\n"
                    << "uint32_t " << prname << "_SIZE = " << content.size() << ";\n";

            xxdStream << "// " << rName << "\n";
            xxdStream << std::setfill('0') << std::hex;
            size_t i = 0;
            for (; i < content.size(); i++) {
                if (i > 0 && i % 20 == 0) {
                    xxdStream << "\n";
                }
                xxdStream << "0x" << std::setw(2) << (int) content[i] << ", ";
            }
            if (i % 20 != 0) { xxdStream << "\n"; }
            xxdStream << "\n";
        }

        jsonStream << "\"" << rName << "\":" << content.size() << ",";
        offset += content.size();
    }

    headerStream << "}\n" << headerMacros.str();
    headerStream << "\n#endif\n";

    // To optimize builds, avoid overwriting the header file if nothing has changed.
    bool headerIsDirty = true;
    std::ifstream headerInStream(headerPath.filePath(), std::ifstream::ate);
    std::string headerContents = headerStream.str();
    if (headerInStream) {
        long fileSize = static_cast<long>(headerInStream.tellg());
        if (fileSize == headerContents.size()) {
            std::vector<char> previous(fileSize);
            headerInStream.seekg(0);
            headerInStream.read(previous.data(), fileSize);
            headerIsDirty = 0 != memcmp(previous.data(), headerContents.c_str(), fileSize);
        }
    }

    if (headerIsDirty) {
        std::ofstream headerOutStream(headerPath.filePath());
        if (!headerOutStream) {
            std::cerr << "Unable to open " << headerPath.filePath() << std::endl;
            exit(1);
        }
        headerOutStream << headerContents;
    }

    if (!sQuietMode) {
        std::cout << "Generated files: "
                  << headerPath.filePath();
    }

    if (sGenerateC) {
        xxdStream << "};\n\n" << xxdDefinitions.str();
        if (!sQuietMode) {
            std::cout << " " << xxdPath.filePath();
        }
    }

    if (!sQuietMode) {
        std::cout << std::endl;
    }
}

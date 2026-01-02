//
// Created by Gxin on 24-4-19.
//

#define USE_GANY_CORE
#include <gx/gany.h>

#include <gx/zipfile.h>

#include <gx/gstring.h>

#include <getopt/getopt.h>


static std::string sOutput = "";

static std::string sRootDir = "";

static std::string sPassword = "";

static int32_t sCompressLevel = -1;

static bool sRecursePaths = false;
static bool sJunkPaths = false;

static bool sQuietMode = false;

static const char *USAGE = R"TXT(Usage:
APP_NAME [options] [input_file_0] [input_file_1] ...
APP_NAME [options] [input_dir_0] [input_file_1] ...

Package the specified file directory as a zip file.

Options:
    --help, -h
        Print this message
    --output=string, -o string
        Specify the output file path
        Ex: ./out.zip
    --recurse-paths, -r
        Recursive compression of directories
    --root-dir=string, -d string
        Specify the root directory name of the zip package.
        If this option is included, the compressed package will contain the root directory structure
    --junk-paths, -j
        Do not retain directory information for compressed files, only retain file names
    --password=string, -P string
        Encrypt the entries in the zip file with the specified password
        Note that this operation is not secure as the password may be detected by other users through viewing historical commands
    --compress-level=6, -l 6
        Compress levels, -1 to 9
        -1 indicates the use of the most effective compression level
         0 indicates no compression
         9 indicates the highest compression rate
    --quiet, -q
        Suppress console output
)TXT";

static void printUsage(const char *name)
{
    std::string execName(GFile(name).fileName());
    const std::string from("APP_NAME");
    std::string usage(USAGE);
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    puts(usage.c_str());
}

static int handleArguments(int argc, char *argv[])
{
    constexpr static const char *OPTSTR = "hrjo:d:P:l:q";
    static const struct option OPTIONS[] = {
            {"help",           no_argument,       nullptr, 'h'},
            {"recurse-paths",  no_argument,       nullptr, 'r'},
            {"junk-paths",     no_argument,       nullptr, 'j'},
            {"output",         required_argument, nullptr, 'o'},
            {"root-dir",       required_argument, nullptr, 'd'},
            {"password",       required_argument, nullptr, 'P'},
            {"compress-level", required_argument, nullptr, 'l'},
            {"quiet",          no_argument,       nullptr, 'q'},
            {nullptr, 0,                          nullptr, 0}
    };

    int opt;
    int optionIndex = 0;

    while ((opt = gp_getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h': {
                printUsage(argv[0]);
                exit(0);
            }
            case 'o': {
                sOutput = arg;
            }
                break;
            case 'r': {
                sRecursePaths = true;
            }
                break;
            case 'd': {
                sRootDir = arg;
            }
                break;
            case 'j': {
                sJunkPaths = true;
            }
                break;
            case 'P': {
                sPassword = arg;
            }
                break;
            case 'l': {
                sCompressLevel = std::stoi(arg);
            }
                break;
            case 'q': {
                sQuietMode = true;
            }
                break;

        }
    }

    return optind;
}

static void writeFileToZip(GxZipWriter &writer, GFile &file, const GString &rootDir)
{
    if (file.isFile()) {
        if (file.open(GFile::ReadOnly | GFile::Binary)) {
            GByteArray data = file.read();

            GString fileName = rootDir + (sJunkPaths ? file.fileName() : file.filePath());

            if (writer.writeFile(fileName.toStdString(), data, sCompressLevel)) {
                if (!sQuietMode) {
                    std::cout << "adding: " << file.filePath() << std::endl;
                }
            } else {
                std::cerr << "error: write file failed: " << file.filePath() << std::endl;
            }

            file.close();
        } else {
            std::cerr << "error: failed to open file: " << file.filePath() << std::endl;
        }
    } else if (file.isDirectory()) {
        auto files = file.listFiles([](const GFile &f) -> bool {
            if (!sRecursePaths) {
                return f.isFile();
            }
            return true;
        });
        for (auto &fi: files) {
            writeFileToZip(writer, fi, rootDir);
        }
    } else {
        std::cerr << "error: Unable to compress non file objects: " << file.filePath() << std::endl;
    }
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

    if (sOutput.empty()) {
        sOutput = "./out.zip";
    }

    GFile outputPath(sOutput);
    if (outputPath.exists() && outputPath.isDirectory()) {
        std::cerr << "error: the output target is not a valid file path: " << sOutput << std::endl;
        return EXIT_FAILURE;
    }

    GxZipWriter writer;
    if (!writer.open(outputPath, sPassword)) {
        std::cerr << "error: failed to open or create zip file" << std::endl;
        return EXIT_FAILURE;
    }

    GString rootDir = GString(sRootDir).replace("\\", "/");
    if (!rootDir.isEmpty() && !rootDir.endWith("/")) {
        rootDir.append("/");
    }

    for (auto &fi: inputPaths) {
        writeFileToZip(writer, fi, rootDir);
    }

    writer.close();
    return EXIT_SUCCESS;
}
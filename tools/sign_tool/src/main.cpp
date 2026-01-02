//
// Created by Gxin on 2022/10/27.
//

#define USE_GANY_CORE
#include <gx/gany.h>

#include <gx/gcrypto.h>
#include <gx/gbytearray.h>
#include <gx/ghashsum.h>
#include <gx/gfile.h>
#include <gx/gtime.h>

#include <getopt/getopt.h>
#include <cstdio>
#include <iostream>
#include <sstream>

#define LICENCE_SIZE 104
#define SIGN_SIZE 96

void printUsage(const char *program)
{
    fprintf(stdout, R"TXT(Usage:
%s [options]

Tools for signature signature and verification of files

1. Signature:
%s -s -k [private key of the license owner] -l [license content] -f [files to be signed]

2. Check signature:
%s -c -k [public key of the licensing authority] -f [files that need to be checked for signatures]

Options:
    --key=base64_string, -k base64_string
        Fill in your own security key when signing;
        Fill in the public key of the signing authority when verifying the signature
    --licence=base64_string, -l base64_string
        Fill in only when signing, License requested using public key
    --sign, -s
        Signature Mode
    --check, -c
        Check signature mode
    --file=string, -f string
        File to be signed
)TXT",
            program,
            program,
            program
    );
}

GByteArray sha256sum(GFile &file, int64_t maxLen = -1)
{
    GByteArray sha256;
    if (file.open(GFile::ReadOnly | GFile::Binary)) {
        char buffer[4096];
        int64_t len;
        int64_t count = 0;

        auto hashSum = GHashSum::hashSum(GHashSum::Sha256);
        while ((len = file.read(buffer, 4096)) > 0) {
            if (maxLen > 0 && count + len >= maxLen) {
                len = maxLen - count;
                if (len <= 0) {
                    break;
                }
            }
            count += len;

            hashSum->update((uint8_t *) buffer, len);
        }
        sha256 = hashSum->final();

        file.close();
    }

    return sha256;
}

bool doSign(const GByteArray &keyBuff, const std::string &licence, GFile &file)
{
    if (keyBuff.size() != GCrypto::SIGN_SECURE_KEY_LEN) {
        std::cerr << "The key length is incorrect. It must be 64 bytes!!" << std::endl;
        return false;
    }

    GByteArray licBuff = GByteArray::base64Decode(licence);
    if (licBuff.size() != LICENCE_SIZE) {
        std::cerr << "Invalid licence." << std::endl;
        return false;
    }

    std::stringstream info;

    info << "Start signing, file: " << file.filePath() << ", size: " << file.fileSize() << std::endl;

    GByteArray sha256 = sha256sum(file);
    if (sha256.isEmpty()) {
        std::cerr << "Failed to calculate file hash value!!" << std::endl;
        return false;
    }

    GByteArray sign = GCrypto::sign(sha256, keyBuff);

    info << "Signature hash: " << sha256.toHexString() << "\ninformation: " << sign.toHexString() << std::endl;

    if (file.open(GFile::ReadWrite | GFile::Binary | GFile::Append)) {
        file.write(licBuff);
        file.write(sign);
        file.close();
        info << "Signing completed." << std::endl;
    } else {
        std::cerr << "Signing failed, unable to write file!!" << std::endl;
        return false;
    }

    std::cout << info.str();

    return true;
}

bool checkSign(const GByteArray &keyBuff, GFile &file)
{
    if (keyBuff.size() != GCrypto::SIGN_PUBLIC_KEY_LEN) {
        std::cerr << "The key length is incorrect. It must be 32 bytes!!" << std::endl;
        return false;
    }

    std::stringstream info;

    info << "Start check signature, file: " << file.filePath() << ", size: " << file.fileSize() << std::endl;
    if (file.fileSize() < LICENCE_SIZE + SIGN_SIZE) {
        std::cerr << "Failed to check signature. The file is not signed!!" << std::endl;
        return false;
    }
    GByteArray sha256 = sha256sum(file, file.fileSize() - (LICENCE_SIZE + SIGN_SIZE));
    if (sha256.isEmpty()) {
        std::cerr << "Failed to calculate file hash value!!" << std::endl;
        return false;
    }

    if (file.open(GFile::ReadOnly | GFile::Binary)) {
        file.seek(file.fileSize() - (LICENCE_SIZE + SIGN_SIZE), SEEK_SET);
        GByteArray licBuff = file.read(LICENCE_SIZE);
        if (licBuff.size() != LICENCE_SIZE) {
            std::cerr << "Failed to obtain licence!!" << std::endl;
            return false;
        }

        GByteArray sign = file.read(SIGN_SIZE);
        if (sign.size() != SIGN_SIZE) {
            std::cerr << "Failed to obtain signature!!" << std::endl;
            return false;
        }

        GByteArray licOpen = GCrypto::signOpen(licBuff, keyBuff);
        if (licOpen.isEmpty()) {
            std::cerr << "License not found!!" << std::endl;
            return false;
        }
        int64_t term;
        std::vector<uint8_t> signKeyTemp(GCrypto::SIGN_PUBLIC_KEY_LEN);
        licOpen.read(term);
        licOpen.read(signKeyTemp.data(), signKeyTemp.size());
        GByteArray signKey(signKeyTemp);

        GTime now = GTime::currentSystemTime();
        if (term <= now.second()) {
            std::cerr << "License expired!!" << std::endl;
            return false;
        }

        GByteArray signOpen = GCrypto::signOpen(sign, signKey);

        info << " > File fingerprint info: " << sha256.toHexString() << std::endl;
        info << " > File signature info: " << signOpen.toHexString() << std::endl;

        if (sha256 == signOpen) {
            info << "Signature matching." << std::endl;
        } else {
            std::cerr << "Signature mismatch!!" << std::endl;
            return false;
        }
    } else {
        std::cerr << "Could not open file!!" << std::endl;
        return false;
    }

    std::cout << info.str();

    return true;
}

int main(int argc, char *argv[])
{
    initGAnyCore();

    constexpr const char *OPT_STR = "hsck:l:f:";

    const static option OPTIONS[] = {
            {"help",    no_argument,       nullptr, 'h'},
            {"key",     required_argument, nullptr, 'k'},
            {"licence", required_argument, nullptr, 'l'},
            {"sign",    no_argument,       nullptr, 's'},
            {"check",   no_argument,       nullptr, 'c'},
            {"file",    required_argument, nullptr, 'f'},
            {nullptr, 0,                   nullptr, 0}
    };

    int opt;
    int optIndex;

    std::string key;
    std::string licence;
    int32_t mode = 0;
    std::string file;

    while ((opt = gp_getopt_long(argc, argv, OPT_STR, OPTIONS, &optIndex)) != -1) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(EXIT_SUCCESS);
            case 'k':
                key = arg;
                break;
            case 'l':
                licence = arg;
                break;
            case 's': {
                if (mode != 0) {
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                mode = 1;
            }
                break;
            case 'c': {
                if (mode != 0) {
                    printUsage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                mode = 2;
            }
                break;
            case 'f':
                file = arg;
                break;
        }
    }

    if (key.empty() || mode == 0 || file.empty()) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    GFile f(file);
    if (!f.exists()) {
        std::cerr << "No such file." << std::endl;
        return EXIT_FAILURE;
    }

    GByteArray keyBuff = GByteArray::base64Decode(key);
    if (keyBuff.isEmpty()) {
        std::cerr << "Invalid key." << std::endl;
        return EXIT_FAILURE;
    }


    if (mode == 1) {
        if (!doSign(keyBuff, licence, f)) {
            return EXIT_FAILURE;
        }
    } else {
        if (!checkSign(keyBuff, f)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
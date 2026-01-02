//
// Created by Gxin on 2022/10/18.
//

#define USE_GANY_CORE
#include <gx/gany.h>

#include <gx/gcrypto.h>
#include <gx/gtime.h>

#include <getopt/getopt.h>

#include <iostream>
#include <sstream>


#define LICENCE_SIZE 104

void printUsage(const char *program)
{
    fprintf(stdout, R"TXT(Usage:
%s [options]

Generate crypto key and generating licenses

Options:
    --sign=string, -s string
        Generate sign key
    --crypto=string, -c string
        Generate crypto box key
    --hex, -x
        (Optional) Output hexadecimal representation
    --base64, -b
        (Optional) Output base64 representation
    --licence, -l
        Generate License
    --licinfo, -i
        Show License info, param: base64 format license.
    --keym=string, -m string
        Valid when generating license, Secure key of the authority
    --keyn=string, -n string
        Valid when generating license, Public key of the demander
    --term=int64/string, -t int64/string,
        Valid when generating license, Term of validity (Unix timestamp in seconds)
            )TXT",
            program);
}

int main(int argc, char *argv[])
{
    initGAnyCore();

    constexpr const char *OPT_STR = "hscxbli:m:n:t:";

    const static option OPTIONS[] = {
            {"help",       no_argument,       nullptr, 'h'},
            {"sign",       no_argument,       nullptr, 's'},
            {"crypto",     no_argument,       nullptr, 'c'},
            {"hex",        no_argument,       nullptr, 'x'},
            {"base64",     no_argument,       nullptr, 'b'},
            {"licence",    no_argument,       nullptr, 'l'},
            {"licinfo", required_argument, nullptr, 'i'},
            {"keym",       required_argument, nullptr, 'm'},
            {"keyn",       required_argument, nullptr, 'n'},
            {"term",       required_argument, nullptr, 't'},
            {nullptr, 0,                      nullptr, 0}
    };

    int opt;
    int optIndex;

    int type = 0;
    int outType = 0;
    std::string key_m;
    std::string key_n;
    std::string inLic;
    int64_t term = 0;

    while ((opt = gp_getopt_long(argc, argv, OPT_STR, OPTIONS, &optIndex)) != -1) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(EXIT_SUCCESS);
            case 's':
                type |= 0x01;
                break;
            case 'c':
                type |= 0x02;
                break;
            case 'x':
                outType |= 0x01;
                break;
            case 'b':
                outType |= 0x02;
                break;
            case 'l':
                type = 0x04;
                break;
            case 'i':
                type = 0x08;
                inLic = arg;
                break;
            case 'm':
                key_m = arg;
                break;
            case 'n':
                key_n = arg;
                break;
            case 't': {
                term = std::stoll(arg);
            }
                break;
        }
    }

    if (type <= 0) {
        printUsage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (outType == 0) {
        outType = 0x01 | 0x02;
    }

    std::stringstream ss;

    if (type == 0x04) {
        if (key_n.empty() || key_m.empty() || term <= 0) {
            std::cerr
                    << "Correct usage: \ncrypto-keygen -l -m [secure key of the authority] -n [public key of the demander] -t [term of validity (Unix timestamp in seconds)]"
                    << std::endl;
            return EXIT_FAILURE;
        }

        GByteArray keyNBytes = GByteArray::base64Decode(key_n);
        if (keyNBytes.size() != GCrypto::SIGN_PUBLIC_KEY_LEN) {
            std::cerr << "KeyN invalid!" << std::endl;
            return EXIT_FAILURE;
        }

        GByteArray keyMBytes = GByteArray::base64Decode(key_m);
        if (keyMBytes.size() != GCrypto::SIGN_SECURE_KEY_LEN) {
            std::cerr << "KeyM invalid!" << std::endl;
            return EXIT_FAILURE;
        }

        GTime now = GTime::currentSystemTime();
        if (term <= now.second()) {
            std::cerr << "The set term of time has expired!" << std::endl;
            return EXIT_FAILURE;
        }

        GByteArray licOriBuff;

        licOriBuff.write(term);
        licOriBuff.write(keyNBytes.data(), keyNBytes.size());

        GByteArray lic = GCrypto::sign(licOriBuff, keyMBytes);

        ss << "Licence:\n";
        if (outType & 0x01) {
            ss << lic.toHexString() << "\n";
        }
        if (outType & 0x02) {
            ss << GByteArray::base64Encode(lic) << "\n";
        }

        ss << std::endl;
    }
    if (type == 0x08) {
        GByteArray licBytes = GByteArray::base64Decode(inLic);
        if (licBytes.size() != LICENCE_SIZE) {
            std::cerr << "Invalid licence." << std::endl;
            std::cerr << "Correct usage: \ncrypto-keygen -i [licence]" << std::endl;
            return EXIT_FAILURE;
        }

        licBytes.seekReadPos(SEEK_SET, LICENCE_SIZE - 40);

        int64_t inTerm;
        std::vector<uint8_t> signKeyTemp(GCrypto::SIGN_PUBLIC_KEY_LEN);
        licBytes.read(inTerm);
        licBytes.read(signKeyTemp.data(), signKeyTemp.size());
        GByteArray signKey(signKeyTemp);

        GTime termTime(GTime::SystemClock, 0);
        termTime.addSecs(inTerm);

        ss << "Licence info:\n";
        ss << "Key: " << GByteArray::base64Encode(signKey) << "\n";
        ss << "Term: " << inTerm << "(" << termTime.toString("yyyy-MM-dd HH:mm:ss") << ").";
        ss << std::endl;
    } else {
        if (type & 0x01) {
            auto key = GCrypto::signKeyPair();

            ss << "Sign key:\n";
            if (outType & 0x01) {
                ss << "Public key hex: " << key.publicKey.toHexString() << "\n"
                   << "Secure key: hex" << key.secureKey.toHexString() << "\n";
            }
            if (outType & 0x02) {
                ss << "Public key base64: " << GByteArray::base64Encode(key.publicKey) << "\n"
                   << "Secure key base64: " << GByteArray::base64Encode(key.secureKey) << "\n";
            }
            ss << std::endl;
        }
        if (type & 0x02) {
            auto key = GCrypto::boxKeyPair();

            ss << "Crypto key:\n";
            if (outType & 0x01) {
                ss << "Public key hex: " << key.publicKey.toHexString() << "\n"
                   << "Secure key hex: " << key.secureKey.toHexString() << "\n";
            }
            if (outType & 0x02) {
                ss << "Public key base64: " << GByteArray::base64Encode(key.publicKey) << "\n"
                   << "Secure key base64: " << GByteArray::base64Encode(key.secureKey) << "\n";
            }
            ss << std::endl;
        }
    }

    std::cout << ss.str();

    return EXIT_SUCCESS;
}
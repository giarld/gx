//
// Created by Gxin on 2022/10/18.
//

#define USE_GANY_CORE
#include <gx/gany.h>

#include <gx/gcrypto.h>
#include <gx/debug.h>


bool testSign()
{
    GCrypto::SignKey key = GCrypto::signKeyPair();

    std::string str = "Hello Gx.";
    GByteArray data;
    data << str;

    GByteArray enData = GCrypto::sign(data, key.secureKey);
    GByteArray deData = GCrypto::signOpen(enData, key.publicKey);

    return data == deData;
}

bool testCryptoBox()
{
    GCrypto::CryptoKey keyA = GCrypto::boxKeyPair();
    GCrypto::CryptoKey keyB = GCrypto::boxKeyPair();

    GByteArray nonce = GCrypto::randomBytes(GCrypto::NONCE_BYTES);

    std::string str = "Hello Gx.";
    GByteArray data;
    data << str;

    GByteArray enData = GCrypto::box(data, nonce, keyB.publicKey, keyA.secureKey);
    GByteArray deData = GCrypto::boxOpen(enData, nonce, keyA.publicKey, keyB.secureKey);

    return data == deData;
}

bool testCryptoBoxBeforeAfter()
{
    GCrypto::CryptoKey keyA = GCrypto::boxKeyPair();
    GCrypto::CryptoKey keyB = GCrypto::boxKeyPair();

    GByteArray nonce = GCrypto::randomBytes(GCrypto::NONCE_BYTES);

    std::string str = "Hello Gx.";
    GByteArray data;
    data << str;

    GCrypto::CryptoShareKey shareKeyA = GCrypto::boxBefore(keyB.publicKey, keyA.secureKey);
    GByteArray enData = GCrypto::boxAfter(data, nonce, shareKeyA);

    GCrypto::CryptoShareKey shareKeyB = GCrypto::boxBefore(keyA.publicKey, keyB.secureKey);
    GByteArray deData = GCrypto::boxOpenAfter(enData, nonce, shareKeyB);

    return data == deData;
}

int main(int argc, char *argv[])
{
    initGAnyCore();

    Log("Test sign: {}", testSign() ? "OK" : "FAIL");
    Log("Test crypto box: {}", testCryptoBox() ? "OK" : "FAIL");
    Log("Test crypto box before-after: {}", testCryptoBoxBeforeAfter() ? "OK" : "FAIL");

    return EXIT_SUCCESS;
}
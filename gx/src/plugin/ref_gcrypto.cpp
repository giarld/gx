//
// Created by Gxin on 2022/10/18.
//

#include <gx/gany.h>

#include <gx/gcrypto.h>


using namespace gany;

void refGCrypto()
{
    Class<GCrypto>("Gx", "GCrypto", "Crypto algorithm based on ECC.")
            .constant("NONCE_BYTES", GCrypto::NONCE_BYTES)
            .constant("SIGN_SECURE_KEY_LEN", GCrypto::SIGN_SECURE_KEY_LEN)
            .constant("SIGN_PUBLIC_KEY_LEN", GCrypto::SIGN_PUBLIC_KEY_LEN)
            .constant("CRYPTO_SECURE_KEY_LEN", GCrypto::CRYPTO_SECURE_KEY_LEN)
            .constant("CRYPTO_SECURE_KEY_LEN", GCrypto::CRYPTO_PUBLIC_KEY_LEN)
            .staticFunc("randomBytes", &GCrypto::randomBytes)
            .staticFunc("signKeyPair", &GCrypto::signKeyPair)
            .staticFunc("sign", &GCrypto::sign)
            .staticFunc("signOpen", &GCrypto::signOpen)
            .staticFunc("boxKeyPair", &GCrypto::boxKeyPair)
            .staticFunc("box", &GCrypto::box)
            .staticFunc("boxOpen", &GCrypto::boxOpen)
            .staticFunc("boxBefore", &GCrypto::boxBefore)
            .staticFunc("boxAfter", &GCrypto::boxAfter)
            .staticFunc("boxOpenAfter", &GCrypto::boxOpenAfter);

    Class<GCrypto::SignKey>("Gx", "SignKey", "GCrypto SignKey.")
            .readOnly("publicKey", &GCrypto::SignKey::publicKey)
            .readOnly("secureKey", &GCrypto::SignKey::secureKey);

    Class<GCrypto::CryptoKey>("Gx", "CryptoKey", "GCrypto CryptoKey.")
            .readOnly("publicKey", &GCrypto::CryptoKey::publicKey)
            .readOnly("secureKey", &GCrypto::CryptoKey::secureKey);
}

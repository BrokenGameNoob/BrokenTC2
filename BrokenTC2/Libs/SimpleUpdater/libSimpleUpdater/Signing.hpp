#pragma once

#include "ManifestTools.hpp"

#include "cryptopp/rsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/files.h"
#include "hex.h"

#include <concepts>

#include <QFileInfo>
#include <QDir>
#include <QFile>

#include <QDebug>

namespace{

//decodeEncode<CryptoPP::HexEncoder>("jlsjflesjfkesjl")
//decodeEncode<CryptoPP::HexDecoder>("jlsjflesjfkesjl")
template<typename Converter_t>
std::string decodeEncode(const std::string& input){
    std::string decoded;
    CryptoPP::StringSource(input, true,
                          new Converter_t(
                          new CryptoPP::StringSink(decoded)));
    return decoded;
}

}

namespace signing{

//concepts

template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

template<class T>
concept CryptoMaterial = Derived<T,CryptoPP::CryptoMaterial>;

template<typename T>
concept SavableKey = CryptoMaterial<T> && requires(T key,CryptoPP::ByteQueue queue){
        key.Save(queue);
};

template<typename T>
concept LoadableKey = CryptoMaterial<T> && requires(T key,CryptoPP::ByteQueue queue){
        key.Load(queue);
};

template<typename T>
concept HasConstKey = requires(const T& object){object.GetKey();};

template<typename T>
concept HasKey = requires(const T& object){object.AccessKey();};


//constants

struct RSACheckLevel{//https://www.cryptopp.com/wiki/Keys_and_Formats#Validating_Keys
    using type_t = int32_t;
    static constexpr type_t kNoCrash{0};//rng ignored
    static constexpr type_t kProbablyCorrect{1};//
    static constexpr type_t kBasicSecured{2};//
    static constexpr type_t kExpensive{3};//
};

// Functions

std::optional<bool> checkFileSignatureFromManifest(const QString& fileToCheck,const QString& manifestPath, const QString& verifierPath);

//load a verifier key from a raw file
std::optional<CryptoPP::RSASSA_PKCS1v15_SHA_Verifier> loadVerifier(const QString& inPath);
//load a signer key from a raw file
std::optional<CryptoPP::RSASSA_PKCS1v15_SHA_Signer> loadSigner(const QString& inPath);

//check a signed file from the target file, a verifier and a signature stored in a file
bool checkFile(const QString& inFilename, const CryptoPP::RSASSA_PKCS1v15_SHA_Verifier& verifier, const QString& signatureFile);
//check a signed file from the target file, a verifier and a signature
bool checkFileFromSign(const QString& inFilename, const CryptoPP::RSASSA_PKCS1v15_SHA_Verifier& verifier, const QString& signatureq);

//return the signature of a given file with a given signer
template<typename Signer_t>
QString signFile(const QString& file, const Signer_t& signer);

//store the signature of file with a given signer to an outputfile
template<typename Signer_t>
bool signFileToFile(const std::string& inFilename, const Signer_t& signer,const QString& outFile);


//load a private (signer) key from a raw file
auto/*CryptoPP::RSASSA_PKCS1v15_SHA_Signer*/
loadPrivateKey(const QString& filename,bool enableCheck);
//load a public (verifier) key from a raw file
auto/*CryptoPP::RSASSA_PKCS1v15_SHA_Verifier*/
loadPublicKey(const QString& filename,bool enableCheck);

//save a key (public or private) to a file
template<SavableKey Key_t>
bool saveKey(const Key_t& key,const std::string& path);
//save a public and a private key to a file
template<SavableKey Private_t,SavableKey Public_t>
bool saveKeys(const Private_t& prvKey,const Public_t& pubKey,
              const std::string& prvKeyPath,const std::string& pubKeyPath);


//tell if a key is valid or not given a check level
template<typename T>
bool validateKey(const T& toCheck,RSACheckLevel::type_t checkLevel);


//get a private/public (signer/verifier) key pair
std::tuple<CryptoPP::RSASSA_PKCS1v15_SHA_Signer,CryptoPP::RSASSA_PKCS1v15_SHA_Verifier>
getKeyPair();

// Utils

inline
bool saveStringToFile(const std::string& content,const QString& fileName){
    QFileInfo fInfo{fileName};
    if(!fInfo.dir().mkpath("."))
    {
        qCritical() << "Can't create folder to write file:" << fileName << (fInfo.exists()?"":" (file doesn't exists)");
        return false;
    }
    QFile f{fileName};
    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "Can't open file:" << fileName;
        return false;
    }
    if(f.write(content.c_str(),content.size()) != static_cast<qint64>(content.size()))
    {
        qCritical() << "An error occured when writing to:" << fileName;
        f.close();
        return false;
    }
    f.close();
    return true;
}

inline
std::string readFile(const QString& filename){
    QFile f{filename};
    if(!f.open(QIODevice::ReadOnly))
    {
        qCritical() << "Can't open file:" << filename;
        return {};
    }
    auto result{f.readAll().toStdString()};
    f.close();
    return result;
}


//Generation / writing

inline
std::tuple<CryptoPP::RSASSA_PKCS1v15_SHA_Signer,CryptoPP::RSASSA_PKCS1v15_SHA_Verifier> getKeyPair(){
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::RSASSA_PKCS1v15_SHA_Signer rsaPrivate{};
    rsaPrivate.AccessKey().GenerateRandomWithKeySize(prng, 4096);
    CryptoPP::RSASSA_PKCS1v15_SHA_Verifier rsaPublic(rsaPrivate);
    rsaPrivate.GetKey();

    return {rsaPrivate,rsaPublic};
}

template<CryptoMaterial Key_t>
auto validateKey(const Key_t& toCheck,RSACheckLevel::type_t checkLevel = RSACheckLevel::kBasicSecured){
    CryptoPP::AutoSeededRandomPool prng;
    auto rVal{toCheck.Validate(prng,checkLevel)};
    return rVal;
}

template<HasConstKey T>
auto validateKey(const T& toCheck,RSACheckLevel::type_t checkLevel = RSACheckLevel::kBasicSecured){
    return validateKey(toCheck.GetKey());
}
//ex:
//if(!validateKey(privateKey,RSACheckLevel::kBasicSecured))

template<SavableKey Key_t>
bool saveKey(const Key_t& key,const std::string& path){
    CryptoPP::ByteQueue queue;
    key.Save(queue);

    CryptoPP::FileSink file(path.c_str());
    queue.CopyTo(file);
    return file.MessageEnd();
}
template<HasConstKey KeyContainer_t>
bool saveKey(const KeyContainer_t& key,const std::string& path){
    return saveKey(key.GetKey(),path);
}
template<SavableKey Private_t,SavableKey Public_t>
bool saveKeys(const Private_t& prvKey,const Public_t& pubKey,
              const std::string& prvKeyPath,const std::string& pubKeyPath){
    auto success{saveKey(prvKey,prvKeyPath)};
    qDebug() << "Saving private key success? ->" << success;
    auto success2{saveKey(pubKey,pubKeyPath)};
    qDebug() << "Saving public key success? ->" << success;
//    return success && success2;
    return true;//don't ask any question please
}
template<HasConstKey Private_t,HasConstKey Public_t>
bool saveKeys(const Private_t& prvKey,const Public_t& pubKey,
              const std::string& prvKeyPath,const std::string& pubKeyPath){
    return saveKeys(prvKey.GetKey(),pubKey.GetKey(),prvKeyPath,pubKeyPath);
}

// ------- LOADING

inline
void load(const QString& filename, CryptoPP::BufferedTransformation& bt){
    QFile f{filename};
    if(!f.open(QIODevice::OpenModeFlag::ReadOnly))
    {
        qCritical() << "Can't open file:" << filename;
    }
    auto ba{f.readAll()};
    bt.Put(reinterpret_cast<CryptoPP::byte*>(ba.data()),ba.size(),true);
    bt.MessageEnd();
}

template<LoadableKey Key_t>
void loadKey(const QString& filename, Key_t& key){
    CryptoPP::ByteQueue queue;
    load(filename, queue);

    key.Load(queue);
}
template<HasKey KeyContainer_t>
void loadKey(const QString& filename, KeyContainer_t& key){
    return loadKey(filename,key.AccessKey());
}

template<typename KeyContainer_t,LoadableKey Key_t>
std::optional<KeyContainer_t> u_baseLoadKey(const QString& filename,bool enableCheck){//unit_baseLoadKey
    Key_t key;
    loadKey(filename,key);
    if(enableCheck)
    {
        if(!validateKey(key))
        {
            qCritical().nospace() << "Invalid key read from <" << filename << ">";
            return {};
        }
    }
    KeyContainer_t signer{key};
    return signer;
}

inline
auto loadPrivateKey(const QString& filename,bool enableCheck = true){
    return u_baseLoadKey<CryptoPP::RSASSA_PKCS1v15_SHA_Signer,CryptoPP::RSA::PrivateKey>(filename,enableCheck);
}
inline
auto loadPublicKey(const QString& filename,bool enableCheck = true){
    return u_baseLoadKey<CryptoPP::RSASSA_PKCS1v15_SHA_Verifier,CryptoPP::RSA::PublicKey>(filename,enableCheck);
}

//return the signature
template<typename Signer_t>
std::string signFile(const std::string& filename, const Signer_t& signer){
    CryptoPP::AutoSeededRandomPool prng;
    std::string signature{};
    try
    {
        CryptoPP::FileSource(filename.c_str(),true,new CryptoPP::SignerFilter{prng, signer, new CryptoPP::StringSink{signature}});
        signature = decodeEncode<CryptoPP::HexEncoder>(signature);
        qDebug() << "Signature:" << QString::fromStdString(signature);
    }
    catch(const std::exception& e)
    {
        qCritical() << "Following exception occured when trying to sign file:" << QString::fromStdString(filename) << ":" << e.what();
    }
    catch(...)
    {
        qCritical() << "An unknown exception occured when trying to sign file:" << QString::fromStdString(filename);
    }

    return signature;
}

template<typename Signer_t>
bool signFileToFile(const std::string& inFilename, const Signer_t& signer,const QString& outFile){
    std::string signature{signFile(inFilename,signer)};
    if(signature.empty())
    {
        qCritical() << "Empty signature for file:" << QString::fromStdString(inFilename);
        return false;
    }
    return saveStringToFile(signature,outFile);
}


inline
bool checkFileFromSign(const QString& inFilename, const CryptoPP::RSASSA_PKCS1v15_SHA_Verifier& verifier, std::string signature){
    using byte = unsigned char;
    byte result = 0;
    signature = decodeEncode<CryptoPP::HexDecoder>(signature);
    CryptoPP::SignatureVerificationFilter filter(verifier, new CryptoPP::ArraySink(&result, sizeof(result)));

    // Wrap the data in sources
    CryptoPP::StringSource ss(signature, true);
    // Add the data to the filter
    ss.TransferTo(filter);
//    fs.TransferTo(filter);
    QFile f{inFilename};
    if(!f.open(QIODevice::ReadOnly))
    {
        qCritical() << "Could not check signature on file:" << inFilename << ": Could not open file";
        return false;
    }
    auto ba{f.readAll()};

    //add the file data to the filter
    filter.Put(reinterpret_cast<CryptoPP::byte*>(ba.data()),ba.size(),true);

    // Signal end of data. The filter will
    // verify the signature on the file
    filter.MessageEnd();

    return (result);
}

inline
bool checkFileFromSign(const QString& inFilename, const CryptoPP::RSASSA_PKCS1v15_SHA_Verifier& verifier, const QString& signatureq){
    auto signature{signatureq.toStdString()};
    return checkFileFromSign(inFilename,verifier,signature);
}

inline
bool checkFile(const QString& inFilename, const CryptoPP::RSASSA_PKCS1v15_SHA_Verifier& verifier, const QString& signatureFile){
    auto signature{readFile(signatureFile)};
    if(signature.empty())
    {
        qCritical() << "Cannot read signature file:" << signatureFile << "(abort)";
        return false;
    }
    qDebug() << "Read signature file from:" << signatureFile;
    qDebug() << QString::fromStdString(signature);
    qDebug() << signature.size();

    auto result{checkFileFromSign(inFilename,verifier,signature)};
    return result;
}


//------------------- High level

inline
std::optional<CryptoPP::RSASSA_PKCS1v15_SHA_Signer> loadSigner(const QString& inPath){
    try
    {
        QFileInfo fInfo{inPath};
        if(inPath.isEmpty() || !fInfo.isReadable() || !fInfo.isFile())
        {
            qCritical() << "Please enter a valid input file (not empty, in a readable dir)";
            if(!fInfo.isFile())
            {
                qCritical() << inPath << "is not a file";
            }
            return {};
        }
        auto keyOpt{signing::loadPrivateKey(fInfo.absoluteFilePath(),true)};
        if(!keyOpt)
        {
            qCritical() << "Invalid key or failed to load file" << fInfo.absoluteFilePath();
            return {};
        }
        return keyOpt;
    }
    catch(const std::runtime_error& e)
    {
        qCritical() << "Following error occured when trying to load signer from:" << inPath << ":" << e.what();
        return {};
    }

    qCritical() << "An unknown error occured when trying to load signer from:" << inPath;
    return {};
}

inline
std::optional<CryptoPP::RSASSA_PKCS1v15_SHA_Verifier> loadVerifier(const QString& inPath){
    try
    {
        QFileInfo fInfo{inPath};
        if(inPath.isEmpty() || !fInfo.isReadable() || !fInfo.isFile())
        {
            qCritical() << "Please enter a valid input file (not empty, in a readable dir)";
            if(!fInfo.isFile())
            {
                qCritical() << inPath << "is not a file";
            }
            return {};
        }
        auto keyOpt{signing::loadPublicKey(fInfo.absoluteFilePath(),true)};
        if(!keyOpt)
        {
            qCritical() << "Invalid key or failed to load file" << fInfo.absoluteFilePath();
            return {};
        }
        return keyOpt;
    }
    catch(const std::exception& e)
    {
        qCritical() << "Following error occured when trying to load signer from:" << inPath << ":" << e.what();
        return {};
    }
    catch(...)
    {
    }
    qCritical() << "An unknown error occured when trying to load signer from:" << inPath;
    return {};
}


template<typename Signer_t>
QString signFile(const QString& file, const Signer_t& signer){
    return QString::fromStdString(signFile(file.toStdString(),signer));
}


inline
std::optional<bool> checkFileSignatureFromManifest(const QString& fileToCheck,const QString& manifestPath, const QString& verifierPath){
    if(!QFileInfo::exists(verifierPath))
    {
        qCritical() << "Verifier (public) key file does not exists:" << verifierPath;
        return {};
    }
    auto verifKeyOpt{signing::loadVerifier(verifierPath)};
    if(!verifKeyOpt)
    {
        qCritical() << "Aborting installation as we could not load the specified verifier (public) key file:" << verifierPath;
        return {};
    }
    const auto& verifKey{verifKeyOpt.value()};

    if(!QFileInfo::exists(manifestPath))
    {
        qCritical() << "Manifest update file does not exists. Required to check signature.";
        return {};
    }
    auto manifest_opt{updt::getManifest(manifestPath)};
    if(!manifest_opt)
    {
        qCritical() << "Could not read manifest:" << manifestPath;
        return false;
    }
    const auto& manifest{manifest_opt.value()};
    if(manifest.packageSignature.isEmpty())
    {
        qCritical() << "Manifest file" << manifestPath << "does not contains a signature";
        return {};
    }
    QFileInfo fInfo{fileToCheck};
    auto safeToInstall{signing::checkFileFromSign(fInfo.absoluteFilePath(),verifKey,manifest.packageSignature)};
    return safeToInstall;
}

}//namespace signing

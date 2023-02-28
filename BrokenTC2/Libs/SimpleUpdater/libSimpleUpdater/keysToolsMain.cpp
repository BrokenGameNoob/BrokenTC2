#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

//#define CMAKE_DEBUG_MODE
#include "LoggerHandler.hpp"
#include "Signing.hpp"
#include "hex.h"

//https://www.cryptopp.com/wiki/RSA_Signature_Schemes
//PKCS 1.5
//https://www.cryptopp.com/wiki/Keys_and_Formats#Validating_Keys
//https://www.cryptopp.com/wiki/RSA_Cryptography
//Signature Scheme PKCS v1.5

constexpr auto kPrivateKeyExtension{".private"};
constexpr auto kPublicKeyExtension{".public"};
constexpr auto kSignatureExtension{".sig"};

inline
void addArgsOption(QCommandLineParser* parser){
    parser->addOptions({
                          {{"o", "output-name"},
                            QCoreApplication::translate("main", "Generate <outputName.private> <outputName.public>"),
                            QCoreApplication::translate("main", "outputName")},
                           {{"g", "generate-keys"},
                            QCoreApplication::translate("main", "Generate keys pair"),
                            QCoreApplication::translate("main", "generatedKeysBasename")},
                           {{"r", "check-private"},
                            QCoreApplication::translate("main", "Private key input (signer) (check validity as well)"),
                            QCoreApplication::translate("main", "privateKeyFile")},
                           {{"u", "check-public"},
                            QCoreApplication::translate("main", "Public key input (verifier) (check validity as well)"),
                            QCoreApplication::translate("main", "publicKeyFile")},
                           {{"s", "sign"},
                            QCoreApplication::translate("main", "Sign file (use with -r (optional -o))"),
                            QCoreApplication::translate("main", "fileToSign")},
                           {{"c", "check"},
                            QCoreApplication::translate("main", "Check file signature (use with -u)"),
                            QCoreApplication::translate("main", "fileToCheck")},
                           {{"p", "sig"},
                            QCoreApplication::translate("main", "Specify a signature file"),
                            QCoreApplication::translate("main", "signatureFile")}
                      });
}

int main(int argc, char *argv[])
{
    installCustomLogHandler("CppSigning.log");
    QCoreApplication a(argc, argv);

    QCommandLineParser parser{};
    parser.addHelpOption();
    addArgsOption(&parser);

    //

    parser.process(a);

    QString outName{};
    if(parser.isSet("output-name"))
    {
        outName = parser.value("output-name");
    }

    if(parser.isSet("generate-keys")){
        outName = parser.value("generate-keys");
        if(outName.isEmpty())
        {
            qCritical() << "Please specify an output base name for the option \"g\"";
            return 1;
        }

        const auto kPrivateKeyName{QString{"%0%1"}.arg(outName,kPrivateKeyExtension)};
        const auto kPublicKeyName{QString{"%0%1"}.arg(outName,kPublicKeyExtension)};

        QFileInfo fInfo{kPrivateKeyName};
        if(outName.isEmpty() || !fInfo.dir().mkpath("."))
        {
            qCritical() << "Please enter a valid output-name (not empty, in a writable dir) "<< kPrivateKeyName << "     " << kPublicKeyName;
            qCritical() << outName.isEmpty() << "  " << !fInfo.dir().mkpath(".");
            return 1;
        }
        qInfo() << "Private key: " << kPrivateKeyName;
        qInfo() << "Public key: " << kPublicKeyName << "\n";
        const auto [kPrivate,kPublic]{signing::getKeyPair()};

//        if(!signing::validateKey(kPrivate,signing::RSACheckLevel::kBasicSecured))
//        {
//            qCritical().nospace() << "Private key validation failed with level <"<<signing::RSACheckLevel::kBasicSecured<<">";
//        }

//        if(!signing::validateKey(kPublic,signing::RSACheckLevel::kBasicSecured))
//        {
//            qCritical().nospace() << "Private key validation failed with level <"<<signing::RSACheckLevel::kBasicSecured<<">";
//        }

        qInfo() << "Saving keys success? ->"<<signing::saveKeys(kPrivate,kPublic,kPrivateKeyName.toStdString(),kPublicKeyName.toStdString());
    }

    std::optional<CryptoPP::RSASSA_PKCS1v15_SHA_Signer> signerOpt;
    if(parser.isSet("check-private"))
    {
        auto inName{parser.value("check-private")};
        signerOpt = signing::loadSigner(inName);
        if(!signerOpt)
        {
            qInfo() << "Invalid key or failed to load file" << inName;
        }
        else
        {
            qInfo() << "Valid private key in file" << inName;
        }
    }

    std::optional<CryptoPP::RSASSA_PKCS1v15_SHA_Verifier> verifierOpt;
    if(parser.isSet("check-public"))
    {
        auto inName{parser.value("check-public")};
        verifierOpt = signing::loadVerifier(inName);
        if(!verifierOpt)
        {
            qInfo() << "Invalid key or failed to load file" << inName;
        }
        else
        {
            qInfo() << "Valid public key in file" << inName;
        }
    }

    if(parser.isSet("sign"))
    {
        if(!signerOpt)
        {
            qCritical() << "Could not load Signer key. Please specify one with \"-s\"";
            return 1;
        }

        auto inName{parser.value("sign")};
        if(!outName.isEmpty())
        {
            auto result{signing::signFileToFile(inName.toStdString(),signerOpt.value(),outName)};
            qInfo() << "Saving signature as" << outName << ": Result:" << result;
        }
        else
        {
            auto signature{signing::signFile(inName.toStdString(),signerOpt.value())};
            using namespace CryptoPP;
            qInfo() << "Signature=";
            StringSource(signature, true, new HexEncoder(new FileSink(std::cout)));
            std::cout << "\n";
            qDebug() << QString::fromStdString(signature);
            qDebug() << signature.size();
        }
    }

    if(parser.isSet("check"))
    {
        if(!verifierOpt)
        {
            qCritical() << "Could not load Verifier key. Please specify one with \"-u\"";
            return 1;
        }

        if(!parser.isSet("sig"))
        {
            qCritical() << "Please specify a signature file using option \"--sig\"";
            return 1;
        }

        auto sigFile{parser.value("sig")};
        if(!QFileInfo::exists(sigFile))
        {
            qCritical() << "Signature file:" << sigFile << " does not exists";
            return 1;
        }

        auto inFilename{parser.value("check")};

        auto result{signing::checkFile(inFilename, verifierOpt.value(),sigFile)};
        if (result)
            qInfo() << "Verified signature on file";
        else
            qInfo() << "Failed to verify signature hash on file";
    }

    return 0;
}

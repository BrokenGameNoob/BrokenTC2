#pragma once

#include <QString>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QDebug>

#include <concepts>

template<typename T>
concept HasNoSpaceFunc = requires(T& stream){stream.nospace();};

struct ProgArgs{
    enum Target{
        INVALID = 0,
        CREATE_PACKAGE = 1,
        INSTALL_UPDATE = 2
    };
    QString targetStr()const{
        switch (progGoal) {
        case Target::INVALID:
            return "INVALID";
            break;
        case Target::CREATE_PACKAGE:
            return "CREATE_PACKAGE";
            break;
        case Target::INSTALL_UPDATE:
            return "INSTALL_UPDATE";
            break;
        default:
            return "----";
            break;
        }
    }

    Target progGoal{Target::INVALID};

    bool quiet{false};

    //update from archive
    QString inputUpdateArchive{};
    QString outputFolder{};
    QString verifierKeyFile{};

    //create archive
    QString manifestFile{}; //Giving this argument means creating a new update archive
    QString signerKeyFile{};
};

template<HasNoSpaceFunc Stream_t>
Stream_t operator<<(Stream_t stream,const ProgArgs& args){
    stream << "Args{" << "\n";
    stream.nospace() << "\tprogGoal=" << args.progGoal << " (" << args.targetStr() << ")\n";
    stream.nospace() << "\tinputUpdateArchive=" << args.inputUpdateArchive << "\n";
    stream.nospace() << "\toutputFolder=" << args.outputFolder << "\n";
    stream.nospace() << "\tverifierKeyFile=" << args.verifierKeyFile << "\n";
    stream.nospace() << "\tmanifestFile=" << args.manifestFile << "\n";
    stream.nospace() << "\tsignerKeyFile=" << args.signerKeyFile << "\n";
    stream << "}";
    return stream;
}

inline
void addArgsOption(QCommandLineParser& parser){
    parser.addOptions({
                          // A boolean option with a single name (-p)
//                          {"p",
//                           QCoreApplication::translate("main", "Show progress during copy")},
//                          // A boolean option with multiple names (-f, --force)
//                          {{"f", "force"},
//                           QCoreApplication::translate("main", "Overwrite existing files.")},
                          // An option with a value

                          //update from archive
                          {{"i", "input-archive"},
                           QCoreApplication::translate("main", "Use <input-archive> as an input for the update. If the output folder is omitted, the update package will be extracted in the same dir as the package."),
                           QCoreApplication::translate("main", "inputUpdateArchive")},
                          {{"v", "verifier-key-file"},
                           QCoreApplication::translate("main", "Used the given file as the public key file required to check a signed update package."),
                           QCoreApplication::translate("main", "signerKeyFile")},
                          {{"o", "output-folder"},
                           QCoreApplication::translate("main", "Use a specific directory as an output for a given command."),
                           QCoreApplication::translate("main", "outputFolder")},
                          {{"m", "manifest"},
                           QCoreApplication::translate("main", "Create an update archive based on the manifest file specified if the option \"-i\" is not specified. Use the manifest to install the update otherwise"),
                           QCoreApplication::translate("main", "manifestFile")},
                          {{"s", "signer-key-file"},
                           QCoreApplication::translate("main", "Sign an update package. Produces a \"manifestSigned.json\" containing the signature of the created update package. Requires the private signer key to be given as an argument."),
                           QCoreApplication::translate("main", "signerKeyFile")},
                          {{"q", "quiet"},
                           QCoreApplication::translate("main", "Will proceed without showing any window nor asking for user interactions")},
//                          {{"s", "show-window"},
//                           QCoreApplication::translate("main", "Show an interface")},
                      });
}

inline
ProgArgs parseArgs(const QCommandLineParser& parser){
    ProgArgs out{};
    auto inputUpdateArchiveSet{parser.isSet("input-archive")};
    auto outputFolderSet{parser.isSet("output-folder")};
    auto manifestFileSet{parser.isSet("manifest")};
    auto signerKeyFileSet{parser.isSet("signer-key-file")};
    auto verifierKeyFileSet{parser.isSet("verifier-key-file")};
    out.quiet = parser.isSet("quiet");
//    auto showWindowSet{parser.isSet("show-window")};

//    out.showWindow = showWindowSet;

    //invalid combination : we can't install an update AND create an archive in the same time
//    if((inputUpdateArchiveSet || outputFolderSet) && manifestFileSet)
//        return out;

    //ask to extract archive somewhere but don't specify from which archive
    if(outputFolderSet && !(inputUpdateArchiveSet || manifestFileSet))
        return out;

    out.inputUpdateArchive = parser.value("input-archive");
    out.outputFolder = parser.value("output-folder");
    out.manifestFile = parser.value("manifest");
    out.signerKeyFile = parser.value("signer-key-file");
    out.verifierKeyFile = parser.value("verifier-key-file");

    if(inputUpdateArchiveSet)
    {
        out.progGoal = ProgArgs::INSTALL_UPDATE;
    }

    //don't if we are installing an update (priority on option "installing"
    if(manifestFileSet && !inputUpdateArchiveSet)
    {
        out.progGoal = ProgArgs::CREATE_PACKAGE;

        if(!signerKeyFileSet)
        {
            qWarning() << "You should create a signed update package";
        }
        else if(out.signerKeyFile.isEmpty())
        {
            qWarning() << "Please indicate a file storing the private signer key (missing argument)";
        }
    }

    if(signerKeyFileSet && !manifestFileSet)
    {
        qCritical() << "Invalid arguments combination: You can't specify a signer key without giving a manifest file (option -m)";
        out.progGoal = ProgArgs::INVALID;
    }

    if(signerKeyFileSet && verifierKeyFileSet)
    {
        qCritical() << "Invalid arguments combination: You can't specify both a signer key and a verifier key";
        out.progGoal = ProgArgs::INVALID;
    }

    if(verifierKeyFileSet && !manifestFileSet && !inputUpdateArchiveSet)
    {
        qCritical() << "Invalid arguments combination: You can't specify a verifier key without giving a manifest file (option -m) and an update archive input (option -i)";
        out.progGoal = ProgArgs::INVALID;
    }

    return out;
}

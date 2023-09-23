#include "TextToSpeech.hpp"

#include <QDebug>

namespace cus{

bool TextPlayer::m_is_init{false};

TextPlayer::TextPlayer(): QObject{nullptr},
    m_speech{},
    m_enable{true}
{
    const auto kFirstCompatibleEngineOpt{GetFirstCompatibleEngine()};
    if(kFirstCompatibleEngineOpt){
        m_speech = new QTextToSpeech(kFirstCompatibleEngineOpt.value(),this);
        qInfo() << "Feature text to speech using engine:" << m_speech->engine();
    } else {
        qWarning() << "Unavailable feature: text to speech";
    }
}

std::optional<QString> TextPlayer::GetFirstCompatibleEngine(){
    const auto kAvailable{QTextToSpeech::availableEngines()};
    for(const auto& engine : kCompatibleEngines){
        if(kAvailable.contains(engine)){
            return engine;
        }
    }
    return {};
}

void TextPlayer::Play(const QString& input){
    if(!HasEngine()){
        return;
    }

    qDebug() << "Playing:" << input;

    i().Speech()->stop();
    i().Speech()->say(input);
}

}//namespace cus

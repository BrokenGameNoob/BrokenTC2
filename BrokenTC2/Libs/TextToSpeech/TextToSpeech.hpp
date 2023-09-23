#pragma once

#include <QObject>
#include <QTextToSpeech>

namespace cus{

/*!
 * \brief The TextPlayer class
 * Do not forget to call the "Init" function before doing anything else
 *
 * You can then play all the functions you want
 * such as
 * Play(QString)
 * or
 * SetRate(double)
 */
class TextPlayer : public QObject
{
private:
    auto* Speech(){
        assert(m_is_init);
        return m_speech;
    }

public:
    static constexpr std::array kCompatibleEngines = {"sapi"};

public:
    static auto& instance(){
        static TextPlayer i{};
        return i;
    }
    static auto& i(){
        return instance();
    }
    static const auto& ci(){
        return instance();
    }
    static auto* p(){
        return &i();
    }

    const auto* Speech()const{
        return m_speech;
    }

    static bool Init(){
        i();
        m_is_init = true;
        return HasEngine();
    }

    static bool HasEngine(){
        return ci().Speech();
    }

    static void SetRate(double rate = 0){
        assert(rate <= 1 && rate >= -1 && "Rate has to be in [-1,1], 0 being the default");
        i().Speech()->setRate(rate);
    }
    static auto Rate(){
        return i().Speech()->rate();
    }

    static void SetLocale(QLocale locale){
        if(!ci().Speech()->availableLocales().contains(locale)){
            qWarning() << "TextPlayer: Failed to set locale (not available):" << locale;
            return;
        }
        i().Speech()->setLocale(locale);
    }
    static auto Locale(){
        return ci().Speech()->locale();
    }

    static void SetVolume(double volume){
        assert(volume <= 1 && volume >= 0 && "Volume has to be in [0,1]");
        i().Speech()->setVolume(volume);
    }
    static auto Volume(){
        return ci().Speech()->volume();
    }

    static void SetEnable(bool state){i().m_enable = state;}
    static auto Enabled(){return ci().m_enable;}

public:
    /*!
     * \brief Is the text to speech service available
     */
    static bool Available(){
        return GetFirstCompatibleEngine().has_value();
    }
    static void Play(const QString& input);

private:
    TextPlayer();
    static std::optional<QString> GetFirstCompatibleEngine();

private:
    static bool m_is_init;
    QTextToSpeech *m_speech;
    bool m_enable;
};

}//namespace cus

#ifndef QSDL_GLOBAL_HPP
#define QSDL_GLOBAL_HPP

#include <cstdint>

#include <SDL2/SDL.h>

#include <QStringList>

namespace qsdl {

//this struct contains data shared between the EventHandler and the EventThread
class EventHandlerSharedConfig
{
public:
    EventHandlerSharedConfig(bool lowPerfMode = false): m_lowPerfMode{lowPerfMode}
    {

    }

    bool lowPerfMode()const{
        return m_lowPerfMode;
    }
    void setLowPerfMode(bool enable){
        m_lowPerfMode = enable;
    }

private:
    friend class SDLEventThread;
    friend class SDLEventHandler;

    bool m_lowPerfMode;


    bool m_shouldStop{false};
};

bool initSDL(uint64_t flags);

inline
int getPluggedJoysticksCount(){
    return SDL_NumJoysticks();
}

QStringList getPluggedJoysticks();

inline
QString getPluggedJoystick(int id)
{
    return SDL_JoystickNameForIndex(id);
}

/*!
 * \brief findJoystickByName search for a plugged in joystick by it's name
 * \return joystick open ID if found, -1 otherwise
 */
int findJoystickByName(const QString name);

} // namespace qsdl

#endif // QSDL_GLOBAL_HPP

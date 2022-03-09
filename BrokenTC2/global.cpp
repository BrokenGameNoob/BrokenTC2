#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <QString>
#include <algorithm>

#pragma clang diagnostic ignored "-Wstring-plus-int"
#define __FILENAME_PRIVATE__ (__FILE__ + SOURCE_PATH_SIZE)

#ifndef __LINE__
#error "Line macro used for debugging purpose is not defined"
#endif

#ifndef __FILE__
#error "File macro used for debugging purpose is not defined"
#else
#define _LINE_ QString::number(__LINE__)
#endif


#define __CURRENT_PLACE__ QString{"%0 : <%1> : l.%2"}.arg(__FILENAME_PRIVATE__,__func__,_LINE_)

inline
QString removeSpecialChars(QString str){
    auto it = std::remove_if(str.begin(), str.end(), [](const QChar& c){ return !c.isLetterOrNumber();});
    str.chop(std::distance(it, str.end()));
    return str;
}

#endif // GLOBAL_HPP

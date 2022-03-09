#ifndef UPDT_POSTUPDATE_HPP
#define UPDT_POSTUPDATE_HPP

#include <QFile>
#include <QFileInfo>

namespace updt {

constexpr auto UPDATED_TAG_FILENAME{"UPDATED.TAG"};

inline
bool wasUpdated(){
    return QFile::exists(UPDATED_TAG_FILENAME);
}

inline
void markAsNotUpdated(){
    QFile::remove(UPDATED_TAG_FILENAME);
}

/*!
 * \brief postUpdateFunction is used to execute code when the soft got updated
 */
void postUpdateFunction();

} // namespace updt

#endif // UPDT_POSTUPDATE_HPP

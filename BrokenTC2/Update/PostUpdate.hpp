#ifndef UPDT_POSTUPDATE_HPP
#define UPDT_POSTUPDATE_HPP

#include <QFile>
#include <QFileInfo>

#include "Utils/GUITools.hpp"

namespace updt {

constexpr auto UPDATED_TAG_FILENAME{"UPDATED.TAG"};

/*!
 * \brief wasUpdated
 * \return
 */
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
 * \return true if the soft was updated, false otherwise
 */
bool postUpdateFunction();

inline
void showChangelog(QWidget *parent)
{
    utils::gui::showMarkdown(parent,":/update/update/changelog.md");
}

} // namespace updt

#endif // UPDT_POSTUPDATE_HPP

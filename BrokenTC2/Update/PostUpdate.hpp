/* Broken The Crew 2 sequential clutch assist
 * Copyright (C) 2022 BrokenGameNoob <brokengamenoob@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

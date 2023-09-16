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

#pragma once

#include <QString>
#include <map>

namespace tc{

enum GameId{
    THE_CREW2,
    THE_CREW_MOTORFEST
};

struct GameInfo{
    const QString kGameName;
    const QString kFolderInDocuments;
    const QString kProcessName;
};

static inline const auto& GetGameInfo(GameId gameId){
    static const std::map<GameId,GameInfo> kGamesInfo{
        {GameId::THE_CREW2,{"The Crew 2", "The Crew 2", "TheCrew2.exe"}},
        {GameId::THE_CREW_MOTORFEST,{"The Crew Motorfest", "TheCrewMotorfest", "TheCrewMotorfest.exe"}}
    };
    return kGamesInfo.at(gameId);
}



}

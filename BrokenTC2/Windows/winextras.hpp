/* reuse of antimicro Gamepad to KB+M event mapper https://github.com/AntiMicroX/antimicrox
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#ifndef WINDAUBE_WINEXTRAS_H
#define WINDAUBE_WINEXTRAS_H

#include <windows.h>

#include <QHash>
#include <QObject>
#include <QPoint>
#include <QString>

class WinExtras : public QObject
{
    Q_OBJECT
  public:
    static QString getDisplayString(unsigned int virtualkey);
    static unsigned int getVirtualKey(QString codestring);
    static unsigned int correctVirtualKey(unsigned int scancode, unsigned int virtualkey);
    static unsigned int scancodeFromVirtualKey(unsigned int virtualkey, unsigned int alias = 0);

    static const unsigned int EXTENDED_FLAG;
    static QString getForegroundWindowExePath();
    static bool containsFileAssociationinRegistry();
    static void writeFileAssocationToRegistry();
    static void removeFileAssociationFromRegistry();
    static bool IsRunningAsAdmin();
    static bool elevateAntiMicro();
    static void disablePointerPrecision();
    static void enablePointerPrecision();
    static bool isUsingEnhancedPointerPrecision();
    static void grabCurrentPointerPrecision();
    static QString getCurrentWindowText();
    static bool raiseProcessPriority();
    static QPoint getCursorPos();

  protected:
    explicit WinExtras(QObject *parent = 0);
    void populateKnownAliases();

    static WinExtras _instance;
    QHash<QString, unsigned int> knownAliasesX11SymVK;
    QHash<unsigned int, QString> knownAliasesVKStrings;
    static int originalMouseAccel;

  signals:

  public slots:
};

QString VirtualKeyCodeToString(UCHAR virtualKey);

#endif // WINDAUBE_WINEXTRAS_H

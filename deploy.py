#encoding : utf-8

import glob
import os
import re
import inspect
import pathlib
import shutil
from wsgiref.handlers import read_environ

def filterListWithRegex(inVar : list, regex : str):
    return list(filter(re.compile(regex).match,inVar))

def ls(rootPath: str = ".", regex : str = None) -> list:
    if(not rootPath.endswith("/*")):
        rootPath += "/*"
    tmpList = glob.glob(rootPath,recursive=False)
    if(regex == None):
        return tmpList
    return filterListWithRegex(tmpList,regex)

def errorOccured(what : str = None,quitProg : bool = False):
    print("------- {} -------- An error occurred :\n\t{}".format(inspect.stack()[1].function,what))
    if(quitProg):
        exit(1)


def mkpathOverwrite(path : str) -> bool:
    """
    This function search for a dir, DELETE it if found
    then re-create it (empty)
    
    BE CAREFUL WHEN USING IT
    """
    if(path == "./"):
        errorOccured("You should not delete current dir",True)
    dirPath = pathlib.Path(path)
    if(dirPath.exists() and dirPath.is_dir()):
        shutil.rmtree(dirPath)
    os.makedirs(path)
    return True

def checkFile(path : str)->bool:
    tmpPath = pathlib.Path(path)
    if(not tmpPath.is_file() or not tmpPath.exists()):
        return False
    return True

#-------------------------------------------------------------

def getReleaseDir(rootPath : str = ".") -> str:
    tmpList = ls(".",".*build-.*Desktop_Qt_6.*MinGW.*_64.*-Release")
    # tmpList = ls()
    if(len(tmpList) != 1):
        errorOccured("Too many build-release folder :\n{}".format(str(tmpList)),False)
        return None
    return tmpList[0]


def getExePath(releaseDir : str) -> str:
    tmpList = ls(releaseDir,".*\.exe")
    # tmpList = ls(releaseDir)
    if(len(tmpList) != 1):
        errorOccured("Too many exe found :\n{}".format(str(tmpList)),False)
        return None
    return tmpList[0]


def main():
    CONFIG_OUTPUT_DIR = "AUTO"#the dir will have the name of the found executable
    CONFIG_WINDEPLOYQT_PATH = "C:/Qt/6.2.3/mingw_64/bin/windeployqt.exe"


    print("------------------------------------------------------")



    releaseDir = getReleaseDir(".")
    if(not releaseDir):
        errorOccured("Cannot find release dir",True)
    print("Found build release dir at : {}".format(releaseDir))

    exePath = getExePath(releaseDir)
    if(not exePath):
        errorOccured("Cannot find exe path",True)
    print("Found exe at : {}".format(exePath))

    if(CONFIG_OUTPUT_DIR == "AUTO"):
        CONFIG_OUTPUT_DIR = "./{}".format(pathlib.Path(exePath).stem)

    print("Creating deploy dir <{}> ...".format(CONFIG_OUTPUT_DIR))
    mkpathOverwrite(CONFIG_OUTPUT_DIR)#erase and re-create deploy dir

    print("Copying exe...")
    shutil.copy(exePath,CONFIG_OUTPUT_DIR)

    print("Running windeployqt...")
    if(not checkFile(CONFIG_WINDEPLOYQT_PATH)):
        errorOccured("Cannot find windeployqt exe at {}".format(CONFIG_WINDEPLOYQT_PATH),True)
    windeployCmd = "{} {} {}".format(CONFIG_WINDEPLOYQT_PATH,CONFIG_OUTPUT_DIR,exePath)
    print("\tusing command <{}>".format(windeployCmd))
    if(os.system(windeployCmd)):#if windeployqt failed
        errorOccured("windeployqt failed to execute properly.",True)
    
    print("")

    print("Deleting useless files")
    uselessFilePath = "{}/opengl32sw.dll".format(CONFIG_OUTPUT_DIR)
    if(checkFile(uselessFilePath)):
        print("\tDeleting : <{}>".format(uselessFilePath))
        try:
            os.remove(uselessFilePath)
        except:
            errorOccured("Cannot delete <{}>".format(uselessFilePath),False)#non fatal error, keep going
    print("Done")


    print("\nCreating setup...")
    

    print("------------------------------------------------------")


if(__name__ == "__main__"):
    main()
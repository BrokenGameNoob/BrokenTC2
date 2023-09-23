#encoding : utf-8

import glob
import os
import re
import inspect
import pathlib
import shutil
import subprocess
import time

GLOB_TARGET_EXE_NAME = ""

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
    isFile = tmpPath.is_file()
    exist = tmpPath.exists()
    if((not isFile) or (not exist)):
        return False
    return True

#-------------------------------------------------------------

def getReleaseDir(rootPath : str = ".") -> str:
    tmpList = ls(rootPath,".*build-.*Desktop_Qt_6.*MinGW.*_64.*-Release")
    # tmpList = ls()
    for i, dir in enumerate(tmpList):
        tmpList[i] = dir + "/Bin"
    if(len(tmpList) != 1):
        errorOccured("Too many build-release folder :\n{}".format(str(tmpList)),False)
        return None
    if(len(tmpList) == 0):
        errorOccured("No build release folder found")
    return tmpList[0]


def getExePath(releaseDir : str,exeRegex:str = None) -> str:
    global GLOB_TARGET_EXE_NAME
    if exeRegex == None:
        exeRegex = f".*{GLOB_TARGET_EXE_NAME}\.exe"
    tmpList = ls(releaseDir,exeRegex)
    # tmpList = ls(releaseDir)
    if(len(tmpList) > 1):
        errorOccured("Too many exe found :\n{}".format(str(tmpList)),False)
        return None
    elif len(tmpList) == 0:
        errorOccured("Exe not found",False)
        return None
    return tmpList[0]


def getAbsolute(path: str)->str:
    return str(pathlib.Path(path).absolute())

def main():
    global GLOB_TARGET_EXE_NAME
    CONFIG_outputDir = "AUTO"#the dir will have the name of the found executable
    QT_VERSION = "6.5.0"
    MINGW_VERSION = "mingw1120_64"
    CONFIG_WINDEPLOYQT_PATH = getAbsolute(f"C:/Qt/{QT_VERSION}/mingw_64/bin/windeployqt.exe")
    CONFIG_DEPENDENCY_DIR = getAbsolute("../BrokenTC2/assets/dependency")
    CONFIG_INNOSETUP_SCRIPT = getAbsolute("./buildSetup.iss")
    CONFIG_DEPLOY_OUTPUT_DIR = "DEPLOY_OUTPUT"

    CONFIG_PRIVATE_SIGNER_KEY_FILE = getAbsolute("../keys/BrokenTC2.private")
    CONFIG_PUBLIC_VERIFIER_KEY_FILE = getAbsolute("../keys/BrokenTC2.public")
    CONFIG_MANIFEST_FILE = "manifest.json"
    CONFIG_outputAssetDir = getAbsolute(f"{CONFIG_outputDir}/Assets/")

    GLOB_TARGET_EXE_NAME = "BrokenTC2"


    print("------------------------------------------------------")



    releaseDir = getAbsolute(getReleaseDir(".."))
    if(not releaseDir):
        errorOccured("Cannot find release dir",True)
    print("Found build release dir at : {}".format(releaseDir))

    exePath = getAbsolute(getExePath(releaseDir))
    if(not exePath):
        errorOccured("Cannot find exe path",True)
    print("Found exe at : {}\n".format(exePath))

    if(CONFIG_outputDir == "AUTO"):
        CONFIG_outputDir = getAbsolute("./{}/{}".format(CONFIG_DEPLOY_OUTPUT_DIR,pathlib.Path(exePath).stem+"_release"))
        CONFIG_outputAssetDir = getAbsolute(f"{CONFIG_outputDir}/Assets/")


    



    uselessFileList = []
    uselessFileList.append(f"{CONFIG_outputDir}/opengl32sw.dll")
    uselessFileList.append(f"{CONFIG_outputDir}/D3Dcompiler_47.dll")
    uselessFileList.append(f"{CONFIG_outputDir}/Qt6Svg.dll")

    
    dependencies = []
    dependencies.append((f"{CONFIG_PUBLIC_VERIFIER_KEY_FILE}",CONFIG_outputAssetDir))
    dependencies.append((f"{CONFIG_DEPENDENCY_DIR}/SDL2.dll",CONFIG_outputDir))
    




    print("Creating deploy dir <{}> ...".format(CONFIG_outputDir))
    mkpathOverwrite(CONFIG_outputDir)#erase and re-create deploy dir
    print("Done\n")

    print("Copying exe...")
    try:
        shutil.copy(exePath,CONFIG_outputDir)
    except:
        errorOccured("Cannot copy exe file",True)
    print("Done\n")

    

    simpleUpdaterExeFile= getAbsolute(getExePath(releaseDir,".*SimpleUpdater\.exe"))
    print("Copying SimpleUpdater exe...")
    try:
        shutil.copy(simpleUpdaterExeFile,CONFIG_outputDir)
    except:
        errorOccured("Cannot copy SimpleUpdater exe file",True)
    print("Done\n")

    print("Running windeployqt...")
    if(not checkFile(CONFIG_WINDEPLOYQT_PATH)):
        errorOccured("Cannot find windeployqt exe at {}".format(CONFIG_WINDEPLOYQT_PATH),True)
    windeployCmd = "{} --no-translations {} {}".format(CONFIG_WINDEPLOYQT_PATH,CONFIG_outputDir,exePath)
    # qtEnv = {**os.environ, 'PATH': f'C:\\Qt\\{QT_VERSION}\\mingw_64\\bin;C:\\Qt\\Tools\\{MINGW_VERSION}\\bin;' + os.environ['PATH']}
    qtEnv = {**os.environ, 'PATH': f'C:\\Qt\\{QT_VERSION}\\mingw_64\\bin;C:\\Qt\\Tools\\{MINGW_VERSION}\\bin;'}
    print("\tusing command <{}>".format(windeployCmd))
    rval = subprocess.Popen(windeployCmd, env=qtEnv)
    if rval.returncode != 0:
        errorOccured("WARNING: windeployqt may have failed to execute properly.",False)
    print("Done")


    print(f"Creating target assets dir <{CONFIG_outputAssetDir}> ...")
    mkpathOverwrite(CONFIG_outputAssetDir)
    print("Done\n")

    print("Adding dependencies")
    
    for dependency in dependencies:
        dependencyPath = dependency[0]
        outputDir = dependency[1]
        print("\tAdding : <{}>".format(dependencyPath))
        if(not checkFile(dependencyPath)):
            errorOccured("Cannot find following dependency : <{}>".format(dependencyPath),True)
        try:
            shutil.copy(dependencyPath,outputDir)
        except:
            errorOccured("Cannot copy dependency <{}> to <{}>".format(dependencyPath,outputDir),True)
    print("Done\n")
    


    for uselessFilePath in uselessFileList:
        toRemove = pathlib.Path(uselessFilePath)
        for i in range(0,10):
            if not checkFile(str(toRemove.absolute())):
                print(f"Waiting for {toRemove}")
                time.sleep(0.3)
            else:
                break
        print("\tDeleting : <{}>".format(toRemove.absolute()))
        try:
            toRemove.unlink()
        except:
            errorOccured("Cannot delete <{}>".format(toRemove),False)#non fatal error, keep going
    print("Done\n")


    print("Creating update package")
    try:
        suExePath = f"{CONFIG_outputDir}/SimpleUpdater.exe"
        args = [
            suExePath,
            "-q",
            "-s",
            CONFIG_PRIVATE_SIGNER_KEY_FILE,
            "-m",
            CONFIG_MANIFEST_FILE,
            "-r",
            CONFIG_outputDir,
            "-o",
            CONFIG_DEPLOY_OUTPUT_DIR
        ]
        rval = subprocess.run(args)
        if rval.returncode != 0:
            errorOccured("Could not run SimpleUpdater create update package",True)
    except Exception as e:
        print(e)
        errorOccured("Cannot create update package",True)
    print("Done\n")



    print("##########################################")
    print("\nCreating setup...")

    os.system(" {}".format(CONFIG_INNOSETUP_SCRIPT))
    
    innoSetup = f"C:\\Program Files (x86)\\Inno Setup 6\\iscc.exe"
    args = [
        innoSetup,
        CONFIG_INNOSETUP_SCRIPT
    ]
    rval = subprocess.run(args)
    if rval.returncode != 0:
        errorOccured("Failed to run the inno setup compiler",False)
    print("##########################################\n")
    

    print("------------------------------------------------------")


if(__name__ == "__main__"):
    main()
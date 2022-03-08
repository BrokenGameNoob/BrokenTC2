#encoding : utf-8

import glob
import os
import re
import inspect
import subprocess
import time
from pathlib import Path


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

#--------------------------------------------------

def main():
    UPDATED_TAG_FILENAME = "UPDATED.TAG"

    updateFile = ls(".",".*\\.update$")
    if(len(updateFile) != 0):
        updateFile = updateFile[0]
    oldExe = ls(".",".*\\.exe$")
    if(len(oldExe) != 0):
        oldExe = oldExe[0]

    print("Update file {}".format(updateFile))
    print("Running file {}".format(oldExe))
    print("Running script {}".format(__file__))

    fTest = None
    start_t = time.time()
    while(1):
        try:
            fTest=open(oldExe,"w")
            print("OPEN")
            break
        except:
            print("Retrying")
            time.sleep(0.2)
        end_t = time.time()
        if(end_t-start_t > 20.):#timeout if last more than 20s
            exit(0)
    fTest.close()
    print("Deletion...")
    
    os.remove(oldExe)
    os.rename(updateFile,oldExe)

    os.remove(__file__)

    Path(UPDATED_TAG_FILENAME).touch()

    subprocess.Popen(oldExe)

if(__name__ == "__main__"):
    main()
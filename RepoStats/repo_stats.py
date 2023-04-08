#encoding: utf-8

import json
from urllib.request import urlopen
import os
from dataclasses import dataclass, is_dataclass, asdict
from datetime import datetime
import argparse

def loadStrFromUrl(url: str)->str:
    try:
        response = urlopen(url)
        return response.read()
    except Exception as e:
        print(f"Error ocurred when retrieving information online from \"{url}\" -> {e}")
        return ""

def fileExists(file_path: str)->bool:
    return os.path.exists(file_path)


def saveStrToFile(input_str: str, file_path: str):
    with open(file_path, 'w') as file:
        file.write(input_str)

def readStrFromFile(file_path: str) -> str:
    with open(file_path) as file:
        return file.read()


def saveJsonToFile(input_json, file_path: str):
    with open(file_path, 'w') as file:
        json.dump(input_json, file, )

def readJsonFromFile(file_path: str) -> str:
    with open(file_path) as file:
        return json.loads(file.read())

def loadJsonFromString(input_str: str):
    return json.loads(input_str)

#------------------ Parsing 
def nested_dataclass(*args, **kwargs):
    def wrapper(cls):
        cls = dataclass(cls, **kwargs)
        original_init = cls.__init__
        def __init__(self, *args, **kwargs):
            for name, value in kwargs.items():
                field_type = cls.__annotations__.get(name, None)
                if is_dataclass(field_type) and isinstance(value, dict):
                     new_obj = field_type(**value)
                     kwargs[name] = new_obj
            original_init(self, *args, **kwargs)
        cls.__init__ = __init__
        return cls
    return wrapper(args[0]) if args else wrapper

@dataclass
class DownloadsCounts:
    setup_downloaded_count: int = 0
    update_downloaded_count: int = 0
    
    def total(self)->int:
        return self.setup_downloaded_count + self.update_downloaded_count
    def hasDl(self)->bool:
        return self.setup_downloaded_count != 0 or self.update_downloaded_count != 0


@dataclass
class Release:
    downloads: DownloadsCounts
    tag_name: str = ""
    name: str = ""
    
    def __post_init__(self):
        if not self.downloads:
            self.downloads = DownloadsCounts()
    def hasDl(self)->bool:
        return self.downloads.hasDl()


def getDownloadsCountsFromAssetList(asset_dict_list: dict)->DownloadsCounts:
    out: DownloadsCounts = DownloadsCounts()
    for asset in asset_dict_list:
        dl_count = asset["download_count"]
        if asset["name"] == "BrokenTC2_setup_x64.exe":
            out.setup_downloaded_count = dl_count
        elif asset["name"] == "Update.pck":
            out.update_downloaded_count = dl_count
    return out


def getReleaseList(api_json)->list:
    out = []
    for rel in api_json:
        tmp = Release(getDownloadsCountsFromAssetList(rel["assets"]),rel["tag_name"],rel["name"])
        out.append(tmp)
    return out



def main(args: dict):
    REPO_RELEASES_URL = "https://api.github.com/repos/BrokenGameNoob/BrokenTC2/releases"
    BUFFER_FILE = "json_buffer.ignore"
    STATS_FILE = "repo_stats.json"

    api_json = None

    if(fileExists(BUFFER_FILE) and not args["force"]):
        api_json = readJsonFromFile(BUFFER_FILE)
        print("Read json from file")
    else:
        print(f"Read json from web api: {REPO_RELEASES_URL}")
        api_text = loadStrFromUrl(REPO_RELEASES_URL)
        if len(api_text) == 0:
            return
        api_json = loadJsonFromString(api_text)
        saveJsonToFile(api_json, BUFFER_FILE)
    
    if not api_json:
        return
    
    # repo stats json

    repo_stats_element = {"date":"","releases":[]}#releases -> list of Release (dict format)
    repo_stats: dict = {}
    date_str: str = datetime.today().strftime("%d-%m-%Y")

    if(fileExists(STATS_FILE)):
        repo_stats = readJsonFromFile(STATS_FILE)
    


    print("")
    print("---------------------------------")
    print("")
    rel_list: list[Release] = getReleaseList(api_json)
    total_downloads = DownloadsCounts()
    rel_list_to_save = []
    for release in rel_list:
        total_downloads.setup_downloaded_count += release.downloads.setup_downloaded_count
        total_downloads.update_downloaded_count += release.downloads.update_downloaded_count

        if release.hasDl():
            rel_list_to_save.append(asdict(release))
    print(f"Total downloads: {total_downloads}")

    repo_stats[date_str] = {"release_list":rel_list_to_save,"total_downloads":asdict(total_downloads)}
    
    saveJsonToFile(repo_stats, STATS_FILE)



if __name__ == "__main__":
    abspath = os.path.abspath(__file__)
    dir_name = os.path.dirname(abspath)
    os.chdir(dir_name)

    parser = argparse.ArgumentParser(description='Retrieve repository stats')
    parser.add_argument('--force', dest='force', action='store_true',
                        help='force the stats to be retrieved online')

    args = vars(parser.parse_args())

    print(args["force"])

    main(args)
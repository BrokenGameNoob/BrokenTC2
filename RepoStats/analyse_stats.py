#encoding: utf-8

import argparse
import json
import os
from datetime import datetime

import matplotlib.pyplot as plt
import matplotlib.dates as mdates

def readJsonFromFile(file_path: str) -> str:
    with open(file_path) as file:
        return json.loads(file.read())


def plotValueByTimeStr(date_str_list:list,y_values : list):
    x_values = []
    for date_str in date_str_list:
        date = datetime.strptime(date_str, '%d-%m-%Y')
        x_values.append(date)

    plt.plot(x_values,y_values)
    formatter = mdates.DateFormatter('%d-%m-%Y')
    plt.gca().xaxis.set_major_formatter(formatter)
    plt.gcf().autofmt_xdate()
    plt.show()

def drawTotalSetup(json_stats: dict):
    print("Drawing the total setup download count")

    x_values = []
    y_values = []

    for date_str in json_stats:
        x_values.append(date_str)
        y_values.append(json_stats[date_str]["total_downloads"]["setup_downloaded_count"])

    plotValueByTimeStr(x_values,y_values)
    


def main(args: dict):
    STATS_FILE = "repo_stats.json"

    json_stats = readJsonFromFile(STATS_FILE)

    if args["total-setup"]:
        drawTotalSetup(json_stats)

if __name__ == "__main__":
    abspath = os.path.abspath(__file__)
    dir_name = os.path.dirname(abspath)
    os.chdir(dir_name)

    parser = argparse.ArgumentParser(description='Retrieve repository stats')
    parser.add_argument('-s','--total-setup', dest='total-setup', action='store_true',
                        help='draw the graphics ')

    args = vars(parser.parse_args())

    should_continue = False
    for arg in args.keys():
        if args[arg] == True:
            should_continue = True
            break
    if not should_continue:
        parser.print_help()
        exit(1)

    main(args)
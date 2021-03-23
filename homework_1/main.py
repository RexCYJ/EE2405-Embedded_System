# Class:        EE2405 Embedded System Lab
# Author:       108061121, 陳禹嘉
# Edit date:    2021/3/5

# Program name: Homework 1, Weather Station Data Analysis

# =========================================================

# Import module
import csv

# File name
cwd_filename = '108061121.csv'

data = []
header = []
i = 0
aver = {'C0A880':None, 'C0F9A0':None, 'C0G640':None, 
        'C0R190':None, 'C0X260':None}

f = open('output.txt', 'w+')

# Read csv file data
with open(cwd_filename) as csvfile:
    mycsv = csv.DictReader(csvfile)     # read data in dictionary type
    header = mycsv.fieldnames
    for row in mycsv:
        data.append(row)

# find the data with pressure being -99 or -999, and then remove them
while (i < len(data)):
    if (data[i]["PRES"] == '-99' or data[i]["PRES"] == '-999'):
        data.pop(i)
    i += 1

# check the list
for row in data:
    if (row['PRES'] == '-99' or row['PRES'] == '-999'):
        f.write(str(row.values()))
        f.write('\n')

# count average
for key in aver.keys():                 # run every ordered station
    i = 0                               # reset var
    total = 0                           # reset var
    for row in data:                    # scan each row of data
        if row['station_id'] == key:
            i += 1
            total += float(row['PRES'])
    if i != 0:
        aver[key] = round(total / i, 2)

f.write(str(list(aver.items())))

f.close()
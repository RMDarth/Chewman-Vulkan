# Copyright (c) 2018-2019, Igor Barinov
# Licensed under the MIT License
# Flatten resource folder & subfolder into single folder with all files
# (Also fixing file references in materials, shaders and other documents)
import sys
import os
import shutil
import json
import xml.etree.ElementTree as ET


def flatten():
    if len(sys.argv) < 3:
        print("Usage: flatten.py srcFolder dstFolder")
        return

    print("Converting %s..." % sys.argv[1])
    for root, subFolder, files in os.walk(sys.argv[1]):
        for item in files:
            process_file(str(os.path.join(root, item)))


def process_file(filepath):
    print(filepath)
    new_path = os.path.join(sys.argv[2], os.path.basename(filepath))
    if os.path.isfile(new_path):
        print(os.path.basename(filepath) + " file already exists! Skipping.")
        return
    shutil.copyfile(filepath, new_path)

    if new_path.endswith('.material'):
        update_json_file(new_path)
    elif new_path.endswith('.shader'):
        update_json_file(new_path)
    elif new_path.endswith('.mesh'):
        update_json_file(new_path)
    elif new_path.endswith('.xml'):
        update_xml_file(new_path)


def update_json_file(filename):
    with open(filename) as json_file:
        data = json.load(json_file)
    update_json_data(data)
    with open(filename, "w") as json_file:
        json.dump(data, json_file, indent=4, separators=(',', ': '))


def update_json_data(json_data):
    for (key, value) in json_data.items():
        if key == "filename":
            json_data[key] = os.path.basename(value)
        if type(value) is dict:
            update_json_data(value)
        elif type(value) is list:
            update_json_list(value)


def update_json_list(json_list):
    for value in json_list:
        if type(value) is list:
            update_json_list(value)
        if type(value) is dict:
            update_json_data(value)


def update_xml_file(filename):
    tree = ET.parse(filename)
    for element in tree.iter():
        if 'image' in element.attrib:
            element.attrib['image'] = os.path.basename(element.attrib['image'])
        if 'hoverimage' in element.attrib:
            element.attrib['hoverimage'] = os.path.basename(element.attrib['hoverimage'])
        if 'pressedimage' in element.attrib:
            element.attrib['pressedimage'] = os.path.basename(element.attrib['pressedimage'])
        if 'source' in element.attrib:
            element.attrib['source'] = os.path.basename(element.attrib['source'])
    tree.write(filename)


if __name__ == '__main__':
    flatten()

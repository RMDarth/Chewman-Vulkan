# Copyright (c) 2018-2019, Igor Barinov
# Licensed under the MIT License
import sys
import json
from xml.dom import minidom


# Convert .fnt file to json .font file for SVE
# .fnt file can be generated here: http://kvazars.com/littera/
def convert_font():
    if len(sys.argv) < 4:
        print("Usage: convertFont.py fontfile.fnt materialName fontName")
        return
    print("Converting %s..." % sys.argv[1])
    xmldoc = minidom.parse(sys.argv[1])

    itemlist = xmldoc.getElementsByTagName('char')
    info = xmldoc.getElementsByTagName('info')[0]
    common = xmldoc.getElementsByTagName('common')[0]

    data = {}
    data['name'] = sys.argv[3]
    data['size'] = int(info.getAttribute('size'))
    data['bold'] = bool(int(info.getAttribute('bold')))
    data['italic'] = bool(int(info.getAttribute('italic')))
    data['width'] = int(common.getAttribute('scaleW'))
    data['height'] = int(common.getAttribute('scaleH'))
    data['material'] = sys.argv[2]
    data['characters'] = {}
    characters = data['characters']

    for node in itemlist:
        char_num = int(node.getAttribute('id'))
        char = chr(char_num)
        characters[char] = {}
        current_char = characters[char]
        current_char['x'] = int(node.getAttribute('x'))
        current_char['y'] = int(node.getAttribute('y'))
        current_char['width'] = int(node.getAttribute('width'))
        current_char['height'] = int(node.getAttribute('height'))
        current_char['originX'] = -int(node.getAttribute('xoffset'))
        current_char['originY'] = -int(node.getAttribute('yoffset'))
        current_char['advance'] = int(node.getAttribute('xadvance'))

    with open(sys.argv[3] + '.font', 'w') as outfile:
        json.dump(data, outfile, indent=4, ensure_ascii=False)


if __name__ == '__main__':
    convert_font()

#!/usr/bin/env python
# -*- coding: utf-8 -*-
import re
import glob
import os
import sys
import codecs
import json
from collections import OrderedDict


def get_translation_text_list(filename):
    return p.findall(codecs.open(filename, 'r', 'utf-8').read())


if __name__ == '__main__':
    text_dict = OrderedDict()
    p = re.compile(r'["]((?:[^"\\\n]|\\.|\\\n)*)["][ \t\n]*_i18n')
    for path in ['../drivers', '../gui', '../libretro']:
        for x in os.walk(path):
            for y in glob.glob(os.path.join(x[0], '*.c*')):
                if os.path.splitext(y)[1] in ['.c', '.cpp']:
                    text_dict.update(OrderedDict.fromkeys(get_translation_text_list(y)))

    for v in text_dict.keys():
        text_dict[v] = v
    if len(sys.argv) < 2:
        filename = 'en-US'
    else:
        filename = sys.argv[1]
    with open('../../lang/' + filename + '.json', 'w') as f:
        json.dump(text_dict, f, indent=4)

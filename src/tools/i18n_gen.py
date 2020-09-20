#!/usr/bin/env python
# -*- coding: utf-8 -*-
import re
import glob
import os
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

    for x in os.walk('../../lang'):
        for y in glob.glob(os.path.join(x[0], '*.json')):
            tr_dict = text_dict.copy()
            depr_dict = OrderedDict()
            with codecs.open(y, 'r', 'utf-8') as f:
                org_dict = OrderedDict(json.load(f))
                for k, v in org_dict.items():
                    if k in tr_dict:
                        tr_dict[k] = v
                    elif k[-12:] == '(deprecated)':
                        depr_dict[k] = v
                    else:
                        depr_dict['__' + k + '__(deprecated)'] = v
            tr_dict.update(depr_dict)
            with codecs.open(y, 'w', 'utf-8') as f:
                json.dump(tr_dict, f, indent=4, ensure_ascii=False)
    with codecs.open('../../lang/en-US.json', 'w', 'utf-8') as f:
        json.dump(text_dict, f, indent=4, ensure_ascii=False)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# filename: util.py
#
# 通用函数库
#

import hashlib


def to_bytes(data):
    if isinstance(data, bytes):
        return data
    elif isinstance(data, (str, int, float)):
        return str(data).encode("utf-8")
    else:
        raise TypeError

MD5 = lambda data: hashlib.md5(to_bytes(data)).hexdigest()


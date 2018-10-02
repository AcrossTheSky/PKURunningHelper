#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# filename: loginUrlSign.py
#
# loginUrlSign 函数演示
#

from util import MD5


def login_url_sign(path, dateline, strAry):
    return MD5("raowenyuan{path}joy{timestamp}the{keys}run".format(
        path = path[:path.index('.')].lower() if '.' in path else path.lower(),
        timestamp = str(dateline),
        keys = "".join(map(str, strAry)).lower(),
    )).lower()

def upload_signature(path, dateline, lasttime, second, meter, **kwargs):
    return login_url_sign(path, dateline, [lasttime, second, meter])


def main():
    path = "po.aspx"
    dateline = 1538284879
    lasttime = 1538284877
    second = 1298
    meter = 1546

    signature = "e4b8e9359e86247954f831cea60abc75"

    sn = upload_signature(path, dateline, lasttime, second, meter)

    print(sn, sn == signature)


if __name__ == '__main__':
    main()
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# filename: getSignature.py
#
# getSignature 函数演示
#

from util import MD5


def __get_signature(params, uid, sid, salt):
    if not uid: # uid == 0 or ''
        uid = sid = ''
    return MD5("{paramsString}{salt}{uid}{sid}".format(
            paramsString = "".join("".join((k, str(v))) for k, v in sorted(params.items())),
            salt = salt,
            uid = str(uid),
            sid = sid,
        )).upper()

def get_signature_v1(params, uid=0, sid=''):
    return __get_signature(params, uid, sid, "1fd6e28fd158406995f77727b35bf20a")

def get_signature_v2(params, uid=0, sid=''):
    return __get_signature(params, uid, sid, "0C077B1E70F5FDDE6F497C1315687F9C")


def main():
    params = {
        "touid": 87808183,
        "option": "info",
        "timestamp": 1538184324,
    }

    uid = 87808183
    sid = "c96db474470a06b3335490d2331d5f5d"

    signatureV1 = "225F10B94560F337EA27F1BFA576ACC0"
    signatureV2 = "098F5D924BC82C040B26A8F05F75A22D"

    sn1 = get_signature_v1(params, uid, sid)
    sn2 = get_signature_v2(params, uid, sid)

    print(sn1, sn1 == signatureV1)
    print(sn2, sn2 == signatureV2)


if __name__ == '__main__':
    main()
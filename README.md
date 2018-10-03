# PKURunningHelper

这是一个慢性腰痛但是开不出证明的苦逼大学生为了能够毕业而写的小工具，用于伪造生成和上传跑步记录，目前支持 `PB`, `PKU Runner`, `悦跑圈 Joyrun`

该项目改写自学长的项目 [PKULayer](https://github.com/tegusi/PKULayer)


## 依赖环境

该项目目前仅支持 `Python 3`
```
$ apt-get install python3.6
```

安装依赖包 `requests`, `requests_toolbelt`
```
$ pip3 install requests requests_toolbelt
```

可选择安装 `simplejson`
```
$ pip3 install simplejson
```


## 下载

下载这个分支到本地
```
$ git clone https://github.com/zhongxinghong/PKURunningHelper
```


## 用法

进入项目根目录
```console
$ cd PKURunningHelper/
```

首先根据内部提示，修改配置文件 `config.ini`。修改 `base/app` 字段和相应 app 配置
```console
$ vim config.ini
```

运行 `runner.py` 查看命令行界面，输入参数 `--help` 查看用法
```console
$ python3 runner.py --help

Usage: runner.py [options]

PKU running helper! Check your config first, then enjoy yourself!

Options:
  -h, --help   show this help message and exit
  -c, --check  show 'config.ini' file
  -s, --start  start uploading job with Joyrun Client
```

输入参数 `--check` 检查配置文件的解析情况
```console
$ python3 runner.py --check

-- Using PB Client [2018.07.26] --
-- Section [Base]
{
    "app": "PB",
    "debug": "true"
}
-- Section [PB]
{
    "studentid": "1x000xxxxx",
    "password": "1x000xxxxx",
    "distance": "1.20",
    "pace": "4.50",
    "stride_frequncy": "160"
}
```

确保配置文件书写，然后输入 `--start`，即可完成一次上传。默认为 `debug` 模式，因此还会输出每次请求返回的 json 数据包
```console
$ python3 runner.py --start

[INFO] runner, 2018-09-28 18:08:55, upload record success !
```


## 文件夹结构

```
PKURunningHelper/
├── Joyrun                    // Joyrun 客户端程序包
│   ├── __init__.py
│   ├── auth.py                   // 请求鉴权类
│   ├── client.py                 // Joyrun 客户端程序包
│   ├── data                  // 跑步记录数据
│   │   └── 400m.250p.54.joyrun.json       // 由 PKURunner 的跑步数据经过校正得到
│   ├── error.py                  // 错误类定义
│   └── record.py                 // 跑步记录类
├── LICENSE
├── PB                        // PB 客户端程序包
│   ├── __init__.py
│   ├── client.py                 // PB 客户端类，伪造 HTTP 请求，生成 Running Record
│   ├── data                  // 跑步记录数据
│   │   └── 400m.locus.json       // 五四跑廊一圈 GPS 数据，抽取和修改自我曾经的跑步记录
│   └── error.py                  // 错误类定义
├── PKURunner                 // PKURunner 客户端程序包
│   ├── __init__.py
│   ├── client.py                 // PKURunner 客户端类
│   ├── data                  // 跑步记录数据
│   │   └── 400m.250p.54.pkurunner.json    // 五四跑廊走一圈的高德 GPS 数据，共 250 点，坐标手动校正过
│   ├── error.py                  // 错误类定义
│   ├── iaaa.py                   // 北大 IAAA 统一认证客户端类
│   └── record.py                 // 跑步记录类
├── README.md
├── cache                     // 缓存文件夹
│   ├── Joyrun_LoginInfo.json         // Joyrun 的登录状态缓存
│   └── PKURunner_AccessToken.json    // PKURunner 的 IAAA 认证所得 token 缓存
├── docs                      // 一些文档，记录了自己的破解思路
│   ├── Joyrun.cracked.md         // Joyrun 破解文档
│   ├── PB.cracked.md             // PB 破解文档
│   ├── PKURunner.cracked.md      // PKURunner 破解文档
│   ├── code                      // 示例代码和关键源码
│   │   ├── Joyrun
│   │   │   ├── getSignature.py
│   │   │   ├── jni_native.c
│   │   │   ├── joytherun.cpp
│   │   │   ├── libjoyrun.so
│   │   │   ├── loginUrlSign.cpp
│   │   │   ├── loginUrlSign.py
│   │   │   ├── md5.cpp
│   │   │   ├── md5.h
│   │   │   └── util.py
│   │   └── PKURunner
│   │       └── a.smali
│   ├── images                // 文档插图
│   │   ├── Joyrun.Fiddler.headers.png
│   │   ├── Joyrun.Fiddler.webforms.png
│   │   ├── PKURunner.Android.cert.error.jpg
│   │   └── PKURunner.Charles.png
│   └── web                   // 参考网页
│       └── 【悦跑圈】app签名算法逆向 - sumousguo的专栏 - CSDN博客.pdf
├── config.ini                    // 项目配置文件
├── packets                   // Fiddler 抓包结果和一些 API 接口的返回数据包
│   ├── Joyrun
│   │   ├── code.401.json
│   │   ├── login.po.getInfo.saz
│   │   ├── po.247040913.json
│   │   ├── po.247040913.parse.json
│   │   ├── ......
│   │   ├── po.aspx.247294144.saz
│   │   ├── po.aspx.247616368.saz
│   │   ├── ......
│   │   ├── record.247616368.json
│   │   └── record.247616368.parse.json
│   ├── PB
│   │   ├── 11km.locus.json
│   │   ├── 309.gz
│   │   ├── ......
│   │   └── 867_Request.txt
│   └── PKURunner
│       ├── code.-1.json
│       ├── ......
│       ├── record.7.json
│       └── upload.json
├── requirements.txt
├── runner.py                     // 项目主程序
└── util                      // 通用程序包
    ├── __init__.py
    ├── class_.py                 // 通用类
    ├── func.py                   // 通用函数库
    └── module.py                 // 通用模板类，统一 import 导入结果
```


## 破解思路

写了两篇短文记录了一下自己的破解流程，详见项目 `docs/` 目录


## 免责声明

本项目仅供参考学习，你可以修改和使用这个项目，但请自行承担使用不当造成的一切后果


## 证书

[MIT LICENSE](https://github.com/zhongxinghong/PKURunningHelper/blob/master/LICENSE)
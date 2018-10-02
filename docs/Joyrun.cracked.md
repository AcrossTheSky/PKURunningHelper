# Joyrun 破解思路

悦跑圈有一套专门的机制来防止伪造发包，不仅设计了多种形式的鉴权算法，而且其最核心的鉴权算法以 C 语言写成，编译成 \*.so 文件，然后以 java 中 native 函数的形式进行调用，这是业内防范通过 Android 逆向工程破解 apk 的一种比较专业的措施。不过好在它没有在 SSL 证书上做太多的手脚，java 反编译上也没做什么防范，绝大多数 java 源码都可以顺利解出来，否则将极难破解！

这里我采用 `Fiddler/Charles 抓包 + Java 逆向 + C/C++ 逆向` 的联合手段对其 API 接口进行破解。


## APP 版本信息

破解版本 v4.2.0
发布时间 2018.09.26


## Android 逆向工具

首先是著名的 apk 反编译三件套，及著名的 c/c++ 反编译工具 IDA Pro 7，其中 IDA 需要破解，可以去找一个现成的破解版本。其他具体介绍就不多写了，网上遍地都是教程～

- [**Apktool** v2.3.4](https://github.com/iBotPeaches/Apktool)
- [**dex2jar** v2.1-SNAPSHOT](https://github.com/pxb1988/dex2jar)
- [**jd-gui** v1.4.0](https://github.com/java-decompiler/jd-gui)
- [**IDA Pro 7**](https://www.hex-rays.com/products/ida/index.shtml)


## Fiddler 初步抓包分析

从 Fiddler 抓包情况来看，悦跑圈对次网络请求都做了签名，并且在 payload 和 headers 内各有一个，分别是 `"signature"` 和 `"_sign"` 字段。

<img src="https://github.com/zhongxinghong/PKURunningHelper/raw/master/docs/images/Joyrun.Fiddler.headers.png" width="80%">
<img src="https://github.com/zhongxinghong/PKURunningHelper/raw/master/docs/images/Joyrun.Fiddler.webforms.png" width="80%">

jd-gui 中，通过搜索 "signature" 字段，可以找到如下代码：

```java
  package co.runner.app.model.repository.a;

  private RequestParams c(f paramf, String paramString, RequestParams paramRequestParams)
    throws Exception
  {
    if (((paramf == null) || (paramf.getUid() <= 0)) && (!p.a(paramString)))
    {
      ar.c("游客限制访问", paramString);
      throw new MyException("请登录");
    }
    paramString = paramRequestParams;
    if (paramRequestParams == null) {
      paramString = new RequestParams();
    }
    if (paramString.has("signature")) {
      return paramString;
    }
    boolean bool = paramString.has("timestamp");
    int k = 0;
    int i;
    if (bool) {
      i = Integer.parseInt(paramString.get("timestamp"));
    } else {
      i = 0;
    }
    if ((i == 0) && (Math.abs(System.currentTimeMillis() - g.a()) > 240000L)) {
      i();
    }
    String str = "";
    int j = k;
    paramRequestParams = str;
    if (paramf != null)
    {
      j = k;
      paramRequestParams = str;
      if (paramf.getUid() > 0)
      {
        j = paramf.getUid();
        paramRequestParams = paramf.getSid();
      }
    }
    k = i;
    if (i == 0) {
      k = g.b();
    }
    a(paramString, k, j, paramRequestParams);
    return paramString;
  }
```

向上查找 `a(paramString, k, j, paramRequestParams)` 的函数定义：

```java
  package co.runner.app.model.repository.a;

  public void a(RequestParams paramRequestParams, int paramInt1, int paramInt2, String paramString)
  {
    paramRequestParams.put("timestamp", paramInt1);
    String str = d.a(paramRequestParams.getUrlParams(), paramInt2, paramString);
    paramString = d.a(paramRequestParams.getUrlParams(), paramInt2, paramString);
    paramRequestParams.put("signature", str);
    paramRequestParams.put("signatureV2", paramString);
  }
```

向上查找两个 `d.a(paramRequestParams.getUrlParams(), paramInt2, paramString)` 函数的定义：

```java
  package co.runner.app.utils;

  public String a(Map<String, String> paramMap, int paramInt, String paramString)
  {
    Object localObject = new ArrayList(paramMap.keySet());
    Collections.sort((List)localObject);
    StringBuilder localStringBuilder = new StringBuilder();
    localObject = ((List)localObject).iterator();
    while (((Iterator)localObject).hasNext())
    {
      String str = (String)((Iterator)localObject).next();
      localStringBuilder.append(str);
      localStringBuilder.append((String)paramMap.get(str));
    }
    if (paramInt > 0) {
      return NativeToolImpl.a().getSignature(localStringBuilder.toString(), String.valueOf(paramInt), paramString).toUpperCase();
    }
    return NativeToolImpl.a().getSignature(localStringBuilder.toString(), null, null).toUpperCase();
  }

  public String a(ConcurrentHashMap<String, String> paramConcurrentHashMap, int paramInt, String paramString)
  {
    Object localObject = new ArrayList(paramConcurrentHashMap.keySet());
    Collections.sort((List)localObject);
    StringBuilder localStringBuilder = new StringBuilder();
    localObject = ((List)localObject).iterator();
    while (((Iterator)localObject).hasNext())
    {
      String str = (String)((Iterator)localObject).next();
      localStringBuilder.append(str);
      localStringBuilder.append((String)paramConcurrentHashMap.get(str));
    }
    if (paramInt > 0) {
      return NativeToolImpl.a().getSignatureV2(localStringBuilder.toString(), String.valueOf(paramInt), paramString).toUpperCase();
    }
    return NativeToolImpl.a().getSignatureV2(localStringBuilder.toString(), null, null).toUpperCase();
  }
```

向上查找两个 `getSignature` 和 `getSignatureV2` 函数的定义：

```java
  package co.runner.app.jni;

  public native String getSignature(String paramString1, String paramString2, String paramString3);

  public native String getSignatureV2(String paramString1, String paramString2, String paramString3);
```

最后找到 JNI 框架里两个 native 函数 ...

到了 native 函数层就比较麻烦了，我们暂且不继续分析下去，而是先分析一下现在解出的代码。

凭借以前我做前端开发的经验，这几段代码其实是在对所有的网络请求统一做时间戳和签名的添加。对一个大型前端项目来说，如果对每一个网络请求都手动添加相应的鉴权字段，那将是很繁琐的，而且事后难以统一修改，因此需要专门定义一个鉴权类来统一处理所有网络请求的鉴权问题。

首先来看一下 `package co.runner.app.model.repository.a` 的 `c` 函数（第一段代码），注意以下这段代码，及函数返回值字段：

```java
  package co.runner.app.model.repository.a;

  private RequestParams c(f paramf, String paramString, RequestParams paramRequestParams)
    throws Exception
  {
    // ......

    paramString = paramRequestParams;
    if (paramRequestParams == null) {
      paramString = new RequestParams();
    }
    if (paramString.has("signature")) {
      return paramString;
    }

    // ......

    a(paramString, k, j, paramRequestParams);
    return paramString;
  }
```

意义很明显，这实际上就是为了在传入的 RequestParams 类的 paramRequestParams 实例中添加一个 `"signature"` 字段，如果之前已经构造过了，那就不需要再构造了，否则准备好几个关键参数，通过 `a(paramString, k, j, paramRequestParams)` 进行构造，然后再将其返回。

在这里首先分析一下传入的四个参数的含义。首先是 `paramString` ，这个很明显是传入的待添加签名字段的 paramRequestParams 。`k` 对应着 `i` ，显然应该是时间戳字段 `timestamp` ，也可以顺着 `k = g.b()` 语句向上查找，可以找到下面的代码，最终确定了是一个以秒为单位的 Unix 时间戳。

```java
package co.runner.app.model.helper;

public class g
{
  // ......

  private static int i = (int)(System.currentTimeMillis() / 1000L);

  // ......

  public static int b()
  {
    return i;
  }

  // ......
}
```

然后是 `j` 和 `paramRequestParams` ，从以下代码可以分析出，这两个参数分别代表 `uid`, `sid` ，即 `User ID` 和 `Session ID` ，实际上描述着登录状态。如果之前这两个字段没有缓存，也就是没有登录状态的记录，则 uid 将为 `0` ，sid 将为 `""` 。

```java
  package co.runner.app.model.repository.a;

  private RequestParams c(f paramf, String paramString, RequestParams paramRequestParams)
    throws Exception
  {
    // ......

    int k = 0;

    // ......

    String str = "";
    int j = k;
    paramRequestParams = str;
    if (paramf != null)
    {
      j = k;
      paramRequestParams = str;
      if (paramf.getUid() > 0)
      {
        j = paramf.getUid();
        paramRequestParams = paramf.getSid();
      }
    }

    // ......
  }
```

至此，弄清楚了四个参数的含义。继续看 `a(paramString, k, j, paramRequestParams)` 的定义（第二段代码），意义很明显，将时间戳添加到 payload 中，然后将剩余参数分别传给两个鉴权函数生成两个版本的签名，并分别存为 `"signature"` 和 `"signatureV2"` 字段。

最后看一下两个鉴权函数的前调用处理（第三段代码）。两个函数用完全相同的方法对 payload 的键值对进行拼接，很容易看出拼接形式为 `"{key1}{value1}{key2}{value3}...{keyN}{valueN}"` 并且 key 按照字典序排列。这个字符串即为两个 native 函数的 paramString1 ，剩下两个参数很显然，分别是 uid 和 sid ，并且他们的缺省值均为 null 。

现在问题的关键是两个 native 函数里究竟写了什么！


## 破解 getSignature 函数

两个月前我曾经尝试过破解悦跑圈的鉴权算法，解到这一步时因为不懂得反编译 \*.so 文件，就放弃了。最近我偶然看到一篇 blog ，发现竟然有人专门破解了这个鉴权算法！ 见 [【悦跑圈】app签名算法逆向](https://blog.csdn.net/sumousguo/article/details/82813948) 。

根据这篇 blog 的思路，必须要尝试反编译 \*.so 文件。

运行 `./apktool d -r -f` 命令，直接解压 apk 文件，这样可以解出 `lib` 文件夹。

```console
debian-9:/home/pyprj/apk_decompile/decompile-apk-v1.0/tools/apktool# ./apktool d -r -f ../../../co.runner.app.apk
I: Using Apktool 2.3.4 on co.runner.app.apk
I: Copying raw resources...
I: Baksmaling classes.dex...
I: Baksmaling classes2.dex...
I: Baksmaling classes3.dex...
I: Baksmaling classes4.dex...
I: Baksmaling assets/hackload.dex...
I: Copying assets and libs...
I: Copying unknown files...
I: Copying original files...
```

然后进入 `lib/armeabi-v7a/` ，用 `grep` 命令搜索 getSignature 函数，可以锁定目标文件为 `libjoyrun.so` 。

```console
debian-9:/home/pyprj/apk_decompile/decompile-apk-v1.0/tools/apktool# cd co.runner.app/lib/armeabi-v7a/
debian-9:/home/pyprj/apk_decompile/decompile-apk-v1.0/tools/apktool/co.runner.app/lib/armeabi-v7a# grep getSignature ./*
Binary file ./libjoyrun.so matches
```

接下来在 Windows 下安装 IDA Pro 7 ，反编译这个 \*.so 文件，可以解出以下代码：

```c
int __fastcall Java_co_runner_app_jni_NativeToolImpl_getSignature(int a1, int a2, int a3, int a4, int a5)
{
  return getSign(a1, a5, a3, a4, a5, "1fd6e28fd158406995f77727b35bf20a");
}

int __fastcall Java_co_runner_app_jni_NativeToolImpl_getSignatureV2(int a1, int a2, int a3, int a4, int a5)
{
  return getSign(a1, a5, a3, a4, a5, "0C077B1E70F5FDDE6F497C1315687F9C");
}

int __fastcall getSign(int a1, int a2, int a3, int a4, int a5, const char *a6)
{
  return getSign((_DWORD *)a1, a2, a3, a4, a5, a6);
}

int __fastcall getSign(_DWORD *a1, int a2, int a3, int a4, int a5, const char *a6)
{
  _DWORD *v6; // r4
  int v7; // r0
  int v8; // r9
  const char *v9; // r11
  size_t v10; // r5
  size_t v11; // r6
  int v12; // r10
  int v13; // r8
  int v14; // r6
  const char *v15; // r9
  int v16; // ST04_4
  const char *v17; // r10
  size_t v18; // r5
  int v19; // r8
  size_t v20; // r6
  char *v21; // r5
  int v22; // r3
  _DWORD *v23; // r0
  int v24; // r1
  int v25; // r2
  char *v27; // r5

  v6 = a1;
  v7 = *a1;
  v8 = a4;
  if ( dword_911E0 != 1 )
    return (*(int (__fastcall **)(_DWORD *, const char *))(v7 + 668))(v6, "error");
  v9 = (const char *)(*(int (__fastcall **)(_DWORD *, int, _DWORD))(v7 + 676))(v6, a3, 0);
  v10 = strlen(v9);
  v11 = strlen(a6);
  v12 = (*(int (__fastcall **)(_DWORD *, const char *))(*v6 + 24))(v6, "co/runner/app/jni/MD5");
  v13 = v10 + v11 + 1;
  v14 = (*(int (__fastcall **)(_DWORD *, int, const char *, const char *))(*v6 + 452))(
          v6,
          v12,
          "md5",
          "(Ljava/lang/String;)Ljava/lang/String;");
  if ( v8 )
  {
    v15 = (const char *)(*(int (__fastcall **)(_DWORD *, int, _DWORD))(*v6 + 676))(v6, v8, 0);
    v16 = v12;
    v17 = (const char *)(*(int (__fastcall **)(_DWORD *, int, _DWORD))(*v6 + 676))(v6, a5, 0);
    v18 = strlen(v15) + v13;
    v19 = v14;
    v20 = v18 + strlen(v17);
    v21 = (char *)malloc(v20);
    _aeabi_memclr();
    strcpy(v21, v9);
    strcat(v21, a6);
    strcat(v21, v15);
    strcat(v21, v17);
    v22 = (*(int (__fastcall **)(_DWORD *, char *))(*v6 + 668))(v6, v21);
    v23 = v6;
    v24 = v16;
    v25 = v19;
  }
  else
  {
    v27 = (char *)malloc(v13);
    _aeabi_memclr();
    strcpy(v27, v9);
    strcat(v27, a6);
    v22 = (*(int (__fastcall **)(_DWORD *, char *))(*v6 + 668))(v6, v27);
    v23 = v6;
    v24 = v12;
    v25 = v14;
  }
  return _JNIEnv::CallStaticObjectMethod(v23, v24, v25, v22);
}
```

解到这里，一切都很清楚了。两个 getSignature 的函数定义其实是一样的，只是用的 salt 不一样而已。在字符串拼接上还分两种情况，如果有 uid 和 sid，则形式为 `"{payloadKeyValueString}{salt}{uid}{sid}"` 否则为 `"{payloadKeyValueString}{salt}"` ，然后调用 java 层的 "co/runner/app/jni/MD5" 类中的 "md5" 方法，来计算这个字符串的 MD5 值。

综上可以得出两个 getSignature 函数的 python 描述：

```python
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
```

至此，还有一个小问题需要考虑： 明明同时构造了两个 "signature" ，为什么 "signatureV2" 后来就不见了？headers 中的 "\_sign" 字段是什么？事实上这个问题很简单。jd-gui 内搜索 "\_sign"　字段，很容易找到如下代码：

```java
  package co.runner.app.model.helper;

  public String a(f paramf, String paramString, RequestParams paramRequestParams)
    throws Exception
  {
    // ......

    paramRequestParams = ((RequestParams)localObject).get("signatureV2");
    ((RequestParams)localObject).remove("signatureV2");

    // ......

    if (paramRequestParams != null) {
      localBuilder.addHeader("_sign", paramRequestParams);
    }
    return a(localBuilder.build(), m);
  }
```

所以 headers 中的 "\_sign" 字段实际上就是 "signatureV2" 。

至此，两个关键签名算法被破解。


## 服务器接入与 cookies 设置

拿到两个签名算法后，很顺利地模拟登录了悦跑圈，并拿到了 sid 和 uid 值。然后需要稍微完善一下 headers 中的 cookies 字段。构造的方法很简单，见如下源码：

```java
  package co.runner.app.model.helper;

  public String a(f paramf, String paramString, RequestParams paramRequestParams)
    throws Exception
  {
    // ......

    localObject = a(paramf);
    if (localObject != null)
    {
      StringBuilder localStringBuilder = new StringBuilder();
      localStringBuilder.append("ypcookie=");
      localStringBuilder.append((String)localObject);
      localBuilder.header("Cookie", localStringBuilder.toString());
      localBuilder.addHeader("ypcookie", b(paramf));
    }

    // ......
  }

  public String a(f paramf)
  {
    if ((paramf != null) && (paramf.getUid() > 0)) {
      try
      {
        paramf = URLEncoder.encode(String.format("sid=%s&uid=%s", new Object[] { paramf.getSid(), Integer.valueOf(paramf.getUid()) }), "utf-8").toLowerCase();
        return paramf;
      }
      catch (UnsupportedEncodingException paramf)
      {
        paramf.printStackTrace();
      }
    }
    return null;
  }

  public String b(f paramf)
  {
    if ((paramf != null) && (paramf.getUid() > 0)) {
      return String.format("sid=%s&uid=%s", new Object[] { paramf.getSid(), Integer.valueOf(paramf.getUid()) });
    }
    return null;
  }
```

再结合抓包结果，可以得到如下的 python 描述：

```python
from urllib.parse import quote

loginCookie = "sid=%s&uid=%s" % (sid, uid)
session.headers.update({"ypcookie": loginCookie})
session.cookies.clear()
session.cookies.set("ypcookie", quote(loginCookie).lower())
```

之后，便可以通过 cookies 字段携带登录状态，去一个个测试 Joyrun 的 API 接口啦～


## 上传接口的鉴权算法

解到这一步以后，80% 的悦跑圈 API 接口都可以正常请求了，现在可以考虑一下怎样模拟请求记录上传接口了。

首先通过 Fiddler 抓包，可以确定上传接口 path 为 `"/po.aspx" ` ，上传数据包的格式如下：

```json
{
    "altitude": "[37.69,37.69,...,36.25,36.53]",
    "private": "0",
    "dateline": "1538395627",
    "city": "北京",
    "signature": "63381A84F09D9B77C9FA6F65A8A1AC3C",
    "sign": "73a28dd93c39064108084feca94f7d93",
    "starttime": "1538385182",
    "type": "1",
    "content": "[39985957,116301648]-[39985957,116301648]-...-[39986760,116301090]",
    "second": "3293",
    "stepcontent": "[[0,0.3],[15,16.5],...,[0,0.21]]",
    "province": "北京市",
    "stepremark": "[[0,1,16],[55,58,24],...,[633,658,78]]",
    "runid": "dd79bdb77cec4887969480318134a996",
    "sampleinterval": "5",
    "timestamp": "1538395627",
    "wgs": "1",
    "nomoment": "1",
    "meter": "4124",
    "heartrate": "[]",
    "totalsteps": "6166",
    "nodetime": "[1000,727,39988473,116302405,146]-...-[4000,3104,39986834,116301044,621]",
    "lasttime": "1538388480",
    "pausetime": "",
    "timeDistance": "[0,16,16,20,...,4124,4124]"
}
```

然后不幸地发现，多了一个 `"sign"` 字段 ... 还有一个 `"runid"` 字段也用了 hash 值来描述。

不得不吐槽悦跑圈的签名算法真多 ...


### "runid" 字段的构造

庆幸的是，`"runid"` 字段的构造方法很简单，几经查找，最终确定的生成方法如下：

```java
  package co.runner.app.utils;

  public static String a()
  {
    return UUID.randomUUID().toString().replace("-", "");
  }
```

这是在不知道记录 fid 的情况下 runid 的构造方法，实际上就是一个随机的 UUID 值。

不过，紧接着还能看到第二种构造方法：

```java
  package co.runner.app.utils;

  public static String a(String paramString, int paramInt1, int paramInt2, int paramInt3)
  {
    return bs.a(String.format("from:%s,id:%s,distance:%s,second:%s", new Object[] { paramString, Integer.valueOf(paramInt1), Integer.valueOf(paramInt2), Integer.valueOf(paramInt3) })).toLowerCase();
  }


  package co.runner.app.utils;

  public static String a(String paramString)
  {
    if (TextUtils.isEmpty(paramString)) {
      return "";
    }
    try
    {
      MessageDigest localMessageDigest = MessageDigest.getInstance("MD5");
      localMessageDigest.update(paramString.getBytes());
      paramString = a(localMessageDigest.digest());
      return paramString;
    }
    catch (NoSuchAlgorithmException paramString)
    {
      ar.a(paramString);
    }
    return "";
  }
```

其调用函数为：

```java
  package co.runner.app.watch.ui;

  private DataInfo a(int paramInt, RunRecord paramRunRecord)
  {
    // ......

    localDataInfo.infoid = paramRunRecord.fid;
    localDataInfo.meter = paramRunRecord.meter;
    localDataInfo.second = paramRunRecord.second;

    // ......

    localDataInfo.from = "joyrun";
    localDataInfo.runid = cd.a(localDataInfo.from, localDataInfo.infoid, localDataInfo.meter, localDataInfo.second);

    // ......
  }

```

最终可以确定的 python 描述为：

```python
runid = MD5("from:{},id:{},distance:{},second:{}".format("joyrun", fid, distance, second)).lower()
```

但是随后各种带入尝试都失败了 ...

其实可以做出如下分析： 根据抓包结果可知，fid 实际上是后端服务器分配的，在记录上传前，手机端并不知道 fid 是多少，所以上传记录时的 "runid" 字段并不应该基于这个算法构建。

所以最终利用随机的 UUID 对 "runid" 进行构建，从之后的模拟上传的结果来看，这样做完全是可行的。


### "sign" 字段的构造

现在的最关键的事情是要弄清楚 `"sign"` 字段的构造方法。jd-gui 内搜索 "po.aspx" ，可以找到如下代码：

```java
  package co.runner.app.model.repository.a;

  public Observable<JSONObject> a(RunRecord paramRunRecord)
  {
    return a(paramRunRecord, "po.aspx", 0, 0);
  }
```

向上查找 `a(paramRunRecord, "po.aspx", 0, 0)` 函数的定义：

```java
  package co.runner.app.model.repository.a;

  private Observable<JSONObject> a(final RunRecord paramRunRecord, final String paramString, final int paramInt1, final int paramInt2)
  {
      // ......

      int i;
      if (paramRunRecord.proofreadMeter > 0) {
        i = paramRunRecord.proofreadMeter;
      } else {
        i = paramRunRecord.meter;
      }

      // ......

      localRequestParams.put("meter", i);

      // ......

      localRequestParams.put("dateline", j);

      // ......

      if (paramInt1 > 0)
      {
        localRequestParams.put("fid", paramInt1);
        localRequestParams.put("sign", b.d.a("dataImportWithFid", Integer.valueOf(j), new String[] { String.valueOf(l.this.b.getUid()), String.valueOf(paramInt1) }));
      }
      else
      {
        localRequestParams.put("sign", b.d.a(paramString, Integer.valueOf(j), new String[] { String.valueOf(paramRunRecord.getLasttime()), String.valueOf(paramRunRecord.getSecond()), String.valueOf(i) }));
      }

      // ......
  }
```

考虑 `paramInt == 0` 的情况，向上查找 `b.d.a` 函数的定义：

```java
  package co.runner.app.utils;

  public String a(String paramString, Object paramObject, String... paramVarArgs)
  {
    boolean bool = paramString.contains(".");
    int i = 0;
    String str = paramString;
    if (bool) {
      str = paramString.substring(0, paramString.indexOf("."));
    }
    paramString = new StringBuilder();
    int j = paramVarArgs.length;
    while (i < j)
    {
      paramString.append(paramVarArgs[i]);
      i++;
    }
    return NativeToolImpl.a().loginUrlSign(str.toLowerCase(), paramObject.toString(), paramString.toString().toLowerCase()).toLowerCase();
  }
```

没什么悬念了，又得破解一个 native 函数了 ...

```java
  package co.runner.app.jni;

  public native String loginUrlSign(String paramString1, String paramString2, String paramString3);
```

老惯例吧，在破解之前，首先还是要确定一下传入参数的含义。从函数名 `loginUrlSign` 可以看出，我们需要携带请求路径与 payload 一同签名，也就是最初调用时传入的 `"po.aspx"`。还会发现，签名前首先还去掉了 ".aspx" 后缀以避免不必要的麻烦，这些都很好理解。这样，native 函数的 `paramsString1` 表示去后缀名的请求路径。`paramsString2` 很显然是 `"dateline"` 字段的值。`paramsString3` 是由跑步记录中的 `"lasttime", "second", "meter"` 字段拼接而来，即 `"{lasttime}{second}{meter}"` 。

接下来又要出动 IDA 了。同样地思路可以从 `libjoyrun.so` 里解出如下代码：

```c
int __fastcall Java_co_runner_app_jni_NativeToolImpl_loginUrlSign(_DWORD *a1, int a2, int a3, int a4, int a5)
{
  _DWORD *v5; // r4
  int v6; // r0
  int v7; // r5
  const char *v8; // r10
  const char *v9; // r9
  const char *v10; // r8
  size_t v11; // r5
  size_t v12; // r5
  size_t v13; // r0
  char *v14; // r5
  int v15; // r9
  int v16; // r8
  int v17; // r0

  v5 = a1;
  v6 = *a1;
  v7 = a4;
  if ( dword_911E0 != 1 )
    return (*(int (__fastcall **)(_DWORD *, const char *))(v6 + 668))(v5, "error");
  v8 = (const char *)(*(int (__fastcall **)(_DWORD *, int, _DWORD))(v6 + 676))(v5, a3, 0);
  v9 = (const char *)(*(int (__fastcall **)(_DWORD *, int, _DWORD))(*v5 + 676))(v5, v7, 0);
  v10 = (const char *)(*(int (__fastcall **)(_DWORD *, int, _DWORD))(*v5 + 676))(v5, a5, 0);
  v11 = strlen(v8);
  v12 = v11 + strlen(v9);
  v13 = strlen(v10);
  v14 = (char *)malloc(v13 + v12 + 20);
  _aeabi_memcpy(v14, "raowenyuan", 11);
  strcat(v14, v8);
  *(_DWORD *)&v14[strlen(v14)] = 7958378;
  strcat(v14, v9);
  *(_DWORD *)&v14[strlen(v14)] = 6645876;
  strcat(v14, v10);
  *(_DWORD *)&v14[strlen(v14)] = 7239026;
  v15 = (*(int (__fastcall **)(_DWORD *, const char *))(*v5 + 24))(v5, "co/runner/app/jni/MD5");
  v16 = (*(int (__fastcall **)(_DWORD *, int, const char *, const char *))(*v5 + 452))(
          v5,
          v15,
          "md5",
          "(Ljava/lang/String;)Ljava/lang/String;");
  v17 = (*(int (__fastcall **)(_DWORD *, char *))(*v5 + 668))(v5, v14);
  return _JNIEnv::CallStaticObjectMethod((int)v5, v15, v16, v17);
}
```

这个函数和之前的函数 getSignature 函数的定义非常相似，但是有一个很关键的区别：

```c
  v14 = (char *)malloc(v13 + v12 + 20);
  _aeabi_memcpy(v14, "raowenyuan", 11);
  strcat(v14, v8);
  *(_DWORD *)&v14[strlen(v14)] = 7958378;
  strcat(v14, v9);
  *(_DWORD *)&v14[strlen(v14)] = 6645876;
  strcat(v14, v10);
  *(_DWORD *)&v14[strlen(v14)] = 7239026;
```

其中最关键的是三个赋值语句！ 究竟 **\_DWORD** 是什么？ 为什么要给一个 char 赋一个这么大的值？

由于我对汇编和指针的内容不是很了解，一开始我相信了网上的这个解释：

```c
v7 = *(_DWORD *)(v7 + 8);

Means :

v7 = *(v7 + 8)
```

那么三个赋值语句就将变成了：

```c
  v14[strlen(v14)] = 7958378;
  v14[strlen(v14)] = 6645876;
  v14[strlen(v14)] = 7239026;
```

这不就是 char 赋值溢出嘛，用 cpp 模拟一下就知道结果了：

```cpp
  cout << char(7958378) << " "
       << char(6645876) << " "
       << char(7239026) << endl;
```

输出结果非常明确：

```console
debian-9:/home/pyprj/pku_running_helper/test/Joyrun# g++ test.cpp
debian-9:/home/pyprj/pku_running_helper/test/Joyrun# ./a.out
j t r
```

所以字符串的拼接方式应该是 `"raowenyuan{str1}j{str2}t{str3}r"` 。

结果 MD5 打死也算不对 ...

显然有一件事情不能很好解释，给 v14 动态分配内存时，多预留了 20 个 char 空位，去掉 "raowenyuan" 和 最后的 '\0' 还剩下 9 个 char 空位，如果仅仅给其中的 3 个 char 赋值，那还有 6 个 char 没有被使用，为什么要故意开大数组呢？也许这预示着目前这种拼接方法是不对。

可是这件事怎么也想不明白，结果就这么卡了一整天 ...

......


后来我才明白 "\_DWORD" 在汇编里表示双字，占 32 bits，所以三段赋值代码的意思是，首先将一个指向 8 bits 的 char 类型指针通过强制类型转换，变成一个指向 32 bits 的指针，然后通过赋值，一次性修改 4 个 char ！

想到这儿，赶紧就写出下面这段代码：

```cpp
  char * str = new char[4];

  *(int *)str = 7958378;
  cout << str << " " << strlen(str) << endl;
  *(int *)str = 6645876;
  cout << str << " " << strlen(str) << endl;
  *(int *)str = 7239026;
  cout << str << " " << strlen(str) << endl;

  delete [] str;
```

一运行，傻眼了 ...

```console
debian-9:/home/pyprj/pku_running_helper/test/Joyrun# g++ test.cpp
debian-9:/home/pyprj/pku_running_helper/test/Joyrun# ./a.out
joy 3
the 3
run 3
```

这波操作也太骚了吧！ orz ...

刚好一次影响 3 个 char ，刚好用完 20 个空位！

（于是赶紧百度 "raowenyuan" 膜拜了一下 orz ...）

所以正确的拼接方式是 `"raowenyuan{str1}joy{str2}the{str3}run"` ，这就得到了如下的 python 描述：

```python
def login_url_sign(path, dateline, strAry):
    return MD5("raowenyuan{path}joy{timestamp}the{keys}run".format(
        path = path[:path.index('.')].lower() if '.' in path else path.lower(),
        timestamp = str(dateline),
        keys = "".join(map(str, strAry)).lower(),
    )).lower()

def upload_signature(path, dateline, lasttime, second, meter, **kwargs):
    return login_url_sign(path, dateline, [lasttime, second, meter])
```

经过检验后终于看到了想要的结果 ...

至此，最麻烦的一个签名算法得到了破解。


## 数据包模拟

连续破解了三道鉴权算法后，剩下的工作就相对比较简单了，考验的只是耐心和细心。具体就不想多谈了。借助抓包分析、java 源码分析、猜测、检验，最终确定了 payload 字段中所有字段的含义，详细可见 `Joyrun/record.py` 中的 Record 类。

当预示着上传成功的返回包发回来的时候，整个破解工作宣告完美结束！

```console
debian-9:/home/pyprj/pku_running_helper# ./runner.py --start
[DEBUG] joyrun, 2018-10-02 21:24:55, request.url = https://api.thejoyrun.com/po.aspx
[DEBUG] joyrun, 2018-10-02 21:24:55, response.json = {
    "fid": 247874981,
    "postRunId": 237561442,
    "ret": "0",
    "msg": "发布成功",
    "sid": "00c343ed55f025ccead01434293aab2f",
    "fraud": "0",
    "lasttime": 1538486695,
    "weixinurl": "http://wap.thejoyrun.com/po_87808183_247874981.html",
    "timeConflict": 0,
    "multipleUpload": false
}
[INFO] runner, 2018-10-02 21:24:55, upload record success !
```

# PKURunner 破解思路

PKURunner 最麻烦的地方在于，学校服务器似乎对 SSL 证书做了限制（也有可能是学校网关的问题)，Charles 和 Fiddler 的 CA 证书不被信任，看不到任何抓包结果，无法采用传统的中间人攻击 (MITM) 的思路破解。所以这整个项目没有借助任何抓包数据，完全是依据 apk 逆向分析结果和老项目 [PKULayer][PKULayer] 的发包思路来构建的。


## APP 版本信息

破解版本 v1.2.1
发布时间 2018.09.25


## 关于 SSL 证书问题

- **Charles** 抓包会看到如下结果，Android 客户端显示证书错误，同时 Charles 没能解出 https 的传输密文。

<img src="https://github.com/zhongxinghong/PKURunningHelper/raw/master/docs/images/PKURunner.Charles">
<img src="https://github.com/zhongxinghong/PKURunningHelper/raw/master/docs/images/PKURunner.Android.cert.error" width="40%">

- **Fiddler** 抓包会看不到 https 传输数据，Android 客户端同样显示证书错误。

- **Wireshark** 抓包，因为拿不到 SSL-key 的信息，所以只能看到密文。


## Android 逆向工具

著名的 apk 反编译三件套放上来，具体介绍就不多写了，网上遍地都是教程～

- [**Apktool** v2.3.4](https://github.com/iBotPeaches/Apktool)
- [**dex2jar** v2.1-SNAPSHOT](https://github.com/pxb1988/dex2jar)
- [**jd-gui** v1.4.0](https://github.com/java-decompiler/jd-gui)


## 破解 IAAA 认证

显然，首先先得想办法模拟登录北大账号，拿到北大 IAAA 认证接口的发包方法。

在这里需要说明一下，学校的 IAAA 认证流程遵从 OAuth 标准，首先根据 appid, userID, password 生成一个 access_token （可能有时效性），然后相应 app 拿着这个 access_token 来访问自己的服务器，服务器端可以根据与 OAuth 认证服务器约定好的算法对 access_token 的真实性进行校验，从而实现鉴权，确保鉴权成功后，再提供服务。

jd-gui 下查看 `package cn.edu.pku.pkuiaaa_android` 下的 `IAAA_Authen.class` 的 `onCreate` 函数，可以确定网络请求的接口如下：

```java
paramAnonymousView = a.a(IAAA_Authen.this.q, IAAA_Authen.this.r, IAAA_Authen.this.s, IAAA_Authen.a, paramAnonymousView);

paramAnonymousView = a.b(IAAA_Authen.this.q, IAAA_Authen.a);
```

所以必须知道 `a.a` 和 `a.b` 函数的定义。查看同路径下的 `a.class`，然后发现两个函数 jd-gui 都解析错误了，怕不是学校还对 apk 做了一些反 Android 逆向分析的措施 ...

```java
  /* Error */
  static String a(String paramString1, String paramString2)
  {

    // Byte code:
    //   0: new 88  java/util/TreeMap
    //   3: dup
    //   4: invokespecial 89    java/util/TreeMap:<init>    ()V
    //   7: astore_2
    //   ......
  }

  /* Error */
  static String a(String paramString1, String paramString2, String paramString3, String paramString4, String paramString5)
  {
    // Byte code:
    //   0: new 88  java/util/TreeMap
    //   3: dup
    //   4: invokespecial 89    java/util/TreeMap:<init>    ()V
    //   7: astore 5
    //   ......
  }

  /* Error */
  static String b(String paramString1, String paramString2)
  {
    // Byte code:
    //   0: new 88  java/util/TreeMap
    //   3: dup
    //   4: invokespecial 89    java/util/TreeMap:<init>    ()V
    //   7: astore_2
    //   ......
  }
```

老项目 [PKULayer][PKULayer] 的方法是用 portal 的 token 来替代，可能以前确实可以这么做，但是现在学校的服务器已经更新，自己尝试着各种模拟都失败了，然后就在这儿就卡了好久 ...

大概反复折腾了一天，最后发现，如果 jd-gui 解不出 java 源码，还是可以直接读 smali 汇编代码的。于是，接下来用 `./apktool d -r -f` 直接解出原始包，在 `pkurunner/smali/cn/edu/pku/pkuiaaa_android/
` 下可以看到 `a.class` 的汇编文件 `a.smali`

```console
debian-9:/home/pyprj/apk_decompile/decompile-apk-v1.0/tools/apktool# ./apktool d -r -f ../../../pkurunner.apk
I: Using Apktool 2.3.4 on pkurunner.apk
I: Copying raw resources...
I: Baksmaling classes.dex...
I: Baksmaling classes2.dex...
I: Copying assets and libs...
I: Copying unknown files...
I: Copying original files...
```

用文本编辑器打开，通过 IAAA 认证的 host 来搜索，可以找到三个 url

```
search by "iaaa.pku.edu.cn":

"https://iaaa.pku.edu.cn/iaaa/svc/authen/sendSMSCode.do"
"https://iaaa.pku.edu.cn/iaaa/svc/authen/login.do"
"https://iaaa.pku.edu.cn/iaaa/svc/authen/isMobileAuthen.do"
```

首先看出来 `sendSMSCode.do` 应该是短信验证 API 接口，不需要理他。`login.do` 接口看起来是我们想要分析的 API 接口。同时，在这之前我通过电脑 chrome 浏览器抓包分析 `portal.pku.edu.cn` 的 IAAA 登录状态，见过 `isMobileAuthen.do` 的 API 调用情况，好像是检查是否为手机验证登录，这里显然也不需要考虑它。

因此这里我们需要分析调用 `login.do` 的函数。smali 源码见 `code/PKURunner/a.smali`。注意到以下关键代码

```smali
.method static a(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
    .locals 2

    new-instance v0, Ljava/util/TreeMap;

    invoke-direct {v0}, Ljava/util/TreeMap;-><init>()V

    const-string v1, "userName"

    invoke-virtual {v0, v1, p0}, Ljava/util/TreeMap;->put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    const-string p0, "password"

    invoke-virtual {v0, p0, p1}, Ljava/util/TreeMap;->put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    const-string p0, "randCode"

    const-string p1, ""

    invoke-virtual {v0, p0, p1}, Ljava/util/TreeMap;->put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    const-string p0, "appId"

    invoke-virtual {v0, p0, p3}, Ljava/util/TreeMap;->put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    const-string p0, "SMS"

    invoke-virtual {p4, p0}, Ljava/lang/String;->equals(Ljava/lang/Object;)Z

    move-result p0

    if-eqz p0, :cond_0

    const-string p0, "smsCode"

    invoke-virtual {v0, p0, p2}, Ljava/util/TreeMap;->put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    const-string p0, "otpCode"

    const-string p1, ""

    invoke-virtual {v0, p0, p1}, Ljava/util/TreeMap;->put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    goto :goto_0

    :cond_0
    const-string p0, "smsCode"

    const-string p1, ""

    invoke-virtual {v0, p0, p1}, Ljava/util/TreeMap;->put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    const-string p0, "otpCode"

    invoke-virtual {v0, p0, p2}, Ljava/util/TreeMap;->put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    :goto_0
    invoke-virtual {v0}, Ljava/util/TreeMap;->keySet()Ljava/util/Set;

    move-result-object p0

    invoke-interface {p0}, Ljava/util/Set;->iterator()Ljava/util/Iterator;

    move-result-object p0

    new-instance p1, Ljava/lang/StringBuilder;

    invoke-direct {p1}, Ljava/lang/StringBuilder;-><init>()V

    :goto_1
    invoke-interface {p0}, Ljava/util/Iterator;->hasNext()Z

    move-result p2

    if-eqz p2, :cond_2

    invoke-interface {p0}, Ljava/util/Iterator;->next()Ljava/lang/Object;

    move-result-object p2

    check-cast p2, Ljava/lang/String;

    invoke-virtual {v0, p2}, Ljava/util/TreeMap;->get(Ljava/lang/Object;)Ljava/lang/Object;

    move-result-object p3

    check-cast p3, Ljava/lang/String;

    invoke-virtual {p1}, Ljava/lang/StringBuilder;->length()I

    move-result p4

    if-eqz p4, :cond_1

    new-instance p4, Ljava/lang/StringBuilder;

    const-string v1, "&"

    invoke-direct {p4, v1}, Ljava/lang/StringBuilder;-><init>(Ljava/lang/String;)V

    :goto_2
    invoke-virtual {p4, p2}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    const-string p2, "="

    invoke-virtual {p4, p2}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    invoke-virtual {p4, p3}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    invoke-virtual {p4}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object p2

    invoke-virtual {p1, p2}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    goto :goto_1

    :cond_1
    new-instance p4, Ljava/lang/StringBuilder;

    invoke-direct {p4}, Ljava/lang/StringBuilder;-><init>()V

    goto :goto_2

    :cond_2
    invoke-virtual {p1}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object p0

    new-instance p1, Ljava/lang/StringBuilder;

    invoke-direct {p1}, Ljava/lang/StringBuilder;-><init>()V

    invoke-virtual {p1, p0}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    const-string p2, "7696baa1fa4ed9679441764a271e556e"

    invoke-virtual {p1, p2}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    invoke-virtual {p1}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object p1

    :try_start_0
    invoke-static {p1}, Lcn/edu/pku/pkuiaaa_android/a;->a(Ljava/lang/String;)Ljava/lang/String;

    move-result-object p1
    :try_end_0
    .catch Ljava/lang/Exception; {:try_start_0 .. :try_end_0} :catch_3

    if-eqz p1, :cond_a

    new-instance p2, Ljava/lang/StringBuilder;

    invoke-direct {p2}, Ljava/lang/StringBuilder;-><init>()V

    invoke-virtual {p2, p0}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    const-string p0, "&msgAbs="

    invoke-virtual {p2, p0}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    invoke-virtual {p2, p1}, Ljava/lang/StringBuilder;->append(Ljava/lang/String;)Ljava/lang/StringBuilder;

    invoke-virtual {p2}, Ljava/lang/StringBuilder;->toString()Ljava/lang/String;

    move-result-object p0
```

首先可以看出 payload 字段分别有 `"appId", "userName", "randCode", "password", "smsCode", "optCode"` ，根据 `IAAA_Authen.class` 对其的调用情况来看可以确定如下 payload 包。经过实验，`"randCode", "smsCode", "optCode"` 三个字段即使为空，也必须要有，否则会登录失败。

```python
payload = {
    "appId": "PKU_Runner",
    "userName": studentID,
    "randCode": "",
    "password": password,
    "smsCode": "",
    "otpCode": "",
}
```

然后发现还有一个 `msgAbs` 字段，显然是对 payload 做了签名。分析可以得出，用了很传统的 `urlencoded string + salt/secretkey`模式拼接字段，即 `"&{key1}={value1}&{key2}={value2}...&{keyN}={valueN}{salt/secretkey}"` ，然后算 MD5。其中 salt/secretkey 为 `7696baa1fa4ed9679441764a271e556e` 。按照经验来讲，为了保证服务器和前段拼接字段的一致性，key 应当是按照字典序排列的，查了一下 java 的 TreeMap （因为本渣并不会 java ...），这种 Map 确实是按照字典序进行遍历的，于是得到下述代码：

```python
payload["msgAbs"] = MD5("&".join("=".join(item) for item in sorted(payload.items())) + "7696baa1fa4ed9679441764a271e556e")
```

至此 IAAA 认证登录接口被破解。


## API 接口破解

接下来需要弄清楚怎样伪造发包。因为没有抓包结果的支持，只能从 java 源码里慢慢分析各种 API 接口的请求方法。依据老项目 [PKULayer][PKULayer] 的构造方式，可以观察到有 `/user`, `/record` 接口，利用搜索功能顺藤摸瓜，最后找到了 PKURunner 的 Network Service 类，见 `package cn.edu.pku.pkurunner.Network.Service`，并解出如下 API 接口：

```java
package cn.edu.pku.pkurunner.Network.Service;

public abstract interface ActivityService
{
  @POST("activity/20180420/user/{userId}/team/purple")
  public abstract Observable<DataPack> clear20180420(@Path("userId") String paramString);

  @POST("activity/{activityId}/user/{userId}/team/{color}")
  public abstract Observable<DataPack> signUp20180420(@Path("activityId") int paramInt, @Path("userId") String paramString1, @Path("color") String paramString2);
}

public abstract interface GymRecordService
{
  @GET("record2/{userId}")
  public abstract Observable<DataPack<ArrayList<GymRecord.Inner>>> getGymRecords(@Path("userId") String paramString);

  @FormUrlEncoded
  @POST("record2/{userId}/{recordId}")
  public abstract Observable<DataPack<GymRecord.Inner>> verifyGymRecord(@Path("userId") String paramString1, @Path("recordId") int paramInt, @Field("token") String paramString2);
}

public abstract interface LoginService
{
  @GET("public/client/android/curr_version")
  public abstract Observable<Version> getLatestVersion();

  @GET("https://raw.githubusercontent.com/pku-runner/pku-runner.github.io/android/public/client/android/curr_version")
  public abstract Observable<Version> getLatestVersionForOffline();

  @GET("public/client/android/min_version")
  public abstract Observable<Version> getMinVersion();

  @FormUrlEncoded
  @POST("user")
  public abstract Observable<DataPack<User.Inner>> login(@Field("access_token") String paramString);
}

public abstract interface RecordService
{
  @GET("record/{userId}")
  public abstract Observable<DataPack<ArrayList<Record.Inner>>> getRecords(@Path("userId") String paramString);

  @GET("record/{userId}/{recordId}")
  public abstract Observable<DataPack<Record.Inner>> getSingleRecord(@Path("userId") String paramString, @Path("recordId") int paramInt);

  @GET("record/status/{userId}")
  public abstract Observable<DataPack<UserStatus>> getStatus(@Path("userId") String paramString);

  @GET("https://restapi.amap.com/v3/geocode/regeo?output=json&extensions=base")
  public abstract Observable<AMapReverseEncoding> reverseEncoding(@Query("location") String paramString1, @Query("key") String paramString2, @Query("radius") int paramInt);

  @Multipart
  @POST("record/{userId}")
  public abstract Observable<DataPack<Record.Inner>> uploadRecord(@Path("userId") String paramString1, @Part("duration") int paramInt1, @Part("date") Date paramDate, @Part("detail") String paramString2, @Part("misc") String paramString3, @Part("step") int paramInt2, @Part("photo\"; filename=\"image.jpg\" ") RequestBody paramRequestBody);

  @Multipart
  @POST("record/{userId}")
  public abstract Observable<DataPack<Record.Inner>> uploadRecordWithoutPhoto(@Path("userId") String paramString1, @Part("duration") int paramInt1, @Part("date") Date paramDate, @Part("detail") String paramString2, @Part("misc") String paramString3, @Part("step") int paramInt2);
}

public abstract interface TaskService
{
  @GET("badge/user/{userId}")
  public abstract Observable<DataPack<Map<String, Task>>> getList(@Path("userId") String paramString);
}


public abstract interface WeatherService
{
  @GET("weather/all")
  public abstract Observable<Weather> getWeather();
}
```

已经很明确了，`/user` 是 PKURunner 的登录接口，需要一个 IAAA 认证返回的 token ，尝试一下发现直接登录成功。

然后尝试其他端口，发现返回 "Unauthorized" ，根据老项目 [PKULayer][PKULayer] ，原来 headers 内还需要添加 `Authorization` 字段，其值为登录 PKURunner 时所用的 access_token ，添加好后就正常访问了。然后开始试着调用里面的各种接口。

```python
r.headers["Authorization"] = access_token
```

但目前来说，关键需要分析的是记录上传的 API 接口，必须要弄清楚跑步记录的 payload 该如何构造。于是我们把注意力放到一下两个 API 接口上：

```java
{
  @Multipart
  @POST("record/{userId}")
  public abstract Observable<DataPack<Record.Inner>> uploadRecord(@Path("userId") String paramString1, @Part("duration") int paramInt1, @Part("date") Date paramDate, @Part("detail") String paramString2, @Part("misc") String paramString3, @Part("step") int paramInt2, @Part("photo\"; filename=\"image.jpg\" ") RequestBody paramRequestBody);

  @Multipart
  @POST("record/{userId}")
  public abstract Observable<DataPack<Record.Inner>> uploadRecordWithoutPhoto(@Path("userId") String paramString1, @Part("duration") int paramInt1, @Part("date") Date paramDate, @Part("detail") String paramString2, @Part("misc") String paramString3, @Part("step") int paramInt2);
}
```

用过 PKURunner 的人都知道，上传记录时可以选择顺便上传一张自拍（虽然我觉得这个功能有一点那个嗯 ... ），这就分别对应着两个接口，显然，我们关注 `uploadRecordWithoutPhoto` 就好了 ...

需要构造六个字段 `"userId", "duration", "date", "detail", "misc", "step"` ，通过函数名和字段名搜索，可以找到如下代码：

```java
  package cn.edu.pku.pkurunner.Network;

  public static Observable<Record> uploadRecord(Record paramRecord, File paramFile)
  {
    JSONArray localJSONArray = new JSONArray();
    try
    {
      Object localObject = paramRecord.getTrack().iterator();
      while (((Iterator)localObject).hasNext()) {
        localJSONArray.put(((Point)((Iterator)localObject).next()).toJSONArray());
      }
      localObject = new JSONObject();
      try
      {
        ((JSONObject)localObject).put("agent", "Android v1.2+");
      }
      catch (JSONException localJSONException)
      {
        localJSONException.printStackTrace();
      }
      if (paramFile == null) {
        return d.uploadRecordWithoutPhoto(Data.getUser().getId(), paramRecord.getDuration(), paramRecord.getDate(), localJSONArray.toString(), ((JSONObject)localObject).toString(), paramRecord.getStep()).subscribeOn(Schedulers.newThread()).flatMap(new a(null)).flatMap(-..Lambda.Network.iTz00fTxEHcj8D-6WG8HEy8YpEw.INSTANCE);
      }
      paramFile = RequestBody.create(MediaType.parse("image/jpeg"), paramFile);
      return d.uploadRecord(Data.getUser().getId(), paramRecord.getDuration(), paramRecord.getDate(), localJSONArray.toString(), ((JSONObject)localObject).toString(), paramRecord.getStep(), paramFile).subscribeOn(Schedulers.newThread()).flatMap(new a(null)).flatMap(-..Lambda.Network.3Q8Pm2XUTTOMgF2UDJf5PDEBdkY.INSTANCE);
    }
    catch (JSONException paramRecord)
    {
      paramRecord.printStackTrace();
    }
    return Observable.error(paramRecord);
  }
```

继续顺着思路搜索，结果发现定义 Record 类的 `Record.class` 解析失败 ... 真是蛋疼。然后仔细读了一下汇编源码，发现有很多细节弄不清楚。

所以我们得转变一个思路。应该从服务器发回的 record 数据包入手。

根据函数名和实际尝试可以看出 `/record/{userId}`, `/record/{userId}/{recordId}`, `/record/status/{userId}` 分别代表： 获得所有跑步记录概况，获得单一跑步记录，获得跑步记录状态（已跑，未跑，打卡起止时段等）。由此可以拿到一段 record ：

```json
{
    "success": true,
    "version": 20170201,
    "code": 0,
    "message": "",
    "data":
    {
        "detail": [
            [116.30783412993175, 39.988840149136045, 0],
            [116.30783540793763, 39.98884028759643, 0],
            ["...", "...", 0],
            [116.3095462555621, 39.98874034889238, 0]
        ],
        "misc":
        {
            "agent": "Android v1.2+"
        },
        "version": 20170201,
        "verified": false,
        "step": 100,
        "type": 0,
        "invalidReason": 7,
        "_id": "5bac773275a6563d7190c0cf",
        "duration": 119,
        "date": "2018-09-27T06:04:50.000Z",
        "userId": 1234567890,
        "recordId": 1,
        "distance": 182,
        "__v": 0,
        "photoPath": "",
        "id": "5bac773275a6563d7190c0cf"
    }
}
```

看到 "\_id" 字段我是彻底送了一口气，为什么呢？建过站的人可能会知道，这个键是 MongoDB 的默认主键字段，而且这里恰好是 24 位的 hash 值，和 MongoDB 是一致的！所以说段记录极有可能就是直接从 MongoDB 里 "SELECT \* " 发回来的，甚至连 "\_id" 这种内部的键值都没有删掉。如果说后端的数据库这一块做得如此简洁，那么可以猜测，记录在发过去以后也许根本就没做过任何格式转化，就直接进库了。那么这段记录的结构，应该就是我的目标 payload 的结构！

下面这些代码支持这一猜测：

```java
  package cn.edu.pku.pkurunner.Network;

  public static Observable<Record> uploadRecord(Record paramRecord, File paramFile)
  {
    // ......

      try
      {
        ((JSONObject)localObject).put("agent", "Android v1.2+");
      }

    // ......
  }
```

以上是对 `"misc"` 字段的构造

```java
  package cn.edu.pku.pkurunner.Network;

  public static Observable<Record> uploadRecord(Record paramRecord, File paramFile)
  {
    // ......

      Object localObject = paramRecord.getTrack().iterator();
      while (((Iterator)localObject).hasNext()) {
        localJSONArray.put(((Point)((Iterator)localObject).next()).toJSONArray());
      }

    // ......
  }


  package cn.edu.pku.pkurunner.Model;

  public JSONArray toJSONArray()
    throws JSONException
  {
    JSONArray localJSONArray = new JSONArray();
    try
    {
      localJSONArray.put(0, this.longitude);
      localJSONArray.put(1, this.latitude);
      localJSONArray.put(2, this.status);
      return localJSONArray;
    }

    // ......
  }
```

以上代码是对 `"detail"` 中每一个 GPS 数据点的构造，这里可以看出，第三个值表示这个点的状态，显然 **0** 表示正常。

然后根据经验和试验可以看出，`"date"` 字段是一个 GMT 时间戳。剩下的 `"userId", "duration", "step"` 三个字段的含义是显然的，就不多说啦。

剩下的工作你懂的，就不多说啦～

至此，便完成所有的破解工作。


[PKULayer]: https://github.com/tegusi/PKULayer
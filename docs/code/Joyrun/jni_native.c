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

<!-- -*- coding: utf-8 -*- -->
# Authenticode署名データ抽出ツール

[ Japanese (日本語) / [English](./README.md) ]

[
https://github.com/trueroad/extract-authenticode
](https://github.com/trueroad/extract-authenticode)

PEフォーマットのファイル（.exe や .dll 等）から
Authenticodeの署名データを抽出します。

## 必要なもの

ビルドには以下が必要です。

- Microsoft Visual Studio Express 2013 for Windows Desktop
- MinGW-W64

等

## ビルド

### Visual Studio

VS2013 のコマンドプロンプトを開き、
ソースのあるフォルダをカレントフォルダにしてから、
"nmake"を実行すると、"extract-authenticode.exe" が得られます。

### MinGW-W64

```
gcc -municode -o extract-authenticode.exe extract-authenticode.c
```

## 使い方

例：explorer.exe の Authenticode データを抽出してみる。

```
extract-authenticode C:\Windows\explorer.exe out.bin
```

例：抽出された出力ファイルを解析してみる。（openssl が必要です）

```
openssl asn1parse -in out.bin -inform der -i
```

## おまけ

Microsoft Authenticodeのタイムスタンプについてお勉強(第2回)

http://blog.livedoor.jp/k_urushima/archives/729378.html

に、Authenticode 署名の構造が載っていて、一応の取り出し方が書いてある。
しかし、残念ながらこの方法では Authenticode 署名を
見つけることができないケースが散見されたので、Microsoft の文献等をあたり、
正しい取り出し方を調べて実装してみたのが、このツールである。

上記ページでは実行プログラムや DLL の末尾の方で、

?0 0? 00 00 00 02 02 00-30 82 ...

の並びが出てきたら、30 82 から最後の部分までが署名データである、
としているが、文献によると、この部分は WIN_CERTIFICATE 構造体であり、
最初の 4 バイトは構造体のサイズを格納する場所であった。
リトルエンディアンなので、構造体のサイズが 0x00000??0 であれば、
上記の並びに一致するわけだ。

古めの署名ツールではアラインメントの関係で下一桁が 0 になるよう
パディングされていたのかもしれないが、
新しめの署名ツールでは 0 になるとは限らないようだ。
また、サイズが大きくなっていて 0xff0 を超えてしまうケースもある。
特に、タイムスタンプの形式が変わっているようで、
古いタイムスタンプの形式（非 RFC 3161）では 0xff0 に収まっていても、
新しいタイムスタンプの形式（RFC 3161）では収まらなかったり、などなど、
あるようにも見える。

## ライセンス

Copyright (C) 2014, 2019 Masamichi Hosoda. All rights reserved.

License: BSD-2-Clause

[LICENSE](./LICENSE) をご覧ください。

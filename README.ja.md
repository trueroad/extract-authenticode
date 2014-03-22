<!-- -*- coding: utf-8 -*- -->
# Authenticode署名データ抽出ツール

PEフォーマットのファイル（.exe や .dll 等）から
Authenticodeの署名データを抽出します。

## 必要なもの

ビルドには以下が必要です。

- Microsoft Visual Studio Express 2013 for Windows Desktop

等

## ビルド

VS2013 のコマンドプロンプトを開き、
ソースのあるフォルダをカレントフォルダにしてから、
"nmake"を実行すると、"extract-authenticode.exe" が得られます。

## 使い方

例：explorer.exe の Authenticode データを抽出してみる。

extract-authenticode C:\Windows\explorer.exe out.bin

例：抽出された出力ファイルを解析してみる。（openssl が必要です）

openssl asn1parse -in out.bin -inform der -i

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

Copyright (C) 2014 Masamichi Hosoda. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

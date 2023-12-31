# このレポジトリについて

デバイスドライバについて学習するため、[こちらの記事](https://qiita.com/iwatake2222/items/580ec7db2e88beeac3de)を再現実装したもの。  
(デバイスドライバ名やファイル名は実験のため変更)  


# 環境

- jetson orin nano developer kit (8GB)

# 実行方法について

おおよそMakefileで提供している。(必要に応じて後述)  

```
# モジュール(デバイスドライバ)のビルド
make

# モジュールの削除
make clean

# モジュールのロード
make load

# モジュールのアンロード
make unload

# デバイスファイルの読み込み
make read

# デバイスファイルへの書き込み
make write

# 確認(詳しくは各モジュールのMakefileを確認)
make check_[hogehoge]
```

## シンプルなモジュール


## デバイスドライバとシステムコールハンドラ

`./dfile`

- デバイスドライバのメジャー番号を決め打ちで静的に登録し、ユーザスペースからデバイスドライバにアクセス  
    - デバイスドライバのメジャー番号を決め打ちで静的に登録(63)  
    - メジャー番号60～63、120～127、240～254はローカルの実験用にreservedされている番号  
- デバイスドライバに対応するデバイスファイルを作成し、そのファイルに対する open/read/write/close のシステムコールを処理するの関数を実装  
    - デバイスファイルの作成は手動

注意点 :  
- 今回利用したjetson orin nanoでは、ユーザスペースのポインタはカーネルスペースから直接アクセスすることはできない  
- カーネルスペースからユーザスペースへのデータの転送には、copy_to_user()関数を使用する  


## 動的なデバイスメジャー番号の割り当て ~ ユーザースペースと値のやり取り

`./ddfile`

デバイスのメジャー番号を決め打ちで静的に設定して、カーネルに登録するという方法は現在では推奨されていない。 

- デバイスのメジャー番号を動的に設定
- ルールファイルを作成し、後続のステップで作成されるデバイスファイルの権限を設定
    - デフォルトでは 600など
    - `/etc/udev/rules.d/`に、拡張子が.rulesで数字から始まるファイルとして作成 (sudo では作成できないので、sudo -i)
- udevの仕組みを利用してデバイスファイルを自動で作成
    - デバイスクラス構造体を用意する
    - 初期化関数で、`/sys/class/`にクラス登録をする (`class_create()`, `device_create()`)
    - insmodしたタイミングで`/sys/class/mydevice/mydevice0/dev`が作成される
    - udevdというデーモンがそれを検出して自動的にデバイスファイルが作成される
    - 手動で/dev/mydevice2を作ることはできるが、openしたりアクセスしようとすると、No such device or addresといったエラーになる


補足 :
- マイナー番号0(/dev/mydevice0)とマイナー番号1(/dev/mydevice1)で処理を変えたい場合
    - read/writeハンドラ関数内で、マイナー番号を使ってswitch-caseで処理を分ける方法
    - 登録するハンドラテーブルを分ける方法
        1. `s_ddfile_fops`を処理を分けたいマイナー数分用意する(例えば、`s_ddfile_fops0`と`s_ddfile_fops1`)
        2. `struct cdev ddfile_cdev;`を配列にする
        3. `cdev_init()`, `cdev_add()`, `cdev_del()`で、別々の設定をする
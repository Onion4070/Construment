# Construment

## 概要
Switch Pro Controller を RP2350-USB-A で USB ホスト接続し、ボタン入力に応じてコントローラーを振動させ、演奏する。

## Prerequisites
- ボード: RP2350-USB-A
- 開発環境: Arduino IDE
- PC はオプション (シリアルログ確認や開発時に推奨)

## Quick Start
1. `toRP2350.ino` を Arduino IDE で開く
2. Tools: CPU Speed を 120 の倍数に、USB Stack を "Adafruit TinyUSB" に設定
3. ボード (RP2350-USB-A) へ書き込み
4. Pro Controller を RP2350-USB-A のホスト端子に接続
5. (任意) PC と接続してシリアル (Serial1, 115200) でログを確認

**重要**: Pro Controller は RP2350-USB-A の電源投入**前**に接続してください。

## Wiring
基本構成: Pro Controller - RP2350-USB-A - PC (任意)

## Libraries
- Adafruit TinyUSB  - USB Host 機能
- Adafruit NeoPixel - NeoPixel 操作

## Configuration
- `HOST_PIN_DP` = 12  (PIO USB D+)
- `DIN_PIN` = 16      (NeoPixel DIN)
- シリアル: `Serial`, 115200

## LED ステータス
RP2350ボードに搭載された内蔵NeoPixelは、動作状態を色でわかりやすく示します。

| LED色 | 状態 | 説明 | 推奨アクション |
|------|------|------|---------------|
| 🔵 青色 | 待機中 | Pro Controllerの接続待ち。USBデバイスが接続されるまでこの状態になります。 | Pro Controllerがボードに正しく接続されているか確認。電源投入前に接続する運用を推奨。 |
| 🟡 黄色 | 接続中 | Pro Controller が検出され、初期化シーケンスを実行中。 | 初期化が長時間続く場合はケーブル・電源を確認し、再接続を試す。 |
| 🟢 緑色 | 接続完了 / 正常動作 | 初期化完了し、コントローラ入力が正常に処理されている状態。 | 正常。問題がなければそのまま使用。 |
| 🔴 赤色 | 切断・エラー | 接続エラーやタイムアウトが発生した状態。 | ケーブル、電源、接続タイミング（Pro Controllerは電源投入前に接続）を確認し、必要なら再起動。シリアルログを確認して原因を特定。 |


## Viewer (デスクトップアプリ) のビルド方法（Windows）
このリポジトリの `Construment\viewer` は Windows 向けのデスクトップアプリ（wxWidgets 使用）です。以下の手順でビルドできます。

### 前提条件
- Visual Studio 2022（C++ ワークロード）をインストールしていること
- wxWidgets がインストール済みで、MSVC 向けにビルドされていること（例: `C:\wxWidgets`）。`lib\vc_x64_lib` に `.lib` が存在することを確認してください。

### 手順
1. `Construment\viewer` フォルダへ移動します:
```powershell
cd .\viewer\
```
2. `build.bat` を実行します:
```powershell
.\build.bat
```
3. `x64\Release\viewer.exe`を実行してアプリを起動します。
```powershell
.\x64\Release\viewer.exe
```

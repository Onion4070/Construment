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
**重要**: Pro Controller は RP2350-USB-A の電源投入**前**に接続してください。
5. (任意) PC と接続してシリアル (Serial1, 115200) でログを確認

## Wiring
基本構成: Pro Controller -> RP2350-USB-A (マイコン/ホスト) -> PC (オプション)

## Libraries
- Adafruit TinyUSB — USB Host 機能
- Adafruit NeoPixel — NeoPixel 操作

## Configuration
- `HOST_PIN_DP` = 12  (PIO USB D+)
- `DIN_PIN` = 16      (NeoPixel DIN)
- シリアル: `Serial1`, 115200

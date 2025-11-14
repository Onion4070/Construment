// ツール->CPU Speedを120の倍数に変更
// ツール->USB StackをAdafruit TinyUSBに変更

#include "pio_usb.h"
#include "Adafruit_TinyUSB.h"
#include "tusb.h"

#define HOST_PIN_DP 12  // PIO USB D+ ピン
Adafruit_USBH_Host USBHost;

// NeoPixel (デバッグ用LED)
#include <Adafruit_NeoPixel.h>
#define DIN_PIN 16
#define LED_COUNT 1
Adafruit_NeoPixel pixels(LED_COUNT, DIN_PIN, NEO_RGB + NEO_KHZ800);

void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

// vid/pid
#define VID_NINTENDO   0x057e
#define PID_SWITCH_PRO 0x2009

// ====== Switch Pro Controller Init State Machine ======

namespace SwitchPro {
  static constexpr uint8_t INFO_CONN_MASK = 0xAB;
  static constexpr uint8_t INFO_BATTERY_MASK = 0x0F;

  namespace CMD {
    static constexpr uint8_t HID = 0x80;
    static constexpr uint8_t RUMBLE_ONLY = 0x10;
    static constexpr uint8_t AND_RUMBLE = 0x01;
    static constexpr uint8_t LED = 0x30;
    static constexpr uint8_t LED_HOME = 0x38;
    static constexpr uint8_t GYRO = 0x40;
    static constexpr uint8_t MODE = 0x03;
    static constexpr uint8_t FULL_REPORT_MODE = 0x30;
    static constexpr uint8_t HANDSHAKE = 0x02;
    static constexpr uint8_t DISABLE_TIMEOUT = 0x04;
  }

  namespace Buttons0 {
    // report[3]
    static constexpr uint8_t Y  = 0x01;
    static constexpr uint8_t X  = 0x02;
    static constexpr uint8_t B  = 0x04;
    static constexpr uint8_t A  = 0x08;
    static constexpr uint8_t R  = 0x40;
    static constexpr uint8_t ZR = 0x80;
  }
  namespace Buttons1 {
    // report[4]
    static constexpr uint8_t MINUS   = 0x01;
    static constexpr uint8_t PLUS    = 0x02;
    static constexpr uint8_t R3      = 0x04;
    static constexpr uint8_t L3      = 0x08;
    static constexpr uint8_t HOME    = 0x10;
    static constexpr uint8_t CAPTURE = 0x20;
  }
  namespace Buttons2 {
    // report[5]
    static constexpr uint8_t DPAD_DOWN  = 0x01;
    static constexpr uint8_t DPAD_UP    = 0x02;
    static constexpr uint8_t DPAD_RIGHT = 0x04;
    static constexpr uint8_t DPAD_LEFT  = 0x08;
    static constexpr uint8_t L   = 0x40;
    static constexpr uint8_t ZL  = 0x80;
  }
}

enum class InitState {
  HANDSHAKE,
  DISABLE_TIMEOUT,
  LED,
  LED_HOME,
  FULL_REPORT,
  IMU,
  DONE
};

InitState init_state = InitState::HANDSHAKE;
uint8_t seq_counter = 0;
uint8_t procon_addr = 0, procon_instance = 0;
bool is_procon = false;
// 接続監視用
unsigned long last_data_time = 0;
const unsigned long CONNECTION_TIMEOUT = 3000UL; // ms

struct OutputReport {
  uint8_t command;
  uint8_t sequence_counter;
  uint8_t rumble_l[4];
  uint8_t rumble_r[4];
  uint8_t sub_command;
  uint8_t sub_command_args[8];
} out_report;

// 送信ラッパ
void send_report(uint8_t size) {
  out_report.sequence_counter = seq_counter++ & 0x0F;
  if (!tuh_hid_send_report(procon_addr, procon_instance, 0, &out_report, size)) {
    Serial.printf("send_report failed. state=%d\r\n", (int)init_state);
  }
}

// 短い（2バイト）レポート送信用ヘルパ。
// ログを統一し、最小限のプリウェイトと 1 回の再試行を行う。
static bool send_short_report(const char* name, uint8_t seq_value) {
  out_report.command = SwitchPro::CMD::HID;
  out_report.sequence_counter = seq_value;
  const uint8_t* b = (const uint8_t*)&out_report;
  Serial.printf("%s: addr=%d inst=%d size=2 data=%02X %02X\r\n", name, procon_addr, procon_instance, b[0], b[1]);
  // 少し待ってから送信
  delay(50);
  bool ok = tuh_hid_send_report(procon_addr, procon_instance, 0, &out_report, 2);
  Serial.printf("%s attempt=1 -> %d\r\n", name, ok);
  if (!ok) {
    // 状態を前進させて再試行を一度だけ行う
    USBHost.task();
    delay(20);
    ok = tuh_hid_send_report(procon_addr, procon_instance, 0, &out_report, 2);
    Serial.printf("%s attempt=2 -> %d\r\n", name, ok);
  }
  return ok;
}

// エラー遷移を一箇所にまとめる
static void set_controller_error_state(const char* reason) {
  is_procon = false;
  procon_addr = 0;
  procon_instance = 0;
  setLEDColor(15, 0, 0);
  Serial.print("ERROR: ");
  Serial.println(reason);
}

// タイムアウトチェック
static void check_connection_timeout(void) {
  if (!is_procon) return;
  if (millis() - last_data_time > CONNECTION_TIMEOUT) {
    set_controller_error_state("Data Timeout");
  }
}

// 次の初期化ステップを進める
void advance_init() {
  memset(&out_report, 0, sizeof(out_report));
  uint8_t report_size = 10;

  switch (init_state) {
    case InitState::HANDSHAKE:
      report_size = 2;

      out_report.command = SwitchPro::CMD::HID;
      out_report.sequence_counter = SwitchPro::CMD::HANDSHAKE;

      // 2バイトの簡易レポートはヘルパで送る（ログを統一）
      if (send_short_report("send(HANDSHAKE)", SwitchPro::CMD::HANDSHAKE)) {
        init_state = InitState::DISABLE_TIMEOUT;
        Serial.println("advance_init(): -> DISABLE_TIMEOUT");
      }
      break;

    case InitState::DISABLE_TIMEOUT:
      report_size = 2;

      out_report.command = SwitchPro::CMD::HID;
      out_report.sequence_counter = SwitchPro::CMD::DISABLE_TIMEOUT;

      // こちらも短いレポートなのでヘルパで送信
      if (send_short_report("send(DISABLE_TIMEOUT)", SwitchPro::CMD::DISABLE_TIMEOUT)) {
        init_state = InitState::LED;
        Serial.println("advance_init(): -> LED");
      }
      break;

    case InitState::LED:
      report_size = 12;

      out_report.command = SwitchPro::CMD::AND_RUMBLE;
      out_report.sub_command = SwitchPro::CMD::LED;
      out_report.sub_command_args[0] = 0x01; // Player 1
      send_report(report_size);
      init_state = InitState::LED_HOME;
      break;

    case InitState::LED_HOME:
      report_size = 14;

      out_report.command = SwitchPro::CMD::AND_RUMBLE;
      out_report.sub_command = SwitchPro::CMD::LED_HOME;
      out_report.sub_command_args[0] = 0x0F;
      out_report.sub_command_args[1] = 0xF0;
      out_report.sub_command_args[2] = 0xF0;
      send_report(report_size);
      init_state = InitState::FULL_REPORT;
      break;

    case InitState::FULL_REPORT:
      report_size = 12;

      out_report.command = SwitchPro::CMD::AND_RUMBLE;
      out_report.sub_command = SwitchPro::CMD::MODE;
      out_report.sub_command_args[0] = SwitchPro::CMD::FULL_REPORT_MODE;
      send_report(report_size);
      // init_state = InitState::IMU;
      init_state = InitState::DONE;
      // 初期化完了: 緑色に変更
      setLEDColor(0, 15, 0);
      break;

    case InitState::IMU:
      report_size = 12;

      out_report.command = SwitchPro::CMD::AND_RUMBLE;
      out_report.sub_command = SwitchPro::CMD::GYRO;
      out_report.sub_command_args[0] = 0x01; // 1=enable
      send_report(report_size);
      init_state = InitState::DONE;
      break;

    default:
      break;
  }
}

// ====== TinyUSB Callbacks ======

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance,
                      uint8_t const* desc_report, uint16_t desc_len) {
  uint16_t vid, pid;
  if (tuh_vid_pid_get(dev_addr, &vid, &pid)) {
    Serial.printf("VID:%04X PID:%04X\r\n", vid, pid);

    if (vid == VID_NINTENDO && pid == PID_SWITCH_PRO) {
      Serial.println("Switch Pro Controller detected!");
      procon_addr = dev_addr;
      procon_instance = instance;
      is_procon = true;
      init_state = InitState::HANDSHAKE;
      last_data_time = millis();
      // 接続中を示す黄色に変更
      setLEDColor(15, 15, 0);
      advance_init();  // 初期化開始
    }
  }
  tuh_hid_receive_report(dev_addr, instance); // 最初の受信開始
}

// --- 音階データと操作（このファイル内にインライン） ---
// 目的
// - 装置固有のノートコード（例: 0x2d）を扱いやすくまとめる。
// - 半音 (+1 / -1) やオクターブ (+12 / -12) 単位で簡単に移調できるようにする。
//
// 構成と使い方
// - enum Scale::Note は「セミトーン順のインデックス」です（0 が最も低いノート）。
//   そのためインデックスに対して加減算するだけで移調できます。
// - 実際にコントローラへ渡す値（0x2d 等）は配列 codes[] に格納しています。
//   インデックス -> デバイスコード は Scale::code(note) で取得します。
// - 範囲外アクセスは自動で下限/上限へクランプするため、安全に使えます。
//
// 例:
//   // C4 を半音上げて振動を送る
//   rumble( Scale::code( Scale::transpose(Scale::C4, +1) ), amp );
//   // A4 を 1 オクターブ下げる
//   auto n = Scale::down_octave(Scale::A4);
//   rumble( Scale::code(n), amp );
namespace Scale {
  enum Note : int8_t {
                                       Gs2, A2, As2, B2,
    C3, Cs3, D3, Ds3, E3, F3, Fs3, G3, Gs3, A3, As3, B3,
    C4, Cs4, D4, Ds4, E4, F4, Fs4, G4, Gs4, A4, As4, B4,
    C5, Cs5, D5,
    Silence,
    COUNT
  };

  // enum と同じ半音順で並んでいます。
  static const uint8_t codes[COUNT] = {
                                                    0x2d, 0x30, 0x33, 0x35,
    0x38, 0x3b, 0x3d, 0x3f, 0x42, 0x45, 0x48, 0x4a, 0x4d, 0x50, 0x52, 0x55,
    0x58, 0x5a, 0x5d, 0x60, 0x62, 0x65, 0x68, 0x6a, 0x6d, 0x70, 0x72, 0x75,
    0x78, 0x7a, 0x7d,
    0x00
  };

  // ノート（enum）から機器固有のコードを取得します（範囲をクランプ）。
  inline uint8_t code(Note n) {
    int idx = (int)n;
    if (idx < 0) idx = 0;
    if (idx >= COUNT) idx = COUNT - 1;
    return codes[idx];
  }

  // 半音単位で移調し、テーブル範囲に収めます（下限/上限にクランプ）。
  inline Note transpose(Note n, int semitones) {
    int idx = (int)n + semitones;
    if (idx < 0) idx = 0;
    if (idx >= COUNT) idx = COUNT - 1;
    return (Note)idx;
  }

  // ヘルパ関数
  inline Note up_semitone(Note n) { return transpose(n, +1); }
  inline Note down_semitone(Note n) { return transpose(n, -1); }
  inline Note up_octave(Note n) { return transpose(n, +12); }
  inline Note down_octave(Note n) { return transpose(n, -12); }
}
// --- end inlined note_map ---

void rumble(int frequency, int amplitude) {
  memset(&out_report, 0, sizeof(out_report));
  out_report.command = 0x10;  // Rumble only
  out_report.sequence_counter = seq_counter++ & 0x0F;

  out_report.rumble_l[0] = 0x00;
  out_report.rumble_l[1] = 0x01;
  out_report.rumble_l[2] = frequency;
  out_report.rumble_l[3] = amplitude;
  out_report.rumble_r[0] = 0x00;
  out_report.rumble_r[1] = 0x01;
  out_report.rumble_r[2] = frequency;
  out_report.rumble_r[3] = amplitude;

  tuh_hid_send_report(procon_addr, procon_instance, 0, &out_report, 10);
}

int amp = 0x60;
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const* report, uint16_t len) {
  if (len == 0) return;

  // Serial.print("HID Report: ");
  // for (uint16_t i = 0; i < len; i++) {
  //   Serial.printf("%02X ", report[i]);
  // }
  // Serial.println();

  // 押されているボタンから移調オフセットを決定
  int semitone_offset = 0;
  if (report[3] & SwitchPro::Buttons0::R) semitone_offset += 1;   // R: semitone up
  if (report[3] & SwitchPro::Buttons0::ZR) semitone_offset += 12; // ZR: octave up
  if (report[5] & SwitchPro::Buttons2::L) semitone_offset -= 1;   // L: semitone down
  if (report[5] & SwitchPro::Buttons2::ZL) semitone_offset -= 12; // ZL: octave down

  // +/- ボタン（Buttons1）で音量調整
  if (report[4] & SwitchPro::Buttons1::MINUS) amp--;
  if (report[4] & SwitchPro::Buttons1::PLUS)  amp++;
  if (amp < 0x40) amp = 0x40;
  if (amp > 0x7f) amp = 0x7f;

  // ボタンを基準ノートに割当て
  Scale::Note base = Scale::C4;
  bool note_selected = false;

  // 方向キー (DPAD, report[5])
  if (report[5] & SwitchPro::Buttons2::DPAD_UP)         { base = Scale::C4; note_selected = true; }
  else if (report[5] & SwitchPro::Buttons2::DPAD_LEFT)  { base = Scale::D4; note_selected = true; }
  else if (report[5] & SwitchPro::Buttons2::DPAD_DOWN)  { base = Scale::E4; note_selected = true; }
  else if (report[5] & SwitchPro::Buttons2::DPAD_RIGHT) { base = Scale::F4; note_selected = true; }

  // ABXY ボタン (report[3])
  if (!note_selected) {
    if (report[3] & SwitchPro::Buttons0::X)       { base = Scale::G4; note_selected = true; }
    else if (report[3] & SwitchPro::Buttons0::A)  { base = Scale::A4; note_selected = true; }
    else if (report[3] & SwitchPro::Buttons0::B)  { base = Scale::B4; note_selected = true; }
    else if (report[3] & SwitchPro::Buttons0::Y)  { base = Scale::C5; note_selected = true; }
  }

  if (note_selected) {
    Scale::Note target = Scale::transpose(base, semitone_offset);
    uint8_t code = Scale::code(target);
    Serial.printf("note code=%02x, amp=%02x, offset=%d\r\n", code, amp, semitone_offset);
    rumble(code, amp);
  } else {
    // ノートボタンが押されていない場合: アイドル振動を送信
    Serial.printf("idle rumble , amp=%02x, offset=%d\r\n", amp, semitone_offset);
    rumble(0x40, 0x40);
  }

  // 受信があったらタイムスタンプのみ更新する。
  if (is_procon && dev_addr == procon_addr) {
    last_data_time = millis();
  }

  // 初期化シーケンスを進める
  if (is_procon && init_state != InitState::DONE) {
    advance_init();
  }

  // 常に次のレポートを要求
  tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  Serial.printf("HID unmounted: addr=%d instance=%d\n", dev_addr, instance);

  // 切断されたデバイスが、現在接続されているProコントローラーか確認
  if (dev_addr == procon_addr && instance == procon_instance) {
    Serial.println("Pro Controller disconnected. Resetting state.");
    is_procon = false;
    procon_addr = 0;
    procon_instance = 0;
    // 切断時はエラー/切断表示として赤色にする
    setLEDColor(15, 0, 0);
    init_state = InitState::HANDSHAKE; // 初期化状態を最初に戻す
    seq_counter = 0;                   // シーケンスカウンターもリセット
  }
}

void send_keepalive() {
  memset(&out_report, 0, sizeof(out_report));
  out_report.command = 0x10;  // Rumble only
  out_report.sequence_counter = seq_counter++ & 0x0F;

  out_report.rumble_l[0] = 0x00;
  out_report.rumble_l[1] = 0x01;
  out_report.rumble_l[2] = 0x60;
  out_report.rumble_l[3] = 0x60;
  out_report.rumble_r[0] = 0x00;
  out_report.rumble_r[1] = 0x01;
  out_report.rumble_r[2] = 0x40;
  out_report.rumble_r[3] = 0x40;

  tuh_hid_send_report(procon_addr, procon_instance, 0, &out_report, 10);
}

// ====== Hardware Timer ======
bool repeating_keep_alive_cb(struct repeating_timer *t) {
  if (is_procon && init_state == InitState::DONE) {
    send_keepalive();
  }
  return true;
}
struct repeating_timer timer;


// ====== Core 0: main logic ======

void setup() {

}

void loop() {

}

// ====== Core1: Process USB Host ======

void init_usb_host() {
  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = HOST_PIN_DP;
  USBHost.configure_pio_usb(1, &pio_cfg);
  USBHost.begin(1);
  Serial.println("USB host init done");
}

void setup1() {
  // UARTでシリアル通信開始
  Serial.begin(115200);
  pixels.begin();
  // 青色LED
  setLEDColor(0, 0, 15);

  // 電源等の安定待ち (これがないとProコンがコネクションを確立できない)
  delay(500);
  init_usb_host();

  // ハードウェアタイマー初期化
  // これを実行するとポーリングレートが75Hzでなんか安定する
  // param1: 実行間隔 (μs), -8000 -> 125Hz (params1 < 0 -> 実行時間を考慮せずタイマー発動)
  // param2: コールバック関数
  // param3: コールバック関数の引数 (nullptr)
  // param4: タイマー構造体のポインタ
  // add_repeating_timer_us(-8000, repeating_keep_alive_cb, nullptr, &timer);
}

// これ以外の処理は走らせない!!! 接続が不安定になる原因となり得る!!!
void loop1() {
  USBHost.task();
  // データ受信タイムアウトのチェック
  check_connection_timeout();
}
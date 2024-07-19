# Operating System Capstone 2024

This repository contains the code I wrote while self-studying the [OSC](https://nycu-caslab.github.io/OSC2024/index.html) (formerly OSDI) course labs.

## Raspberry Pi Model 3 B+

開發板: Raspberry Pi 3 Model B+ (參考[此連結](https://ricelee.com/product/raspberry-pi-3-model-b-plus))。

## Usb to TTL Serial Cable

傳輸線: PL2303HXD USB 轉 TTL 序列傳輸線 (參考[此連結](https://ricelee.com/product/PL2303HXD-USB-to-TTL-Cable))。

### Specification

開發板的各個腳位該如何接線 (e.g., 紅色線 (5V) 接 5V power, 白色線 (RX) 接 GPIO 14 (TXD)) 可參考 [lab0](https://nycu-caslab.github.io/OSC2024/labs/lab0.html#interact-with-rpi3)。以下為傳輸線的規格 (參考[此網站](https://piepie.com.tw/2044/pl2303hxd-usb-to-ttl-serial-cable))。

* 紅線: 5V
* 黑色線：GND
* 白色線: RX
* 綠色線: TX
* 黃色線: RTS
* 藍色線: CTS

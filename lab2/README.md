# Lab2: Booting

## Introduction

Booting 是在一個裝置重置後，設置執行不同使用者程式的環境的過程。這個過程包含 bootloader 載入 kernel，初始化子系統，對應裝置驅動以及載入初始使用者程式去執行各項 userspace 的服務。

這個實驗會讓我們學到一種載入核心以及使用者程式的方法，並且也會讓我們學到如何對應一個裝置到 Rpi3 的驅動程式。後續的實驗會介紹如何初始化子系統。以下為這個 lab 透過實做帶給我們的知識:

1. Bootloader: 載入完成開機所需程式 (e.g., kernel) 的程式
2. Initial ramdisk: 真正掛載檔案系統根目錄前，負責完成系統初始化的檔案系統。
3. Simple allocator:
4. Devicetree:

## 背景知識

### 核心是如何被載入到 Rpi3 的？

在核心開始執行前會經過以下四個步驟:

1. GPU 從 SoC 上的 ROM 執行第一階段 bootloader。
2. 第一階段 bootloader 會辨認 FAT16/32 檔案系統，並把 SD 卡中的第二階段 bootloader `bootcode.bin` 載入到 L2 cache。
3. `bootcode.bin` 初始化 SDRAM 並載入 `start.elf`。
4. `start.elf` 讀取載入 kernel 的設置與其他資料到記憶體中，然後再喚醒 CPU 開始去執行。

在第四步被載入的核心可以是另一個像是網路開機或 ELF loading 這類功能更強大的 bootloader。

這個 lab 會實做一個在上述階段後被載入，並且可以透過 UART 載入實際要測試的核心的 bootloader。

## Basic Exercise 1 - UART Bootloader - 30%

實做 lab1 的過程中應該常常會在 debug 的時後讓 SD 卡在主機與 rpi3 之間移動。接下來將透過引入另一個能在 debug 時載入核心的 bootloader 來免去移動 SD 卡的動作。

上個 lab 中，主機與 Rpi3 是透過 UART 溝通的。這個練習將沿用 UART 讓主機傳輸核心給開發板 (代表需要為我們的程式設計一個能讀取 raw data 的功能)。在 Linux 中我們可以有效的透過序列裝置的裝置檔案 (serial device) 讓主機寫資料到 rpi3。

* 在 Linux 中，操作裝置的方式會是利用把裝置當成檔案的方式去做讀寫，而這個實驗因為是透過 USB 轉 TTL 傳輸線進行資料傳輸，因此以下程式透過對 `/dev/ttyUSB0` 檔案進行寫入操作來傳輸資料。

  ```python
  with open('/dev/ttyUSB0', "wb", buffering = 0) as tty:
      tty.write(...)
  ```

### Config Kernel Loading Setting

或許你仍然會想把你實際的核心載入到 0x80000，但這會與你的 bootloader 重疊。

你可以先透過重寫 linker script 來指定另一個起始位址，接下來把 config.txt 檔放到 SD 卡的開機區塊，再透過 `kernel_address=` 來指定載入位址。

為了讓我們更容易區別出 bootloader 與實際核心，我們可以透過 `kernel=` 新增載入映像名稱並設置 `arm_64bit=1`。

```txt
kernel_address=0x60000
kernel=bootloader.img
arm_64bit=1
```

## Basic Exercise 2 - Initial Ramdisk - 30%

當核心初始化後，它會掛載一個根檔案系統並且執行一個初始使用者程式。這個初始程式可以是一個腳本，或是用來帶起其他服務或晚點用來載入其他驅動程式的可執行 binary。

然而，因為目前還沒實做任何檔案系統與儲存裝置的驅動程式，因此無法透過核心從 SD 卡載入任何東西 (包含初始程式與相關服務)。對於這種情況的其中一種解決方式為透過 initial ramdisk 載入使用者程式。

Initial ramdisk 是一個透過 bootloader 載入或是嵌入在核心的檔案。它通常是用來建構根檔案系統的檔案 (負責掛載最終掛載的檔案系統根目錄前所需的初始化設定)。([Ref.](https://www.kingston.com/tw/blog/pc-performance/what-is-ram-disk))

### New ASCII Format Cpio Archive

Cpio 是一個用來打包目錄與檔案的簡易檔案格式。每個目錄與檔案會以 {標頭 (header)，路徑與內容} 這個的格式被紀錄 (`{header: full filepath + filename: data}`)。

這個 lab 需要用 New ASCII Format Cpio 格式去創建一個 cpio 壓縮檔，並且在創建一個目錄 (`rootfs`) 並放一些測試用的檔案在裡面。接下來用下列指令去建立 cpio 檔:

* `cpio` 指令各項參數的功用:
  * `-o`: 創建壓縮檔
  * `-H`: 指定以下支援的格式:
    * bin: 指定較舊的二進位格式。
    * odc: 舊版 (POSIX.1) 的可攜式格式。
    * newc: 新版 (SVR4) 的可攜式格式，支援多於 65536 個 inodes 的檔案系統。
    * crc: 新增 checksum 的 newc 格式。
    * ...
* `find` 指令會搜尋給定位置的所有目錄。若沒有指定起始位置的話會使用 `.` (i.e., 上方的 `find .` 可以簡化成 `find`)。

```bash
cd rootfs
find . | cpio -o -H newc > ../initramfs.cpio
cd ..
```

[FreeBSD 的手冊](https://man.freebsd.org/cgi/man.cgi?query=cpio&sektion=5)有詳細定義 New ASCII Format Cpio Archive 的組成方式，在這個練習中需要閱讀手冊並實做一個解析器讀取壓縮檔中的檔案。

### Loading Cpio Archive

#### QEMU

#### Rpi3

將 cpio 壓縮檔移動到 SD 卡，並透過 `config.txt` 指定檔案名稱與載入位址。

```txt
initramfs initramfs.cpio 0x20000000
```

## Basic Exercise 3 - Simple Allocator - 10%

實做簡易的記憶體配置器，僅需實做 `malloc()`，先不用實做 `free()`。

## Advanced Exercise 1 - Bootloader Self Relocation - 10%

雖然前面的練習直接使用 `config.txt` 指定自定義 bootloader 的載入位址，但並非所有 bootloader 都支援這種方式，因此這個練習要設計一個能將自己擺放到其他位址，並且將核心載入到與自己原先位址的 bootloader。這樣的 bootloader 使我們不需要在 `config.txt` 中加入 `kernel_address=` 選項。

## Advanced Exercise 2 - Devicetree - 30%

開機時，核心需要知道目前有哪些裝置連接，並利用對應的驅動程式來初始化與存取這些裝置。對於強大 bus (e.g., PCIe 跟 USB)，核心可以透過讀取匯流排 (bus) 的暫存器來偵測連接的裝置。接下來，核心會將偵測到的裝置與驅動程式做匹配，並使用相容的驅動程式來初始化裝置。

對於只有簡易匯流排的系統，核心可能無法動態偵測到連接的裝置。其中一種解決方式是像 lab1 一樣，由於開發者知道開發的機器是什麼，因此直接將該裝置的 IO 記憶體位址紀錄在核心中。但這樣的作法缺乏可移植性。

為了解決上述問題，引入了 **devicetree** 的概念。devicetree 為一種描述描述系統硬體的檔案，它紀錄了系統中的裝置，裝置特性與裝置之間的關係。核心可以透過 devicetree 以類似強大匯流排系統的動態偵測機制的方式載入正確的驅動程式。

以下為兩個 devicetree 的應用場景 ([Ref.](http://www.wowotech.net/device_model/dt_basic_concept.html)):

* [PCI (Peripheral Component Interconnect)](https://tldp.org/LDP/tlk/dd/pci.html) 裝置:
  * PCI 為一種標準化的匯流排，通常可以動態偵測到連接的裝置。
  * 如果負責管理 PCI 匯流排的 [PCI bridge](https://ithelp.ithome.com.tw/articles/10363456) 無法被偵測，就需要在 devicetree 中提供相關資訊。
  * Bootloader 可能會偵測 PCI 裝置並產生包含偵測結果的 devicetree，然後將其傳遞給 OS。
* USB 裝置通常可以被動態偵測，但 SoC 上的 usb host controller 無法被動態偵測，因此需要在 devicetree 中描述。

### Format

Devicetree 有以下兩種格式:

* devicetree source (dts): 用人類可讀的格式描述 devicetree，會再被編譯成 flattened devicetree 使較慢的嵌入式系統可以用簡單與快速一點的方式做 parsing。
* Flattened devicetree (dtb): 可以透過自行手動編譯取得或是去 raspberry pi 的[官方 github](https://github.com/raspberrypi/firmware/raw/master/boot/) 下載[現成的檔案](https://github.com/raspberrypi/firmware/raw/master/boot/bcm2710-rpi-3-b-plus.dtb)。

Devicetree 為一個描述系統硬體的樹狀結構。開機程式會將一個 devicetree 載入給使用者程式的記憶體，並傳遞一個指向 devicetree 的指標給使用者。

### Parsing

這個練習需要實做一個解析 flatenned devicetree 的解析器。除此之外，核心應該要提供一個能接收 callback function 引數的界面，使驅動程式可以遍歷整個 devicetree 去查詢每個裝置節點，並透過檢查節點名稱和屬性去做匹配。以下為一個簡單的界面範例:

```C
void initramfs_callback(...) {
  ...
}

int main() {
  fdt_traverse(initramfs_callback);
}
```

Flattened devicetree 的詳細格式可以參考[官方規格書](https://www.devicetree.org/specifications/)。依照第 5, 2, 3 章的順序閱讀後實做 dtb 解析器。

### Dtb loading

Bootloader 會把 dtb 載入到記憶體並把載入位址指定給暫存器 `x0` 後傳遞給核心。除此之外，bootloader 會修改 dtb 檔原本的內容去匹配實際機器的設置。舉例來說，如果要 bootloader 載入一個初始 ramdisk，bootloader 便會新增 ramdisk 的初始載入位址到 dtb。

# Note

## Linker

為了規劃空間給 allocator，因此新增了 .heap 與 .stack section 並紀錄 sections 的起始位址。除此之外，也在 linker script 中設置代表 heap 與 stack 的可使用空間大小的變數 `_heap_stack_size`。

* 使用 `K` 與 `M` 後綴的變數分別會乘以 1024 與 1024*1024。([Ref.](https://sourceware.org/binutils/docs/ld/Constants.html))

  ```ld
  _heap_stack_size = 256K;
  ```

* GNU linker 支援 `+=` 指定運算，其餘支援的指定運算子可以參考[官方文件](https://sourceware.org/binutils/docs/ld/Simple-Assignments.html)。

## Boot Code

`.extern`: 僅僅是為了與其他組譯器相容而出現的指示詞，實際上會被忽略。GNU assembler 會將所有未定義的符號視為外部符號。([Ref.](https://ftp.gnu.org/old-gnu/Manuals/gas/html_chapter/as_7.html#SEC89))

* `.extern dtb_addr`: 由於要將 UART bootloader 傳遞給我們的 devicetree 載入位址的資訊存放到外部符號 (`dtb_addr`)，因此原先打算以此指示詞宣告外部符號。
* 除了 `extern` 以外，也可以使用 `IMPORT`。 ([Ref.](https://developer.arm.com/documentation/dui0041/c/Assembler/Directives/IMPORT-or-EXTERN-directive))

決定好執行指令的核心後，接下來同樣先設置好 stack pointer 與初始化 .bss section。與之前不同的是，由於 `x0` 會帶有 devicetree 的載入位址資訊，因此在將 `x0` 資訊寫入特定變數後才跳去執行 C 程式。

<!-- [AArch64 registers list](https://developer.arm.com/documentation/dui0801/l/Overview-of-AArch64-state/Registers-in-AArch64-state)

.quad: 用來配置記憶體在當前所在的 section，並且定義記憶體的初始值。[Ref.](https://developer.arm.com/documentation/100067/0606/armclang-Integrated-Assembler-Directives/Data-definition-directives?lang=en) -->

## C Code

[static 與 inline 擺放順序建議](https://stackoverflow.com/questions/61714110/static-inline-vs-inline-static)

[static 概念釐清](https://medium.com/@hauyang/%E6%88%91%E6%9C%89%E6%89%80%E4%B8%8D%E7%9F%A5%E7%9A%84-static-inline-b363892b7450) (static 影響 visibility 的方式)

### shell

新增以下兩個指令來呼叫讀取 cpio 壓縮檔的相關功能:

* `ls`: 列出 cpio 壓縮檔中的檔案。
* `cat [FILE]`: 輸出 cpio 壓縮檔中的 `FILE` 檔的內容。

### cpio

#### Background Knowledge of cpio

[FreeBSD 的手冊](https://man.freebsd.org/cgi/man.cgi?query=cpio&sektion=5)

實做上 cpio 有幾種壓縮格式:
cpio 有許多種壓縮格式，例如 PWB format，New Binary Format 與 New ASCII Format 等等。各個 cpio 格式的架構大致上是依照以下概念去設計的:

* 把任意數量的檔案，目錄跟其他檔案系統物件 (e.g., symbolic links, device nodes, etc.) 聚集成一個 bytes stream。
* cpio 壓縮檔中的每個檔案系統物件都由一個包含基本 metadata 的標頭紀錄組成，緊隨標頭之後的是檔案入口的完整路徑名稱 (路徑名稱的長度會儲存在標頭中) 與檔案資料。(`{header: full filepath + filename: data}`)
* 標頭紀錄儲存一系列的整數值，這些整數值通常是根據 `struct stat` 的欄位定義設定的。不同壓縮方式的差異主要在儲存值的方式 (可能是二進位，八進位或十六進位)。
* 壓縮檔的結尾會透過具有 "TRAILER!!!" 路徑名稱的特殊紀錄標注。

#### Implementation of cpio Parser

依實驗規定，我使用了 **New ASCII Format** 壓縮檔案，並實做以下使用界面:

* `cpio_ls()`: 利用下方的 `parse_header()` 解析壓縮檔，並列印壓縮檔中的所有檔案名稱。
* `cpio_cat()`: 同樣利用下方的 `parse_header()` 解析壓縮檔，在找到符合要求的檔案後列印檔案內容。
* `parse_header()`: 解析傳入的 cpio 壓縮檔，解析過程如下:
    1. 首先檢查 magic number，New ASCII 格式的 magic number 為 `070701`。
    2. 取得檔名，並檢查是否已經讀取完所有檔案。
        * cpio 壓縮檔會以 `"TRAILER!!!"` 路徑名稱做結尾。
    3. 計算標頭起點到檔案內容的偏移量，並紀錄檔案內容的位置。
        * 須注意這個格式中的所有數字都用 8-byte 的十六進位表示，因此需要做轉換。
        * 須注意標頭加上路徑名稱的大小會是 4-byte 的倍數。
    4. 計算並紀錄檔案大小後，將指標移動到下一個檔案的標頭開頭以便後續利用。
        * 同樣要記得將數字從十六進位轉為十進位。

### dtb

#### Background Knowledge of Devicetree

Devicetree 是一種用於描述系統硬體的樹狀結構，主要用於嵌入式系統中，幫助作業系統識別和配置硬體裝置。它分為兩種格式:

* Devicetree Source (DTS): 用人類可讀的格式描述硬體結構，需再編譯成 Flattened Devicetree (DTB) 使較慢的嵌入式系統可以用較簡單且快速的方式解析。
  * Rpi3 位於 [linux repository](https://github.com/raspberrypi/linux/blob/rpi-5.10.y/arch/arm/boot/dts/bcm2710-rpi-3-b-plus.dts) 的 dts。
  * 除了 dts 以外，還有 dtsi (i 為 include)，是存放了裝置中較通用的描述，並且可以讓使用者 include，避免花時間重複定義。
* **Flattened Devicetree (DTB)**: 二進制格式，由 DTS 編譯而成，供系統直接解析。

根據[官方規格書](https://www.devicetree.org/specifications/)可知，DTB 格式的結構由節點 (node) 與屬性 (property) 組成。

* 節點 (Node): 描述一個硬體裝置或功能模組，節點之間形成樹狀結構。
  * 節點名稱格式為 `node-name@unit-address`，其中 `unit-address` 與 `reg` 屬性相關。
  * 常見節點包括 `/`，`/cpus`，`/memory` 與 `/chosen` 等。
* 屬性 (Properties): 每個節點包含多個屬性，用於描述裝置的特性。
  * 標準屬性包括 `compatible`，`model`，`reg` 與 `status` 等。
  * 屬性值可以是字符串、整數、陣列等。

本次實驗使用的 dtb 格式由以下部份組成:

* header: 包含 magic number，devicetree 大小與 structure block 的偏移等資訊。
* memory reservation block: 記錄系統中預留的記憶體區域，避免被作業系統覆寫。
* structure block: 描述 devicetree 的結構，由一系列 tokens 與資料組成。
* strings block: 存儲所有屬性名稱的字符串。

本次實驗中會使用到 header 與 structure block 的資訊，以下為這兩者的詳細介紹:

1. header: 由以下 C 結構組成，所有欄位都是以 **big-endian 格式儲存的 32-bit 整數**。

    ```C
    struct fdt_header {
        uint32_t magic;             /* shall contain the value 0xd00dfeed 
                                    (big-endian) */
        uint32_t totalsize;         /* shall contain the total size in bytes of 
                                    the devicetree data structure */
        uint32_t off_dt_struct;     /* shall contain the offset in bytes of the 
                                    structure block */
        uint32_t off_dt_strings;    /* shall contain the offset in bytes of the 
                                    strings block */
        uint32_t off_mem_rsvmap;    /* hall contain the offset in bytes of the 
                                    memory reservation block */
        uint32_t version;           /* shall contain the version of the devicetree 
                                    data structure */
        uint32_t last_comp_version; /* shall contain the lowest version of the 
                                    devicetree data structure with which the 
                                    version used is backwards compatible */
        uint32_t boot_cpuid_phys;   /* shall contain the physical ID of the 
                                    system’s boot CPU */
        uint32_t size_dt_strings;   /* shall contain the length in bytes of the 
                                    strings block section of the devicetree 
                                    blob */
        uint32_t size_dt_struct;    /* shall contain the length in bytes of the 
                                    structure block section of the devicetree 
                                    blob */
    };
    ```

2. structure block: **每個 token 都應該位於相對 dtb 起始位址 4-byte 倍數的偏移位置上**，並且**每個 tokens 都是以 big-endian 表示的 32-bit 整數**。以下為各 tokens 的介紹:
    1. `FDT_BEGIN_NODE` (`0x00000001`): 標記節點開始。
    2. `FDT_END_NODE` (`0x00000002`): 標記節點結尾。
    3. `FDT_PROP` (`0x00000003`): 描述節點屬性，後面會接著以下描述屬性的資訊的 C 結構:

        ```C
        struct {
            uint32_t len;
            uint32_t nameoff;
        }
        ```

        結構中的**兩個欄位都是 32-bit 的 big-endian 整數**。
          * `len` 代表屬性的值以 bytes 為單位的長度。
          * `nameoff` 代表移動到 strings block 所需的偏移值，儲存在 strings block 的屬性名稱是以 NULL 為結尾的字串。
          * `nameoff` 代表屬性名稱的偏移值。屬性名稱為一個以 NULL 為結尾的字串，儲存在 strings block 中。

        **屬性的值沒有對齊 32-bit 邊界時，會填補 zeroed padding bytes 使其對齊。**
    4. `FDT_NOP` (`0x00000004`): 以覆寫的方式刪除 devicetree 中的屬性 (`FDT_PROP`) 或 token。
    5. `FDT_END` (`0x00000009`): 標記 structure 結尾的 token。

以下為用 structure block 表示[本課程使用的開發板的 dts 檔](https://github.com/raspberrypi/linux/blob/rpi-5.10.y/arch/arm/boot/dts/bcm2710-rpi-3-b-plus.dts)的簡易範例:

```json
/ {
    compatible = "raspberrypi,3-model-b-plus", "brcm,bcm2837";
    model = "Raspberry Pi 3 Model B+";

    chosen {
        bootargs = "coherent_pool=1M 8250.nr_uarts=1 snd_bcm2835.       enable_compat_alsa=0 snd_bcm2835.enable_hdmi=1";
    };

    aliases {
        serial0 = &uart1;
        serial1 = &uart0;
        mmc1 = &mmcnr;
    };
};
```

* `FDT_BEGIN_NODE`: 代表根節點 (`/`) 的開始
  * `FDT_PROP`: 代表 `compatible` 屬性。
  * `FDT_PROP`: 代表 `model` 屬性。
  * `FDT_BEGIN_NODE`: 代表 `chosen` 節點的開始。
    * `FDT_PROP`: 代表 `bootargs` 屬性。
  * `FDT_END_NODE`: 代表 `chosen` 節點的結束
  * `FDT_BEGIN_NODE`: 代表 `aliases` 節點的開始。
    * `FDT_PROP`: 代表 `serial0` 屬性。
    * `FDT_PROP`: 代表 `serial1` 屬性。
    * `FDT_PROP`: 代表 `mmc1` 屬性。
  * `FDT_END_NODE`: 代表 `aliases` 節點的結束。
* `FDT_END_NODE`: 代表 `/` 節點的結束。
* `FDT_END`: 代表 structure block 的結樹。

#### Implementation of dtb

首先檢查 `fdt_header` 的 `magic` 值，須注意 `ftd_header` 所有的欄位都使用 big-endian 格式除存，因此需要將數字轉成 litte-endian 後再做使用。轉換方式除了使用 gcc 提供的內建函數 `__bswap32` 外，也可以使用以下程式:

```C
uint32_t bswap_32(uint32_t x) {
    return ((x & 0xFF000000) >> 24) |
        ((x & 0x00FF0000) >> 8) |
        ((x & 0x0000FF00) << 8) |
        ((x & 0x000000FF) << 24);
}
```

以下為 `__bswap32` 實際使用的案例:

* `htonl()`: [Ref. 1](https://github.com/lattera/glibc/blob/master/inet/htonl.c), [Ref. 2](https://stackoverflow.com/questions/21527957/htonl-vs-builtin-bswap32)

接下來根據 `fdt_header` 中的 `size_dt_struct` 欄位取得 structure 與 string block 的位址後開始遍歷 devicetree。

* 根據 [linux kernel 官方文件](https://www.kernel.org/doc/Documentation/devicetree/bindings/chosen.txt)可知，devicetree 中的 `chosen` node 是一塊讓軔體與 OS 互相傳輸資料的空間，而帶有 `linux,initrd-start` 屬性的 node 會存放由 bootloader 載入的 initrd 的實體位址。
  * [Linux kernel documentation of devicetree](https://kernel.org/doc/Documentation/devicetree/)
  * [linux,initrd-start Ref.](https://stackoverflow.com/questions/73974443/how-does-the-linux-kernel-know-about-the-initrd-when-booting-with-a-device-tree)

<!-- ## makefile -->
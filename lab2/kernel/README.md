# Note

## Linker

為了規劃空間給 allocator，因此新增了 .heap 與 .stack section 並紀錄 sections 的起始位址。除此之外，也在 linker script 中設置代表 heap 與 stack 的可使用空間大小的變數 `_heap_stack_size`。

* 使用 `K` 與 `M` 後綴的變數分別會乘以 1024 與 1024*1024。([Ref.](https://sourceware.org/binutils/docs/ld/Constants.html))

```ld
_heap_stack_size = 256K;
```

GNU linker 支援 `+=` 指定運算，其餘支援的指定運算子可以參考[官方文件](https://sourceware.org/binutils/docs/ld/Simple-Assignments.html)。

<!-- ## Boot Code -->

<!-- > 程式執行流程與 lab1 類似，但最後跳去執行程式的位址是 bootloader - 0x20000 的地方。

.global: 讓接在後方的符號能被 linker 看到。([Ref. 1](https://ftp.gnu.org/old-gnu/Manuals/gas/html_chapter/as_7.html#SEC93), [Ref. 2](https://developer.arm.com/documentation/100068/0622/Migrating-from-armasm-to-the-armclang-Integrated-Assembler/Miscellaneous-directives?lang=en))

.end: 標注組語程式結尾。([Ref.](https://developer.arm.com/documentation/100068/0622/Migrating-from-armasm-to-the-armclang-Integrated-Assembler/Miscellaneous-directives?lang=en)) -->

<!-- ## Boot Code

[AArch64 registers list](https://developer.arm.com/documentation/dui0801/l/Overview-of-AArch64-state/Registers-in-AArch64-state)

.quad: 用來配置記憶體在當前所在的 section，並且定義記憶體的初始值。[Ref.](https://developer.arm.com/documentation/100067/0606/armclang-Integrated-Assembler-Directives/Data-definition-directives?lang=en) -->

## C Code

[static 與 inline 擺放順序建議](https://stackoverflow.com/questions/61714110/static-inline-vs-inline-static)

[static 概念釐清](https://medium.com/@hauyang/%E6%88%91%E6%9C%89%E6%89%80%E4%B8%8D%E7%9F%A5%E7%9A%84-static-inline-b363892b7450) (static 影響 visibility 的方式)

### shell

新增以下兩個指令來呼叫讀取 cpio 壓縮檔的相關功能:

* `ls`: 列出 cpio 壓縮檔中的檔案。
* `cat [FILE]`: 輸出 cpio 壓縮檔中的 `FILE` 檔的內容。

### cpio

[FreeBSD 的手冊](https://man.freebsd.org/cgi/man.cgi?query=cpio&sektion=5)

實做上 cpio 有以下幾種壓縮格式:

* PWB format
* New Binary Format
* Portable ASCII Format
* New ASCII Format
* New CRC Format
* HP variants

各個 cpio 壓縮檔格式的架構大致上是依照以下概念去設計的:

* 把任意數量的檔案，目錄跟其他檔案系統物件 (e.g., symbolic links, device nodes, etc.) 聚集成一個 bytes stream。
* cpio 壓縮檔中的每個檔案系統物件都由一個包含基本 metadata 的標頭紀錄組成，緊隨標頭之後的是檔案入口的完整路徑名稱 (路徑名稱的長度會儲存在標頭中) 與檔案資料。(`{header: full filepath + filename: data}`)
* 標頭紀錄儲存一系列的整數值，這些整數值通常是根據 `struct stat` 的欄位定義設定的。不同壓縮方式的差異主要在儲存值的方式 (可能是二進位，八進位或十六進位)。
* 壓縮檔的結尾會透過具有 "TRAILER!!!" 路徑名稱的特殊紀錄標注。

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

<!-- ### dtb

首先檢查 `ftd_header` 的 `magic` 值，須注意 `ftd_header` 所有的欄位都使用 big-endian 格式除存，因此需要將數字轉成 litte-endian 後再做使用。轉換方式除了自行定義外，也可以使用 gcc 提供的內建函數 `__bswap32`。

* 根據 chatgpt 的說法，`__bswap32` 這個 gcc 提供的內建函數可以通過 gcc 編譯成能在 ARM64 架構下運行的執行檔。
* 除此之外，也可以使用以下程式

    ```C
    uint32_t bswap_32(uint32_t x) {
        return ((x & 0xFF000000) >> 24) |
            ((x & 0x00FF0000) >> 8) |
            ((x & 0x0000FF00) << 8) |
            ((x & 0x000000FF) << 24);
    }
    ```

    或[此連結](https://www.ffmpeg.org/doxygen/0.6/bswap_8h-source.html)的程式實做同樣功能。

    [htonl](https://github.com/lattera/glibc/blob/master/inet/htonl.c)
    [htonl2](https://stackoverflow.com/questions/21527957/htonl-vs-builtin-bswap32) -->

<!-- ## makefile -->

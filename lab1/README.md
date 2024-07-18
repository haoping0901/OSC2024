# Lab 1: Hello World

這個 lab 會練習再沒有作業系統的環境下 (bare metal) 開發程式，藉此讓我們了解以下知識:

1. Linker script: 決定程式該如何擺放到記憶體上。
2. Assembly: 在開始執行程式前先初始化特定資料 (e.g., 全域變數和靜態變數)。
3. mini UART (UART1): 讓主機 (e.g., 我們開發程式的電腦) 跟 rpi3 互相溝通的管道。
4. Mailbox: SoC 的各個核心 (e.g., CPU 跟 GPU) 之間通訊的方法

## Basic Exercise 1 - Basic Initialization - 20%

### Linker Scripts

在 bare metal programming 中由於沒有 OS 的幫忙，因此需要自己撰寫 linker script 決定資料擺放在記憶體上的位置。以下為 `start.S` 中一些指令的解釋。

`ENTRY(_start)`: 根據[官方文件](https://sourceware.org/binutils/docs/ld/Entry-Point.html)可知，這個命令設置程式第一個執行的指令。

* 根據[這篇文章](https://stackoverflow.com/questions/40606700/what-does-entry-mean-in-a-linker-script)的說法，許多微控制器會有預設的程式起始位置 (舉個例子，課程提供的[外部資源](https://github.com/bztsrc/raspi3-tutorial/tree/master/01_bareminimum)跟[這篇文章](https://forums.raspberrypi.com/viewtopic.php?t=328000)都提到 AArch64 處理器的預設程式起始位址為 0x80000)，這個命令的功能偏像是提供一些像是用來除錯的額外資訊，有沒有加實際上不會影響到執行結果。

`. = 0x80000`: 如同上面提到的，由於 AArch64 處理器預期的內核程式碼擺放的位址位址為 0x80000，因此這邊一開始就先將 location counter 移動到這個位址去擺放要被執行的指令。

根據課程提供的[外部資源](https://github.com/bztsrc/raspi3-tutorial/blob/master/02_multicorec/link.ld)，我們分別設置了以下三個 output sections。

* 在 jserv 的[教材](https://wiki.csie.ncku.edu.tw/embedded/Lab19/stm32-prog.pdf)中提到 ARM 的 cross compiler 一般會有 `.text`, `.data`, `.rodata`, `.bss`, `.comment` 與 `ARM.attributes` 等不同 sections，其中 `.comment` 與 `ARM.attributes` 這兩個 sectios 是提供便於使用 debugger 的資訊。
* `.text`: 在這個 output section 中會放入所有 input files 中的 `*.text, *.text.*` 與 `*.rodata` sections。
  * 其中 `KEEP(*(.text.boot))` 根據 [GNU 官方文件](https://sourceware.org/binutils/docs/ld/Input-Section-Keep.html#Input-Section-Keep)所說，如果 link-time garbage collection 這個功能被啟用時，可以透過這個 input section description 來標注特定 section，使這個 section 不會被回收掉。
* `.data`: 將所有 input files 中的 `*.data` sections 放到這個 output section 後，接下來會去對齊這個 output section 擺放
* `.bss`: 根據[這篇文章](https://pinglinblog.wordpress.com/2016/10/18/linux-%E7%A8%8B%E5%BA%8F%E7%9A%84-memory-layout-%E5%88%9D%E6%B7%BA%E8%AA%8D%E8%AD%98/)可以知道這個 output section 會放入程式中未初始化的全域變數或靜態變數，因此在對齊這個 output section 的起始位址後會將所有 input files 中的 `*.bss` 與 `*COMMON` sections 放到這個 output section。
  * `ALIGN(exp)`: 根據[官方手冊](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_14.html)可以知道這個內建函式會計算並回傳目前位置對齊到下一個 `exp` 邊界的位置。由於我們後續會使用 64-bit 的零暫存器 ([xzr register](https://developer.arm.com/documentation/den0024/a/ARMv8-Registers/AArch64-special-registers)) 初始化，為了讓初始化的過程更加方便，我們便把這個 output section 的擺放位址對齊到 8 的倍數(可以參考[這篇文章](https://stackoverflow.com/questions/8458084/align-in-linker-scripts)知道 `ALIGN(n)` 會增加 padding bytes 直到目前的位址是 n 的倍數)。
  * `COMMON`: 關於 `COMMON` 符號的使用可參考 [GNU 官方手冊](https://sourceware.org/binutils/docs/ld/Input-Section-Common.html)的介紹。另外，根據[這篇文章](https://swaywang.blogspot.com/2012/06/elfbss-sectioncommon-section.html)可以知道 `COMMON` section 的主要用途在於讓 linker 合併同名的未初始化全域變數 (也可參考[此篇文章](https://stackoverflow.com/questions/16835716/bss-vs-common-what-goes-where)的回覆)。

擺放好各個 output sections 後會計算 `.bss` section 的大小，並在初始化的時候使用這個資訊。

* 計算大小所使用的函式可以參考[官方手冊](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_14.html)。

### Boot Code

決定好記憶體怖局後，接下來要在程式開始執行前先初始化像是全域變數跟靜態變數等變數。

首先參考課程提供的[外部資源](https://github.com/bztsrc/raspi3-tutorial/tree/master/02_multicorec)可以知道須要考慮到 Raspberry Pi Model 3 B+ 中的 CPU 為四核心，因此要決定程式要在哪顆核心上執行。

* 在開始撰寫組語前，可以先參考[這篇文章](https://winddoing.github.io/post/7190.html)初步了解 ARMv8-A 架構中的 64-bit 指令集。

`.section ".text.boot"`: 根據[官方文件](https://sourceware.org/binutils/docs/as/Section.html)可以知道這個指示詞會把接下來的程式碼聚集到 `.text.boot` section 中。

* 根據[官方文件](https://sourceware.org/binutils/docs/as/Section.html)可以知道把 section name 引用是為了增加相容性。

`_start` label:

* `mrs x0, mpidr_el1`: 讀取 CPU ID，讀取方式可參考[官方文檔](https://developer.arm.com/documentation/ddi0500/j/System-Control/AArch64-register-descriptions/Multiprocessor-Affinity-Register?lang=en)。
  * 官方文檔對 `mpidr_el1` 暫存器的解釋為
  
    > Provides an additional core identification mechanism for scheduling purposes in a cluster system.

    其中關於 cluster system 的定義根據[這篇文章](https://blog.csdn.net/djkeyzx/article/details/132270341)可得知[官方文檔](https://developer.arm.com/documentation/den0024/a/Multi-core-processors/Multi-processing-systems?lang=en)提到說一個各個核心可以獨立執行指令的多核處理器，像是 Cortex-A53，可以被視為一個 cluster。

* `and x0, x0, #0xff`: 用來找出執行中的核心是哪顆 (暫存器 [7:0] 用來表示 Cortex-A53 處理器的核心數)。
* `cbz x0, master`: 讓第 0 顆核心執行程式時跳去初始化 bss section 的 master label (指令使用方式可參考[官方文件](https://developer.arm.com/documentation/den0024/a/The-A64-instruction-set/Flow-control))。

`hang` label: 如果不是第 0 顆核心的話就讓核心進入低功耗狀態，並且確保就算意外接收到其他核心發起的 event 也會變回低功耗狀態。

* `wfe`: 用來把核心變為低功耗狀態的指令，可參考[官方文檔](https://developer.arm.com/documentation/den0024/a/Power-Management/Assembly-language-power-instructions)與[這篇文章](http://www.wowotech.net/armv8a_arch/wfe_wfi.html)。

`core0` label:

* `ldr x0, =_start`: 根據[官方文件](https://developer.arm.com/documentation/dui0379/e/writing-arm-assembly-language/load-addresses-into-registers)會把 `_start` label 的位址載入到 x1 暫存器中。
  * 於 `_start` label 是否有加等號的差異在於有加等號的程式會載入 label 的位址，沒有加等號的程式會把 label 指向的位址中存放的值載入。([參考網址](https://www.cnblogs.com/blogernice/articles/13840178.html))
* `mov sp, x0`: 設定 stack 的位址
* 把 linker script 中的 `.bss` section 起始位址資訊載入到 `x0` 暫存器，`.bss` section 大小資訊載入到 `w1` 暫存器中，以便接下來初始化 `.bss` section。

`init_bss` label:

* `cbz w1, exec_c_prog`: 判斷仍要初始化的空間，為 0 的話就跳去 label `exec_c_prog`。
* `str xzr, [x0], #8`: 仍有空間要被初始化的話就把目前的位址初始化為 0 (把 xzr 暫存器載入到 x0 暫存器)，並更新目前的位址 (把 x0 暫存器加 8)。(參考[官方文件](https://developer.arm.com/documentation/den0024/a/The-A64-instruction-set/Memory-access-instructions/Specifying-the-address-for-a-Load-or-Store-instruction)中的 Table 6.9)

`exec_c_prog` label:

* `bl main`: 跳到 C 程式的 main 函式
  * `bl` 指令: 除了跳到其他指令的位址去執行還會把 `bl` 的下一道指令的位址紀錄到 Link Register (LR) 中，以便之後可能可以透過 LR 暫存器跳回來繼續執行。([參考網站](https://stackoverflow.com/questions/34091898/bl-instruction-arm-how-does-it-work))

`.end`: [Arm 官方文件](https://developer.arm.com/documentation/100068/0622/Migrating-from-armasm-to-the-armclang-Integrated-Assembler/Miscellaneous-directives?lang=en)，[台大課程投影片](https://www.csie.ntu.edu.tw/~cyy/courses/assembly/12fall/lectures/handouts/lec10_ARMasm.pdf)與[這篇文章](https://stackoverflow.com/questions/22486532/the-end-directive-in-assembly-language)都提到這個指令是被用來標記組語檔案的結尾。

## Basic Exercise 2 - Mini UART - 20%

接下來，為了讓主機 (e.g., 我們開發程式的電腦) 能跟 rpi3 互相通訊，因此我們使用 mini UART (UART1) 把傳輸資料。

使用 UART1 前，因為 rpi3 會透過 MMIO (memory mapped input output) 的方式存取週邊裝置，因此要先把各個記憶體位址設定成對應的週邊裝置 (`gpio.h`)。

* 設定方式可以參考[外部資源](https://github.com/bztsrc/raspi3-tutorial/blob/master/03_uart1/gpio.h)與課程提供的[官方文檔](https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf)。

接下來，參考[外部資源](https://github.com/bztsrc/raspi3-tutorial/blob/master/03_uart1/uart.c)設計傳輸資料的程式。

## Basic Exercise 3 - Simple Shell - 20%

不斷接收輸入字元，並在接收到換行的時候進行字元判斷。

* 在處理換行時要加上回車 (carriage return) 字元，如果沒加上的話會只往下一行而已，不會回到開頭。

## Basic Exercise 4 - Mailbox - 20%

### Background

在 rpi3 中，mailboxes 的主要功能是促進 ARM 處理器和 VideoCore (GPU) 之間的通訊。這個機制包含了以下三個部份:

1. Mailbox registers: Mailbox 暫存器是透過 MMIO 去存取的，在這個實驗我們僅需要讀取 Mailbox 0 Read/Write (CPU 讀取 GPU)，Mailbox 0 status (檢查 GPU 的狀態) 跟 Mailbox 1 Read/Write (CPU 寫到 GPU)。
    * 在樹莓派的官方 [github](https://github.com/raspberrypi/firmware/wiki/Mailboxes) 上提供了不同 mailboxes 的 registers offsets。
2. Channels: 根據 [rpi3 官方 github](https://github.com/raspberrypi/firmware/wiki/Mailboxes) 可以知道 Mailbox 0 定義了許多通道，在這個 lab 我們只使用通道 8 (CPU/ARM -> GPU/VideoCore)。
    * 由於只有 mailbox 0 可以產生中斷給 ARM，因此 mailbox 0 會用來讓 videocore 傳遞訊息給 ARM。
3. Message: 為了用 mailbox 傳遞與接收訊息，我們需要遵照以下步驟準備一個陣列存放要傳遞的訊息 (參考[外部資源](https://github.com/bztsrc/raspi3-tutorial/blob/master/04_mailboxes/mbox.c))。
    1. Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    2. Check if Mailbox 0 status register’s full flag is set.
    3. If not, then you can write to Mailbox 1 Read/Write register.
    4. Check if Mailbox 0 status register’s empty flag is set.
    5. If not, then you can read from Mailbox 0 Read/Write register.
    6. Check if the value is the same as you wrote in step 1.

### Board Revision

在[此網站](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)中可以知道 ARM memory 相關資訊的 tag 為 `0x00010005`，並且回覆長度為 4 個 bytes。

### ARM memory base address

在[此網站](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)中可以知道 ARM memory 相關資訊的 tag 為 `0x00010005`，並且回覆長度為 8 個 bytes。

## Advanced Exercise 1 - Reboot - 30%

直接使用課程提供的程式碼即可。

## Makefile

`$(notdir $(SRCS:%.c=%.o))`: 將所有產生的目標檔的目錄部份去掉。(參考 [GNU 官方手冊](https://www.gnu.org/software/make/manual/html_node/File-Name-Functions.html))

`OBJS = $(SRCS:.c=.o)`: 據[官方手冊](https://www.gnu.org/software/make/manual/make.html#Substitution-Refs)可知這段指令會把變數 `SRCS` 尾端的每個 `.c` 替換成 `.o`。

以下為 [GNU 官方手冊](https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html)中關於 automatic variables 的介紹:

* `$@`: 目的檔檔名。
* `$<`: 第一個依賴檔檔名。
* `$?`: 所有比目標檔還新的依賴檔案，各個檔名中有空白。
* `$^`: 所有依賴檔檔名，各個檔名中有空白。
  
  也可以參考[這篇文章](https://jasonblog.github.io/note/gunmake/makefile_zhong_de_,_%5E,__,__fu_hao.html)。

`%.o: %.c`: 為每個以 `.c` 結尾的檔案規定一個會產生同檔名但以 `.o` 做結尾的目標檔的規則。(參考 [GNU 官方手冊](https://www.gnu.org/software/make/manual/html_node/Pattern-Match.html))

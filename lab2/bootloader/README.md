# Note

## Linker

[GNU linker 官方文件連結](https://sourceware.org/binutils/docs/ld/index.html#SEC_Contents)

與 lab1 不同的是，連結器 (linker) 將 0x60000 設置為用來擺放我們定義的 bootloader 的位址。

`PROVIDE()`: 只在被引用或其他連結物件沒有定義的時候才定義符號 (symbol)。 ([Ref.](https://sourceware.org/binutils/docs/ld/PROVIDE.html))

`(NOLOAD)`: 用來標記一個 section 在執行時期不用去載入的指示詞。([Ref.](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_21.html))

* 由於 .bss section 存放的變數值預設都會初始化為 0，因此不需要將內容紀錄於執行檔，**除了可以減少執行檔大小以外，也可以減少載入執行檔的時間**。

## Boot Code

程式執行流程與 lab1 類似，但避免使用 lab1 使用到的 `x0` 暫存器，因為 `x0` 在後續的練習中會被用來傳遞其他資訊 (dtb loading address)。(參考[課程網站](https://nycu-caslab.github.io/OSC2024/labs/lab2.html#dtb-loading))

* 由於 `x0` 可能會在後續的函式呼叫中被用來傳遞參數與回傳值，因此在透過下方指令確認程式有使用到哪些暫存器後，我事先將 `x0` 的值存放到未被使用的 `x10` 暫存器，後續載入完核心後再將 `x10` 的值存回 `x0` 並跳去執行載入的核心。

    ```Bash
    aarch64-linux-gnu-objdump -d bootloader.elf
    ```

.global: 讓接在後方的符號能被 linker 與其他一起連結的程式看到。([Ref. 1](https://ftp.gnu.org/old-gnu/Manuals/gas/html_chapter/as_7.html#SEC93), [Ref. 2](https://developer.arm.com/documentation/100068/0622/Migrating-from-armasm-to-the-armclang-Integrated-Assembler/Miscellaneous-directives?lang=en))

## C Code

### main

先檢查是否重新擺放過自己設計的 bootloader，還沒的話就去重新擺放 bootloader，並在結束後跳去執行重新擺放的位址上的 bootloader。這個時候用來確認擺放狀態的變數 (`had_relocated`) 已經更新了，因此會直接開始執行 shell。

### shell

shell 中新增了載入 kernel 的功能，透過在命令列中輸入 load 可以讓板子開始透過 UART 接收實際要測試的核心，最後跳去載入核心的位址執行。

在接收並載入完透過 UART 傳入的核心後，會將在 boot code 中儲存在 `x10` 的 dtb loading address 載入回 `x0`，並將新載入的核心位址 (`0x80000`) 載入到用來儲存回傳位址的 link register (i.e., `x30`. ([Ref.](https://developer.arm.com/documentation/dui0801/l/Overview-of-AArch64-state/Link-registers)))。

* `__asm__`: 為 GNU extension 中 `asm` 關鍵字的替代版本。由於使用特定選項 (e.g., `-ansi` 跟 `-std`) 時會禁用 `asm`，`typeof` 跟 `inline` 等關鍵字，因此產生了這個替代關鍵字來避免問題。([Ref. 1](https://gcc.gnu.org/onlinedocs/gcc-12.1.0/gcc/Extended-Asm.html), [Ref. 2](https://gcc.gnu.org/onlinedocs/gcc-12.1.0/gcc/Alternate-Keywords.html#Alternate-Keywords))

## makefile

$(wildcard pattern...): 用一個以空白隔開的列表替換，列表中包含符合 pattern 的檔案名稱。([Ref.](https://www.gnu.org/software/make/manual/html_node/Wildcard-Function.html))

.PHONY: 讓 `make recipe` 不用檢查依賴檔案的修改時間，確保每次都會執行。([Ref.](https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html))

* 如果在 makefile 存在的目錄下有與 `recipe` 同名稱的檔案，`make recipe` 會去檢查這個 `recipe` 依賴檔案的最後修改時間是否比 target 晚，由於有些 target 後方不會接著依賴檔案 (e.g., clean)，因此這樣的 target 會因為永遠比依賴檔案新而不執行。

```makefile
run:
    qemu-system-aarch64 -machine raspi3b -kernel bootloader.img -display none -serial null -serial stdio
```

* `-serial null -serial stdio`: 將 UART1 (mini UART) 重新導向 (redirect) 到 `stdio`。因為 `UART1` 預設是不會被 qemu 重新導向到終端機的，因此需要用兩個 `-serial`。([Ref.](https://github.com/bztsrc/raspi3-tutorial/blob/master/README.md#emulation))

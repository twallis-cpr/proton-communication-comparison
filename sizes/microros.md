## Per-archive contributions to ELF file

Source: `idf.py size-components` in `micro_ros/`. The two `.text` columns from
the tool are disambiguated below as `IRAM .text` (grouped under DIRAM, code
executed from RAM) and `Flash .text` (grouped under Flash Code, execute-in-place
from flash).

```
Executing "ninja size-components"...
[0/2] Re-checking globbed directories...
[0/1] cd /workspaces/proton-communication-comparison/micro_ros/build && /opt/esp/tools/cmake/4.0.3/bin/cmake -D "IDF_SIZE_TOOL=/opt/esp/python_env/idf6.0_py3.12_env/bin/python;-m;esp_idf_size" -D IDF_SIZE_MODE=--archives -D MAP_FILE=/workspaces/proton-communication-comparison/micro_ros/build/esp32_uros_bench.map -D OUTPUT_JSON= -P /opt/esp/idf/tools/cmake/run_size_tool.cmake
```

| Archive File | Total Size | DIRAM | .bss | .data | IRAM .text | Flash Code | .tdata | .init_array | .rodata | .appdesc | Flash .text | LP SRAM | .rtc_reserved |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| libnet80211.a | 194656 | 19545 | 10596 | 1571 | 7378 | 175111 | 0 | 0 | 20051 | 0 | 155060 | 0 | 0 |
| libtfpsacrypto.a | 117585 | 654 | 444 | 128 | 82 | 116931 | 0 | 4 | 16423 | 0 | 100504 | 0 | 0 |
| libpp.a | 111650 | 46762 | 2771 | 3201 | 40790 | 64888 | 0 | 0 | 8564 | 0 | 56324 | 0 | 0 |
| libmicroros.a | 107818 | 30523 | 24628 | 5895 | 0 | 77295 | 0 | 4 | 3075 | 0 | 74216 | 0 | 0 |
| liblwip.a | 99636 | 3733 | 3721 | 12 | 0 | 95903 | 0 | 0 | 3433 | 0 | 92470 | 0 | 0 |
| libwpa_supplicant.a | 73613 | 1371 | 1363 | 8 | 0 | 72242 | 0 | 0 | 1660 | 0 | 70582 | 0 | 0 |
| libesp_stdio.a | 61265 | 16 | 16 | 0 | 0 | 61249 | 0 | 8 | 60817 | 0 | 424 | 0 | 0 |
| libphy.a | 32029 | 5783 | 44 | 951 | 4788 | 26246 | 0 | 0 | 0 | 0 | 26246 | 0 | 0 |
| libesp_hw_support.a | 29587 | 10442 | 96 | 1616 | 8730 | 19121 | 0 | 28 | 2359 | 0 | 16734 | 24 | 24 |
| libc.a | 17924 | 4 | 0 | 4 | 0 | 17920 | 0 | 0 | 2412 | 0 | 15508 | 0 | 0 |
| libspi_flash.a | 16576 | 13845 | 6 | 4939 | 8900 | 2731 | 0 | 8 | 1147 | 0 | 1576 | 0 | 0 |
| libfreertos.a | 15692 | 5424 | 2244 | 16 | 3164 | 10268 | 0 | 0 | 1068 | 0 | 9200 | 0 | 0 |
| libnvs_flash.a | 14712 | 24 | 24 | 0 | 0 | 14688 | 0 | 0 | 136 | 0 | 14552 | 0 | 0 |
| libesp_system.a | 10246 | 1973 | 138 | 41 | 1794 | 8273 | 0 | 40 | 409 | 0 | 7824 | 0 | 0 |
| libesp_netif.a | 7664 | 193 | 189 | 4 | 0 | 7471 | 0 | 0 | 167 | 0 | 7304 | 0 | 0 |
| libcoexist.a | 6667 | 463 | 2 | 307 | 154 | 6204 | 0 | 0 | 1330 | 0 | 4874 | 0 | 0 |
| libesp_libc.a | 5672 | 3646 | 540 | 328 | 2778 | 2026 | 24 | 16 | 120 | 0 | 1866 | 0 | 0 |
| libesp_wifi.a | 4969 | 737 | 43 | 492 | 202 | 4232 | 0 | 4 | 790 | 0 | 3438 | 0 | 0 |
| libesp_hal_mspi.a | 4584 | 4032 | 0 | 72 | 3960 | 552 | 0 | 0 | 0 | 0 | 552 | 0 | 0 |
| libhal.a | 4028 | 2923 | 13 | 274 | 2636 | 1105 | 0 | 0 | 257 | 0 | 848 | 0 | 0 |
| libesp_event.a | 3785 | 4 | 4 | 0 | 0 | 3781 | 0 | 0 | 141 | 0 | 3640 | 0 | 0 |
| libvfs.a | 3589 | 236 | 44 | 192 | 0 | 3353 | 0 | 8 | 131 | 0 | 3214 | 0 | 0 |
| libheap.a | 3364 | 1183 | 8 | 13 | 1162 | 2181 | 0 | 40 | 401 | 0 | 1740 | 0 | 0 |
| libesp_mm.a | 3185 | 629 | 48 | 71 | 510 | 2556 | 0 | 0 | 166 | 0 | 2390 | 0 | 0 |
| libesp_driver_uart.a | 3051 | 116 | 28 | 88 | 0 | 2935 | 0 | 8 | 279 | 0 | 2648 | 0 | 0 |
| libesp_phy.a | 2952 | 395 | 44 | 1 | 350 | 2557 | 0 | 0 | 299 | 0 | 2258 | 0 | 0 |
| libesp_timer.a | 2941 | 1266 | 36 | 16 | 1214 | 1675 | 0 | 16 | 75 | 0 | 1584 | 0 | 0 |
| libesp_driver_dma.a | 2698 | 86 | 0 | 12 | 74 | 2612 | 0 | 0 | 102 | 0 | 2510 | 0 | 0 |
| libesp_hal_security.a | 2454 | 0 | 0 | 0 | 0 | 2454 | 0 | 0 | 32 | 0 | 2422 | 0 | 0 |
| libesp_driver_usb_serial_jtag.a | 2170 | 157 | 17 | 48 | 92 | 2013 | 0 | 16 | 125 | 0 | 1872 | 0 | 0 |
| libesp_partition.a | 2160 | 8 | 8 | 0 | 0 | 2152 | 0 | 0 | 168 | 0 | 1984 | 0 | 0 |
| libefuse.a | 1973 | 296 | 4 | 288 | 4 | 1677 | 0 | 24 | 417 | 0 | 1236 | 0 | 0 |
| liblog.a | 1918 | 720 | 276 | 8 | 436 | 1198 | 0 | 0 | 48 | 0 | 1150 | 0 | 0 |
| libesp_common.a | 1790 | 0 | 0 | 0 | 0 | 1790 | 0 | 0 | 1742 | 0 | 48 | 0 | 0 |
| libesp_security.a | 1694 | 20 | 20 | 0 | 0 | 1674 | 0 | 8 | 0 | 0 | 1666 | 0 | 0 |
| libmain.a | 1657 | 332 | 332 | 0 | 0 | 1325 | 0 | 0 | 9 | 0 | 1316 | 0 | 0 |
| libbench_common.a | 1630 | 8 | 8 | 0 | 0 | 1622 | 0 | 0 | 40 | 0 | 1582 | 0 | 0 |
| libstdc++.a | 1606 | 25 | 17 | 8 | 0 | 1581 | 0 | 4 | 195 | 0 | 1382 | 0 | 0 |
| libesp_driver_gpio.a | 1381 | 0 | 0 | 0 | 0 | 1381 | 0 | 0 | 177 | 0 | 1204 | 0 | 0 |
| libriscv.a | 1290 | 1108 | 256 | 0 | 852 | 182 | 0 | 0 | 50 | 0 | 132 | 0 | 0 |
| libesp_rom.a | 1238 | 770 | 84 | 0 | 686 | 468 | 0 | 4 | 76 | 0 | 388 | 0 | 0 |
| libpthread.a | 1207 | 4 | 4 | 0 | 0 | 1203 | 0 | 0 | 89 | 0 | 1114 | 0 | 0 |
| libesp_hal_dma.a | 1092 | 342 | 0 | 12 | 330 | 750 | 0 | 0 | 0 | 0 | 750 | 0 | 0 |
| libesp_app_format.a | 770 | 10 | 10 | 0 | 0 | 760 | 0 | 8 | 0 | 256 | 496 | 0 | 0 |
| libbootloader_support.a | 756 | 666 | 0 | 0 | 666 | 90 | 0 | 0 | 40 | 0 | 50 | 0 | 0 |
| libsoc.a | 461 | 0 | 0 | 0 | 0 | 461 | 0 | 0 | 461 | 0 | 0 | 0 | 0 |
| libesp_hal_pmu.a | 454 | 116 | 0 | 0 | 116 | 338 | 0 | 0 | 32 | 0 | 306 | 0 | 0 |
| libesp_hal_ana_conv.a | 437 | 199 | 0 | 109 | 90 | 238 | 0 | 0 | 0 | 0 | 238 | 0 | 0 |
| libesp_coex.a | 436 | 158 | 0 | 80 | 78 | 278 | 0 | 8 | 0 | 0 | 270 | 0 | 0 |
| libcore.a | 366 | 9 | 9 | 0 | 0 | 357 | 0 | 0 | 43 | 0 | 314 | 0 | 0 |
| libesp_ringbuf.a | 265 | 0 | 0 | 0 | 0 | 265 | 0 | 0 | 45 | 0 | 220 | 0 | 0 |
| libapp_update.a | 214 | 4 | 4 | 0 | 0 | 210 | 0 | 0 | 30 | 0 | 180 | 0 | 0 |
| libesp_hal_wdt.a | 102 | 102 | 0 | 0 | 102 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libesp_hal_uart.a | 92 | 0 | 0 | 0 | 0 | 92 | 0 | 0 | 0 | 0 | 92 | 0 | 0 |
| libmesh.a | 64 | 4 | 4 | 0 | 0 | 60 | 0 | 0 | 0 | 0 | 60 | 0 | 0 |
| libcxx.a | 52 | 0 | 0 | 0 | 0 | 52 | 0 | 0 | 0 | 0 | 52 | 0 | 0 |
| libmbedtls.a | 32 | 0 | 0 | 0 | 0 | 32 | 0 | 8 | 0 | 0 | 24 | 0 | 0 |
| libesp_hal_gpio.a | 31 | 0 | 0 | 0 | 0 | 31 | 0 | 0 | 31 | 0 | 0 | 0 | 0 |
| libmbedtls.a | 22 | 0 | 0 | 0 | 0 | 22 | 0 | 0 | 0 | 0 | 22 | 0 | 0 |
| libespnow.a | 3 | 3 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| libnvs_sec_provider.a | 2 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 2 | 0 | 0 |

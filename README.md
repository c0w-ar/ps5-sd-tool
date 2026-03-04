# ps5-sd-tool

This payload let's you handle and work with PS5 Savedata (SD) files.

This is a CLI App. It offers the following features:

- Savedata file Mount and information visualization
- Savadata file Resign
- Export Savedata file to unencrypted folder
- Import Savedata file from unencrypted folder (with auto resign to selected local user)

This is WIP. Code is a mess.

PLEASE USE AT YOUR OWN RISK. A backup is always recommended before messing with savedata files. 

## Credits

[bucanero](https://github.com/john-tornblom): PS4 Apollo Save Tool (https://github.com/bucanero/apollo-ps4)

[john-tornblom](https://github.com/john-tornblom): PS5 SDK (https://github.com/ps5-payload-dev/sdk)

[earthonion](https://github.com/earthonion): PFS mounting reference

[echostretch](https://github.com/echostretch): PFS mounting reference

## Own compilation

First, install the ps5-payload-sdk

Second, download the SQLite Amalgamation C files (https://www.sqlite.org/amalgamation.html).  Drop them on "ps5-sd-tool/external/sqlite"

```
ps5-sd-tool
└── 📁 external
    └── 📁 sqlite
        └── 📄 sqlite3.c
        └── 📄 sqlite3.h
        └── 📄 sqlite3ext.h
        └── 📄 archiveXXXX.dat
```

Third, time to compile

```sh
make
```

## How to Use

Send the payload

```sh
socat -t 99999 - TCP:YOUR_PS5_IP:9021 < ps5-sd-tool.elf
```

Wait until you see the notification "waiting for client on port xxxx"

Connect to the port and use the CLI App

```sh
socat -t 99999 - TCP:YOUR_PS5_IP:6666
```

When you finish your session, the payload ends. If you wish to use it again, you need to send again the payload.
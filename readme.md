Server parameters:

 - `-p` - select port (eg. `-p 8080`)

Client parameters:

 - **[REQUIRED]** `filename` file to be sent (eg. `./document.txt`)
 - `-p` - select port (eg. `-p 8080`)
 - `-a` - select IP (eg. `-a 192.168.0.5`)
 
### Deploy info - Linux
Run this to get list of dependencies on linux. Copy all that are not in t`/lib/`
```
ldd server
	linux-vdso.so.1 (0x00007ffdb19b3000)
	libQt5Network.so.5 => /home/jakub/Qt/5.6/gcc_64/lib/libQt5Network.so.5 (0x00007fc81d804000)
	libQt5Core.so.5 => /home/jakub/Qt/5.6/gcc_64/lib/libQt5Core.so.5 (0x00007fc81d0ed000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007fc81ced0000)
	libstdc++.so.6 => /usr/lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007fc81cbc5000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007fc81c8c4000)
	libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007fc81c6ae000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fc81c303000)
	libz.so.1 => /lib/x86_64-linux-gnu/libz.so.1 (0x00007fc81c0e8000)
	libicui18n.so.56 => /home/jakub/Qt/5.6/gcc_64/lib/libicui18n.so.56 (0x00007fc81bc4e000)
	libicuuc.so.56 => /home/jakub/Qt/5.6/gcc_64/lib/libicuuc.so.56 (0x00007fc81b896000)
	libicudata.so.56 => /home/jakub/Qt/5.6/gcc_64/lib/libicudata.so.56 (0x00007fc819eb3000)
	libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007fc819caf000)
	libgthread-2.0.so.0 => /usr/lib/x86_64-linux-gnu/libgthread-2.0.so.0 (0x00007fc819aad000)
	librt.so.1 => /lib/x86_64-linux-gnu/librt.so.1 (0x00007fc8198a5000)
	libglib-2.0.so.0 => /lib/x86_64-linux-gnu/libglib-2.0.so.0 (0x00007fc819596000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fc81db64000)
	libpcre.so.3 => /lib/x86_64-linux-gnu/libpcre.so.3 (0x00007fc819328000)
```


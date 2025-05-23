dumpusn is a command line tool to display USN Journal events.\
\
Usage\
\
dumpusn.exe <volume> [file-reference-number]\
-or-\
dumpusn.exe <filename>\
\
<volume>\
The volume path.\
For example:\
dumpusn.exe C:\\
dumpusn.exe \\\\.\\C:\\
dumpusn.exe \\\\?\\Volume{00000000-0000-0000-0000-000000000000}\
\
[file-reference-number]\
The file reference number in hexidecimal (with 0x prefix) or decimal.\
The sequence number is ignored if 0.\
All USN journal events are shown if omitted.\
For example:\
dumpusn.exe C: 0x0005000000000005\
dumpusn.exe C: 5\
\
<filename>\
Show USN journal events for the specified file or folder.\
For example:\
dumpusn.exe C:\\windows\\explorer.exe\
\
Requires administrative privileges for low level read access to NTFS/ReFS volumes.\
\
\
\
Output:\
\
Reason Parent-file-reference-number Filename\
Reason Parent-file-reference-number Filename\
Reason Parent-file-reference-number Filename\
...\
\
\
\
Example usage:\
\
Show all USN events for the C: drive:\
dumpusn c:\\
\
Show USN events for the file c:\\windows\\explorer.exe:\
dumpusn c:\\windows\\explorer.exe\
\
Show USN events for the file 0x00810000001f4356:\
dumpusn c: 0x00810000001f4356\
\
\
![image](https://github.com/user-attachments/assets/4d5a2603-103e-47b3-a369-76e3d6c20996)\
\
dumpusn c: 0x00810000001f4356\

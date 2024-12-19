# networking
Send and receive files over the network via TCP and UDP. 

## To compile

Run `make`.

## To use

```
Usage: sender [ -h ] [ -b buffer_size ] [ -p TCP|UDP ] to_host port input_file
-h: Prepend a sequence number to each packet (type uint64). Ignored for TCP. Default: FALSE
-b: Size of network packet (excluding sequence number). Default: 4096
-p: Network protocol, TCP or UDP. Default: UDP
```

The `to_host` is a host name or ip address. The `input_file` is a file to send.
```
Usage: receiver [ -h ] [ -b buffer_size ] [ -p TCP|UDP ] port output_file
-h: Expect a sequence number prepended to each packet (type uint64). Cannot be used with TCP. Default: FALSE
-b: Size of network packet (excluding sequence number). Default: 4096
-p: Network protocol, TCP or UDP. Default: UDP
-k: VDIF STREAM ONLY. Peek at the input stream and report the first VDIF header. Do not write to file but exit. Default: FALSE
    VDIF frames must be sent individually in each network packet, so that the header is at the front of every packet.
```
The received data is written to `output_file`.

The sequence header is used to comply with [VTP protocol](https://vlbi.org/wp-content/uploads/2019/03/2012.10.16_VTP_0.9.7.pdf).

First, run `receiver` on the machine you want to send to. Example: `receiver 12345 out.txt`. Then run the sender on the machine you want to send from. Example: `sender 127.0.0.1 12345 bigfile.txt`. Make sure you use the same protocol (tcp/udp) on both machines. It may also be a good idea to use the same buffer size.

`sender` can be used to send content to  programs other than `receiver`. `sender` will terminate when all of the file is sent. When  `receiver` is used with TCP, it will terminate when `sender` terminates, because the socket is closed. When `receiver` is used with UDP, it will not terminate, and you have to Ctrl-C it. This is because UDP is not connection-oriented. A way round this would be to send some EOF marker to `receiver`, but such a marker may confuse other programs that you want to use with `sender`.


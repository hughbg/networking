# networking
Send and receive files over the network via TCP and UDP. 

## To compile

Run `make`.

## To use

```
sender -b buffer_size -p [tcp | udp] destination port input_file
Default buffer_size: 4096
Default protocol: udp
```

The `destination` is a host name or ip address. The `file` is a file to send.
```
receiver -b buffer_size -p [tcp | udp] port output_file
Default buffer_size: 4096
Default protocol: udp
```
The received data is written to `file`.

First, run `receiver` on the machine you want to send to. Example: `receiver 12345 out.txt`. Then run the sender on the machine you want to send from. Example: `sender 127.0.0.1 12345 bigfile.txt`. Make sure you use the same protocol (tcp/udp) on both machines.

`sender` can be used to send content to  programs other than `receiver`. `sender` will terminate when all of the file is sent. When  `receiver` is used with TCP, it will terminate when `sender` terminates, because the socket is closed. When `receiver` is used with UDP, it will not terminate, and you have to Ctrl-C it. This is because UDP is not connection-oriented. A way round this would be to send some EOF marker to `receiver`, but such a marker may confuse other programs that you want to use with `sender`.



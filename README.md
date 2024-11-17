# networking
Send and receive files over the network via TCP and UDP. 

## To compile

Run `make`.

## To use

```
sender -b buffer_size -p [tcp | udp] destination port file
Default buffer_size: 4096
Default protocol: udp
```

The `destination` is a host name or ip address. The `file` is a file to send.
```
receiver -b buffer_size -p [tcp | udp] destination port file
Default buffer_size: 4096
Default protocol: udp
```
The network input is written to `file`.

First, run `receiver` on the machine you want to send to. Example: `receiver 12345 out.txt`. Then run the sender on the machine you want to send from. Example: `sender 127.0.0.1 12345 bigfile.txt`. Make sure you use the same protocol (tcp/udp) on both machines.

`sender` can be used to send content to other programs, if they use the right protocol and content format. `sender` will terminate when all of the file is sent. The behaviour for `receiver` is a bit different. When the `receiver` is used with TCP, it will terminate when `sender` terminates, i.e. when all of the file is sent. The behaviour is different for UDP, `receiver` will continue to wait for a message and you have to ^C it. Not sure how to change that (yet).



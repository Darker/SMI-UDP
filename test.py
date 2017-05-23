import socket
host="127.20.120.56"
port=9666
udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
print(udp_sock.sendto(b'PING', (host, port)))
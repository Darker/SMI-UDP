import socket
host="127.0.0.1"
port=6660
udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_sock.sendto(b'PING', (host, port))
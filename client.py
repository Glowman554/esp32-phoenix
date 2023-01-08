import sys
import tqdm
import socket
import struct

host = sys.argv[1]
port = 9001  # socket server port number

client_socket = socket.socket()  # instantiate
client_socket.connect((host, port))  # connect to the server

CMD_READ = 0x1
CMD_WRITE = 0x2
CMD_SAVE = 0x3
CMD_HALT = 0x4
CMD_RST = 0x5

def write_cmd(cmd, addr, data, res = True):
	data = struct.pack("<BHB", cmd, addr, data)
	client_socket.send(data)
	if res:
		return client_socket.recv(1)

write_cmd(CMD_HALT, 0, 0)
with open(sys.argv[2], 'rb') as f:
	data = f.read()
	for i in tqdm.tqdm(range(0, len(data))):
		write_cmd(CMD_WRITE, i, data[i])
		pass
	for i in tqdm.tqdm(range(0, len(data))):
		if struct.pack("<B", data[i]) != write_cmd(CMD_READ, i, 0):
			print(f"Verify failed at {i}")

write_cmd(CMD_SAVE, 0, 0)
write_cmd(CMD_RST, 0, 0, False)


client_socket.close()  # close the connection

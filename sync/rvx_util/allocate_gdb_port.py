import os
import argparse
import socket
import stat

class PortManager():
	def __init__(self, range_file=None):
		assert range_file
		self.range_list = []
		with open(range_file,'r') as f:
			file_contents = f.read()
			range_line_list = file_contents.split('\n')
			for range_line in range_line_list:
				if range_line:
					range = range_line.split(' ')
					assert len(range)==2, range_line
					self.range_list.append((int(range[0]),int(range[1])))

	def print_port(self):
		for range in self.range_list:
			print(range)

	def import_file(self,allocated_file=None):
		assert allocated_file
		set_previous = False
		allocation_list = []
		if os.path.isfile(allocated_file):
			with open(allocated_file,'r') as f:
				file_contents = f.read()
				allocation_list = file_contents.split('\n')
				previous = allocation_list[0].split(' ')
				assert len(previous)==2, previous
				if previous[0].isdigit():
					self.previous = (int(previous[0]),int(previous[1]))
					set_previous = True
					allocation_list = allocation_list[1:]
		if not set_previous:
			self.previous = (-1,None)

		self.user2port = {}
		self.port2user = {}

		for allocation_line in allocation_list:
			if allocation_line:
				allocation = allocation_line.split(' ')
				assert len(allocation)==2, allocation_line
				assert allocation[1].isdigit(), allocation[1]
				port = int(allocation[1])
				self.user2port[allocation[0]] = port
				self.port2user[port] = allocation[0]

		#print(self.user2port)

	def __find_new(self):
		index, port = self.previous
		valid_previous = True
		init = False
		if index==(-1):
			valid_previous = False
		elif index >= len(self.range_list):
			valid_previous = False
		elif port < self.range_list[index][0]:
				valid_previous = False
		elif port > self.range_list[index][1]:
				valid_previous = False

		if not valid_previous:
			self.previous = (0, self.range_list[0][0])
		else:
			self.__go_next()

		while 1:
			port = self.previous[1]
			if port not in self.port2user:
				sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
				result = False
				try:
					sock.bind(("0.0.0.0", port))
					result = True
				except:
					pass
				sock.close()
				if result:
					break
			self.__go_next()

	def __go_next(self):
		index, port = self.previous
		if port==self.range_list[index][1]:
			if index==(len(self.range_list)-1):
				index = 0
			else:
				index += 1
			port = self.range_list[index][0]
		else:
			port+=1
		self.previous = (index, port)

	def allocate(self, username=None):
		port = self.user2port.get(username)
		if not port:
			self.__find_new()
			port = self.previous[1]
			self.user2port[username] = port
			self.port2user[port] = username
		return port

	def export_file(self,allocated_file=None):
		assert allocated_file
		exist = os.path.isfile(allocated_file)
		with open(allocated_file,'w') as f:
			f.write('{0} {1}'.format(self.previous[0],self.previous[1]))
			for username, port in self.user2port.items():
				f.write('\n{0} {1}'.format(username,port))
		if not exist:
			os.chmod(allocated_file, stat.S_IRUSR|stat.S_IWUSR|stat.S_IRGRP|stat.S_IWGRP|stat.S_IROTH|stat.S_IWOTH)

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Allocating GDB Port')
	parser.add_argument('-ws', help='workspace')
	parser.add_argument('-username', '-u', help='username')
	parser.add_argument('-output', '-o', help='output file')
	parser.add_argument('--append', action='store_true', help='append port to output file')

	args = parser.parse_args()
	assert args.ws
	assert args.username

	port_file = os.path.join(args.ws,'gdb_port_range.txt')
	allocated_file = os.path.join(args.ws,'gdb_port_allocated.txt')

	port_manager = PortManager(port_file)
	#port_manager.print_port()
	port_manager.import_file(allocated_file)
	port = str(port_manager.allocate(args.username))
	port_manager.export_file(allocated_file)
	if args.output:
		if args.append:
			with open(args.output,"a") as f:
				f.write('\n' + port)
		else:
			with open(args.output,"w") as f:
				f.write(port)
	else:
		print(port)

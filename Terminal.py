#!/usr/bin/env python3
import tkinter as tk
import serial, threading, re
microcontroler_port = "/dev/ttyACM0"
class TTY:
	def __init__(self, tty_name):
		self.ser = serial.Serial(tty_name)
		self.ser.baudrate = 115200
		self.ser.flushInput()
		self.ser.flushOutput()
		self.ser.timeout = 2
	def send(self, msg):
		#self.ser.write(bytes(msg, 'utf-8'))
		self.ser.write(bytes(msg))
	def read(self):
		return self.ser.read()

stm = TTY(microcontroler_port)

not_finished = True
parameters_space = 29
start_byte = b"\x24"
end_byte = b"\x25"
help_byte = b"\x40"
start_byte_encoded = b"\x41"
end_byte_encoded = b"\x42"
help_byte_encoded = b"\x43"
resolution_x = 2560
resolution_y = 1440

class Function:
	def __init__(self, name, byte, parameters):
		self.name = name
		self.byte = byte
		self.parameters = parameters
	def check_raw(self, msg):
		print("rawstart")
		byte_list = []
		list_of_matches = re.findall(r"\d+", msg)
		print(list_of_matches)
		for i in list_of_matches:
			try:
				parameter = int(i)
				byte_list.append(parameter.to_bytes(1, byteorder='big'))
			except:
				return False
		print(byte_list)
		return byte_list
	def check_function(self, msg):
		msg = msg.lower()
		name_regex = r"^" + self.name
		regex = name_regex
		param_regex = r"\D+" + r"\d+"
		match = re.search(regex, msg)
		if match is None:
			return False
		if self.name == "raw":
			return self.check_raw(msg)
		for i in self.parameters:
			regex += param_regex
		regex += r"\D|$"
		match = re.search(regex, msg)
		if match is None:
			return False
		match = re.search(name_regex, msg)
		end = match.end()
		parameter_list = []
		for i in self.parameters:
			tmp_msg = msg[end:]
			match = re.search(param_regex, tmp_msg)
			parameter = 0
			try:
				parameter = int(msg[end:match.end() + end])
			except:
				return False
			if parameter >= 2**(i*8):
				return False
			parameter_list.append(parameter)
			end += match.end()
		byte_list = [self.byte]
		for indeks, integer in enumerate(parameter_list):
			byte_list.append(integer.to_bytes(self.parameters[indeks], byteorder='big'))
		return byte_list
somelist = []
for i in range(1,30):
	somelist.append(1)

function_list = []
#turning on LED
function_list.append(Function("on", b"\x00", []))
#turning off LED
function_list.append(Function("off", b"\x01", []))
#printing date every second
function_list.append(Function("start", b"\x02", []))
#stop printing date every second
function_list.append(Function("stop", b"\x03", []))
#print date
function_list.append(Function("print", b"\x04", []))
#send byte to RTC
function_list.append(Function("send", b"\x05", [1, 1]))
#read byte from RTC 
function_list.append(Function("read", b"\x06", [1]))
#print date with clock burst
function_list.append(Function("clock burst", b"\x07", []))
#print ram content
function_list.append(Function("ram burst", b"\x08", []))
#send data to ram
function_list.append(Function("send ram", b"\x09", somelist))
#set time H,M,S
function_list.append(Function("set time", b"\x0A", [1, 1, 1]))
#set date W,D,M,Y
function_list.append(Function("set date", b"\x0B", [1, 1, 1, 1]))
#set date and time H,M,S,W,D,M,Y
function_list.append(Function("set full date", b"\x0C", [1, 1, 1, 1, 1, 1, 1]))
#enable write
function_list.append(Function("enable", b"\x0D", []))
#disable write
function_list.append(Function("disable", b"\x0E", []))
#read clock halt flag
function_list.append(Function("flag", b"\x0F", []))
#stop clock
function_list.append(Function("halt", b"\x10", []))
#turn on clock
function_list.append(Function("poweron", b"\x11", []))
#raw input to STM
function_list.append(Function("raw", None, None))

#send ram 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29

class Terminal:
	def __init__(self):
		self.window = tk.Tk()
		self.window.geometry(str(resolution_x) + "x" + str(resolution_y))
		self.window.title("STM32 Terminal")
		self.window.configure(background="black")
		self.output_name = tk.Label(self.window, text="Output from STM32:", fg="black", bg="white", anchor="w")
		self.output_name.pack(side="top", fill="x")
		self.output_list = []
		self.max_element_of_list = int(resolution_y / 24)
		self.entry = tk.Entry(self.window)
		self.entry.pack(side="bottom", fill="x", padx=20)
	def print(self, print_text):
		fg_color = "white"
		bg_color = "black"
		#fg_color = "black"
		#bg_color = "white"
		if print_text[:10] == "Function: ":
			fg_color = "gold"
			#fg_color = "dark goldenrod"
		if len(self.output_list) < self.max_element_of_list:
			self.output_list.append(tk.Label(self.window, text=print_text, fg=fg_color, bg=bg_color, anchor="w"))
			self.output_list[-1].pack(side="top", fill="x")
		else:
			self.output_list[0].destroy()
			del self.output_list[0]
			self.output_list.append(tk.Label(self.window, text=print_text, fg=fg_color, bg=bg_color, anchor="w"))
			self.output_list[-1].pack(side="top", fill="x")
	def send(self, event):
		global parameters_space
		message_to_send = self.entry.get()
		self.entry.delete(0, "end")
		string_list = []
		byte_list = False
		raw = False
		input_msg = "Function: "
		for i in function_list:
			byte_list = i.check_function(message_to_send)
			if byte_list != False:
				if i.name == "raw":
					raw = True
				input_msg += message_to_send
				break
		if byte_list == False:
			print("Wrong command")
			return
		if raw == False:
			#for i in range(0, parameters_space - len(byte_list)):
			#	byte_list.append(b"\x00")
			string_list.append(start_byte)
			for byte in byte_list:
				if byte == start_byte:
					string_list.append(help_byte)
					string_list.append(start_byte_encoded)
				elif byte == help_byte:
					string_list.append(help_byte)
					string_list.append(help_byte_encoded)
				elif byte == end_byte:
					string_list.append(help_byte)
					string_list.append(end_byte_encoded)
				else:
					string_list.append(byte)
			string_list.append(end_byte)
			byte_list = []
			for i in string_list:
				byte_list.append(int.from_bytes(i, "big"))
		else:
			string_list = byte_list
			byte_list = []
			for i in string_list:
				byte_list.append(int.from_bytes(i, "big"))
		byte_list = bytearray(byte_list)
		stm.send(byte_list)
		input_msg = input_msg + ", Input: " + str(byte_list)
		self.print(input_msg)
	def mainloop(self):
		self.entry.bind('<Return>', self.send)
		self.window.mainloop()

terminal = Terminal()

class Receiving_Thread(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
	def run(self):
		global not_finished
		byte_list = []
		start_was_sent = False
		help_byte_was_sent = False
		global start_byte
		global help_byte
		global start_byte_encoded
		global help_byte_encoded
		while not_finished:
			received_byte = stm.read()
			if start_was_sent:
				#End of frame
				if received_byte == end_byte and not help_byte_was_sent:
					start_was_sent = False
					bytes_sequence = b"".join(byte_list)
					decoded_message = bytes_sequence.decode("utf-8")
					print(decoded_message)
					terminal.print(decoded_message)
					byte_list = []
				#Help byte
				elif received_byte == help_byte and not help_byte_was_sent:
					help_byte_was_sent = True
				#Help byte part 2
				elif help_byte_was_sent:
					help_byte_was_sent = False
					if received_byte == start_byte_encoded:
						byte_list.append(start_byte)
					elif received_byte == help_byte_encoded:
						byte_list.append(help_byte)
					elif received_byte == end_byte_encoded:
						byte_list.append(end_byte)
					else:
						#error
						start_was_sent = False
				#Start byte received when start was already received earlier
				elif received_byte == start_byte:
					byte_list = []
				#Normal character
				else:
					byte_list.append(received_byte)
			#Start of frame
			elif received_byte == start_byte:
				start_was_sent = True

Receiving_Thread().start()
terminal.mainloop()
not_finished = False
print("koniec")

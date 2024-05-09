#Code is taken and based on the GIT Hub repo UART terminal in the repo
#https://github.com/linmh0130/GUI_UART_Demo/blob/master/GUI_UART_Demo.py

import tkinter as tk
from tkinter import ttk
import serial
import threading

class InformWindow:
    def __init__(self, inform_str):
        self.inform_str = inform_str
        self.window = None  # Placeholder for the window

    def show(self):
        self.window = tk.Toplevel()
        self.window.title("Information")
        label = tk.Label(self.window, text=self.inform_str)
        button_ok = tk.Button(self.window, text="OK", command=self.close_window)
        label.pack(side=tk.TOP)
        button_ok.pack(side=tk.BOTTOM)

    def close_window(self):
        self.window.destroy()

class MainGUI:
    def __init__(self):
        self.window = tk.Tk()
        self.window.title("UART terminal")
        self.uart_state = False  # Starting the UART as false
        self.ser = serial.Serial()  # Serial port
        self.parity = tk.StringVar(value="NONE")  # Baurdrate used has no parity
        self.stopbits = tk.StringVar(value="1")  # and 1 stopbit
        self.create_widgets()
        self.thread_active = True
        self.read_uart_thread = threading.Thread(target=self.read_uart) #threading for UART
        self.read_uart_thread.start()
        self.window.mainloop() #Starting the main loop

    def create_widgets(self):
        self.create_com_frame() #Creating each different frame 
        self.create_transmission_frame()
        self.create_reception_frame()
        #And then defining what is in each frame 
    def create_com_frame(self):
        frame_com_inf = tk.Frame(self.window)
        frame_com_inf.grid(row=1, column=1, sticky="ew") #Creating a grid and the placement for it
        #COM port is set but this can be changed here or in the text box
        tk.Label(frame_com_inf, text="COMx: ").grid(row=1, column=1, padx=5, pady=3)
        self.com = tk.StringVar(value="/dev/cu.usbmodem11203")
        tk.Entry(frame_com_inf, textvariable=self.com).grid(row=1, column=2, padx=5, pady=3)
        #Baudrate is fixed and should need adjustment 
        tk.Label(frame_com_inf, text="Baudrate: ").grid(row=1, column=3, padx=5, pady=3)
        self.baudrate = tk.IntVar(value=9600)
        tk.Entry(frame_com_inf, textvariable=self.baudrate).grid(row=1, column=4, padx=5, pady=3)
        #Buttons for each of the different test cases
        tk.Button(frame_com_inf, text="Test Servo", command=self.send_one).grid(row=4, column=1, padx=3, pady=2, sticky=tk.E)
        tk.Button(frame_com_inf, text="Test IR", command=self.send_two).grid(row=4, column=2, padx=3, pady=2, sticky=tk.E)
        tk.Button(frame_com_inf, text="Test IMU", command=self.send_three).grid(row=4, column=3, padx=3, pady=2, sticky=tk.E)
        tk.Button(frame_com_inf, text="Test Ultrasonic", command=self.send_four).grid(row=4, column=4, padx=3, pady=2, sticky=tk.E)
        #Toggling the UART once the start button has been pressed
        self.button_ss = tk.Button(frame_com_inf, text="Start", command=self.toggle_uart)
        self.button_ss.grid(row=3, column=4, padx=5, pady=3, sticky=tk.E)

    #Defining the tranmission (Tx) frame 
    def create_transmission_frame(self):
        frame_trans = tk.Frame(self.window)
        frame_trans.grid(row=2, column=1)
        tk.Label(frame_trans, text="To Transmit Data:").grid(row=1, column=1, padx=3, pady=2, sticky=tk.W)

        frame_trans_son = tk.Frame(frame_trans)
        frame_trans_son.grid(row=2, column=1)
        scrollbar_trans = tk.Scrollbar(frame_trans_son)
        scrollbar_trans.pack(side=tk.RIGHT, fill=tk.Y)

        self.input_text = tk.Text(frame_trans_son, wrap=tk.WORD, width=60, height=5, yscrollcommand=scrollbar_trans.set)
        self.input_text.pack()
        tk.Button(frame_trans, text="Send", command=self.send_data).grid(row=3, column=1, padx=5, pady=3, sticky=tk.E)
        #Defining the receive (Rx) frame, both have scrollbars in case there is a lot of data inputted out outputted
    def create_reception_frame(self):
        frame_recv = tk.Frame(self.window)
        frame_recv.grid(row=3, column=1)
        tk.Label(frame_recv, text="Received Data:").grid(row=1, column=1, padx=3, pady=2, sticky=tk.W)

        frame_recv_son = tk.Frame(frame_recv)
        frame_recv_son.grid(row=2, column=1)
        scrollbar_recv = tk.Scrollbar(frame_recv_son)
        scrollbar_recv.pack(side=tk.RIGHT, fill=tk.Y)
        
        tk.Button(frame_recv, text="Clear", command=self.clear_received_data).grid(row=3, column=1, padx=5, pady=3, sticky=tk.E)
        self.output_text = tk.Text(frame_recv_son, wrap=tk.WORD, width=60, height=20, yscrollcommand=scrollbar_recv.set)
        self.output_text.pack()
        
        #Function once the start button has been pressed to set the UART
    def toggle_uart(self):
        if self.uart_state:
            self.ser.close()
            self.button_ss["text"] = "Start" #m
            self.uart_state = False
            InformWindow("UART connection closed.")
        else:
            self.configure_serial()
            try:
                self.ser.open()
                self.button_ss["text"] = "Stop"
                self.uart_state = True
                InformWindow("UART connection established.")
            except serial.SerialException as e:
                InformWindow(f"Failed to open {self.ser.port}: {str(e)}")
                #This was used at the start but unused as only got fixed values for the parity and stopbits
    def configure_serial(self):
        self.ser.port = self.com.get()
        self.ser.baudrate = self.baudrate.get()
        self.ser.parity = {'NONE': serial.PARITY_NONE, 'ODD': serial.PARITY_ODD, 
                           'EVEN': serial.PARITY_EVEN, 'MARK': serial.PARITY_MARK,
                           'SPACE': serial.PARITY_SPACE}[self.parity.get()]
        self.ser.stopbits = {'1': serial.STOPBITS_ONE, '1.5': serial.STOPBITS_ONE_POINT_FIVE,
                             '2': serial.STOPBITS_TWO}[self.stopbits.get()]

    #Functions to send numbers once a button has been pressed
    def send_one(self):
        number = 1
        data_to_send = str(number).encode('ascii')
        self.ser.write(data_to_send)
        
    def send_two(self):
        number = 2
        data_to_send = str(number).encode('ascii')
        self.ser.write(data_to_send)
        
    def send_three(self):
        number = 3
        data_to_send = str(number).encode('ascii')
        self.ser.write(data_to_send)
        
    def send_four(self):
        number = 4
        data_to_send = str(number).encode('ascii')
        self.ser.write(data_to_send)
        
    def send_data(self):
        if self.uart_state:
            data_to_send = self.input_text.get(1.0, tk.END).rstrip()
            self.ser.write(data_to_send.encode('ascii'))
            #Reading in the UART and using ascii to encode/decode
    def read_uart(self):
        while self.thread_active:
            if self.uart_state and self.ser.is_open:
                try:
                    data_received = self.ser.read(self.ser.in_waiting or 1).decode('ascii') #
                    if data_received:
                        self.output_text.after(0, self.update_received_data, data_received)
                except serial.SerialException:
                    self.output_text.after(0, self.handle_serial_error)

    def update_received_data(self, data):
        self.output_text.insert(tk.END, data) #Reading new data and displaying it

    def handle_serial_error(self):
        InformWindow("Error in receiving data.")
        self.toggle_uart()

    def clear_received_data(self):
        self.output_text.delete(1.0, tk.END) #Deleting the text inside the text box
        #to clear for space


if __name__ == "__main__":
    gui = MainGUI()
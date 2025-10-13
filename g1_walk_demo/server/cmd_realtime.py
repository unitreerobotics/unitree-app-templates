import subprocess, threading, queue
from config import *
import time

realtime_proc = None
output_queue = queue.Queue()

def print_output_realtime():
    def printer():
        while True:
            if realtime_proc and realtime_proc.poll() is None:
                while not output_queue.empty():
                    line = output_queue.get()
                    print(line, end='')
            else:
                break
    threading.Thread(target=printer, daemon=True).start()
    
def start_realtime(cmd_args):
    global realtime_proc, output_queue
    if realtime_proc and realtime_proc.poll() is None:
        return  

    output_queue = queue.Queue()
    realtime_proc = subprocess.Popen(
        ["stdbuf", "-oL", "-eL", EXECUTABLE_BIN] + cmd_args,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    threading.Thread(target=lambda p,realtime_queue: [realtime_queue.put(l) for l in iter(p.stdout.readline, '')],
                     args=(realtime_proc, output_queue),
                     daemon=True).start()


def send_input_realtime(cmd_args):
    global realtime_proc
    if REALTIME:
        if not realtime_proc or realtime_proc.poll() is not None:
            start_realtime(cmd_args)

        input_string = cmd_args
        for char in input_string:
            realtime_proc.stdin.write(char)
            realtime_proc.stdin.flush() 
            time.sleep(0.01) 
    else: 
        if not realtime_proc or realtime_proc.poll() is not None:
            start_realtime(cmd_args)
        realtime_proc.stdin.write("\n".join(cmd_args) + "\n")
        realtime_proc.stdin.flush()

def read_output():
    lines = []
    while not output_queue.empty():
        lines.append(output_queue.get())
    return lines

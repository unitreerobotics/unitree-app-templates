import subprocess, threading, queue
from config import *
# Global realtime process and output queue
realtime_proc = None
output_queue = queue.Queue()

def print_output_realtime():
    """Continuously print subprocess output in a background thread."""
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
    """Start the subprocess in realtime mode."""
    global realtime_proc, output_queue
    if realtime_proc and realtime_proc.poll() is None:
        return  # Already running

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


# def send_input_realtime(cmd_args):
#     """Send input to the realtime process in a single write."""
#     global realtime_proc
#     if not realtime_proc or realtime_proc.poll() is not None:
#         start_realtime(cmd_args)
#     realtime_proc.stdin.write("\n".join(cmd_args) + "\n")
#     realtime_proc.stdin.flush()
import time
def send_input_realtime(cmd_args):
    """Send input to the realtime process one character at a time."""
    global realtime_proc
    if not realtime_proc or realtime_proc.poll() is not None:
        start_realtime(cmd_args)

    input_string = cmd_args
    for char in input_string:
        realtime_proc.stdin.write(char)
        realtime_proc.stdin.flush()
        # Add a short delay to simulate human typing and avoid overwhelming the process.
        time.sleep(0.01) 
        
def read_output():
    lines = []
    while not output_queue.empty():
        lines.append(output_queue.get())
    return lines

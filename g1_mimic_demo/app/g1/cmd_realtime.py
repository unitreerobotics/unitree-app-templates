import subprocess, threading, queue
from config import *
# 全局实时进程与输出队列
realtime_proc = None
output_queue = queue.Queue()

def print_output_realtime():
    """后台线程，持续打印子进程输出"""
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
    """启动实时模式子进程"""
    global realtime_proc, output_queue
    if realtime_proc and realtime_proc.poll() is None:
        return  # 已经在运行

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
#     """向实时进程发送一次性输入"""
#     global realtime_proc
#     if not realtime_proc or realtime_proc.poll() is not None:
#         start_realtime(cmd_args)
#     realtime_proc.stdin.write("\n".join(cmd_args) + "\n")
#     realtime_proc.stdin.flush()
import time
def send_input_realtime(cmd_args):
    """向实时进程发送输入 (逐个字符)"""
    global realtime_proc
    if not realtime_proc or realtime_proc.poll() is not None:
        start_realtime(cmd_args)

    input_string = cmd_args
    for char in input_string:
        realtime_proc.stdin.write(char)
        realtime_proc.stdin.flush()
        # 增加一个小的延迟，以模拟人类的打字速度，防止进程被数据淹没
        time.sleep(0.01) 
        
def read_output():
    lines = []
    while not output_queue.empty():
        lines.append(output_queue.get())
    return lines

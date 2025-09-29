EXECUTABLE_BIN = "/workspace/app/cmd"
REALTIME=False

OPTIONS = [
    {"id": 1, "name": "前进一步", "cmd": "move", "args": ["0.6", "0", "0"]},
    {"id": 2, "name": "后退一步", "cmd": "move", "args": ["-0.6", "0", "0"]},
    {"id": 3, "name": "向左平移", "cmd": "move", "args": ["0", "0.6", "0"]},
    {"id": 4, "name": "向右平移", "cmd": "move", "args": ["0", "-0.6", "0"]},
    {"id": 5, "name": "原地左转", "cmd": "move", "args": ["0", "0", "0.6"]},
    {"id": 6, "name": "原地右转", "cmd": "move", "args": ["0", "0", "-0.6"]},
]
 
from fastapi import FastAPI, Request
from pydantic import BaseModel
import subprocess
import uvicorn
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
from config import *
from cmd_realtime import *

app = FastAPI(title="sdk2-example-cmd-server")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

def run_cmd(cmd, args):
    command = [EXECUTABLE_BIN, cmd] + args
    try:
        result = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=False
        )

        return {
            "command": " ".join(command),
            "stdout": result.stdout.strip(),
            "stderr": result.stderr.strip(),
            "returncode": result.returncode
        }
    except Exception as e:

        return {"error": str(e)}

# ========== 1. 原始 execute 接口 ==========
@app.post("/api/command")
async def execute(request: Request):
    data = await request.json()
    return run_cmd(data["cmd"], data.get("args", []))

@app.get("/api/options")
async def get_options():
    return {"options": OPTIONS}

# ========== 3. 按选项执行 ==========
@app.post("/api/execute")
async def api_execute(request: Request):
    data = await request.json()
    action_id = int(data["id"])

    option = next((o for o in OPTIONS if o["id"] == action_id), None)
    if not option:
        return {"error": f"未知动作 id={action_id}"}

    if REALTIME:
        # 内部实时模式处理
        send_input_realtime([option["cmd"]] + option["args"])
        output = read_output()
        return {"status": "realtime_sent", "stdout": output}
    else:
        # 一次性执行
        return run_cmd(option["cmd"], option["args"])

@app.post('/api/action/start')
async def api_execute_start(request: Request):
    action_id = int(1)

    option = next((o for o in OPTIONS if o["id"] == action_id), None)
    if not option:
        return {"error": f"未知动作 id={action_id}"}

    if REALTIME:
        send_input_realtime([option["cmd"]] + option["args"])
        output = read_output()
        return {"status": "realtime_sent", "stdout": output}
    else:
        return run_cmd(option["cmd"], option["args"])

@app.post('/api/action/stop')
async def api_execute_stop(request: Request):
    action_id = int(2)

    option = next((o for o in OPTIONS if o["id"] == action_id), None)
    if not option:
        return {"error": f"未知动作 id={action_id}"}

    if REALTIME:
        send_input_realtime([option["cmd"]] + option["args"])
        output = read_output()
        return {"status": "realtime_sent", "stdout": output}
    else:
        return run_cmd(option["cmd"], option["args"])
# 静态页面
# app.mount("/ui", StaticFiles(directory="ui"), name="ui")

if __name__ == '__main__':
    if REALTIME:
        start_realtime([])
        output = read_output()

    uvicorn.run('cmd_server:app', host='0.0.0.0', port=80, log_level='info')

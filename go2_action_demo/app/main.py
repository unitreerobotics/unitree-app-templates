import time
import sys
from unitree_sdk2py.core.channel import ChannelSubscriber, ChannelFactoryInitialize
from unitree_sdk2py.idl.default import unitree_go_msg_dds__SportModeState_
from unitree_sdk2py.idl.unitree_go.msg.dds_ import SportModeState_
from unitree_sdk2py.go2.sport.sport_client import (
    SportClient,
    PathPoint,
    SPORT_PATH_POINT_SIZE,
)
import math
from dataclasses import dataclass
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import asyncio
import uvicorn

app = FastAPI(title="sdk2-example-server")

from fastapi.middleware.cors import CORSMiddleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@dataclass
class TestOption:
    name: str
    id: int

option_list = [
    TestOption(name="damp", id=0),         
    TestOption(name="stand_up", id=1),     
    TestOption(name="stand_down", id=2),   
    TestOption(name="move forward", id=3),         
    TestOption(name="move lateral", id=4),    
    TestOption(name="move rotate", id=5),  
    TestOption(name="stop_move", id=6),  
    TestOption(name="hand stand", id=7),
    TestOption(name="balanced stand", id=9),     
    TestOption(name="recovery", id=10),       
    TestOption(name="left flip", id=11),      
    TestOption(name="back flip", id=12),
    TestOption(name="free walk", id=13),  
    TestOption(name="free bound", id=14), 
    TestOption(name="free avoid", id=15),  
    TestOption(name="walk upright", id=17),
    TestOption(name="cross step", id=18),
    TestOption(name="free jump", id=19)       
]


sport_client: SportClient | None = None

class UserInterface:
    def __init__(self):
        self.test_option_ = None

    def convert_to_int(self, input_str):
        try:
            return int(input_str)
        except ValueError:
            return None

    def terminal_handle(self):
        input_str = input("Enter id or name: \n")

        if input_str == "list":
            self.test_option_.name = None
            self.test_option_.id = None
            for option in option_list:
                print(f"{option.name}, id: {option.id}")
            return

        for option in option_list:
            if input_str == option.name or self.convert_to_int(input_str) == option.id:
                self.test_option_.name = option.name
                self.test_option_.id = option.id
                print(f"Test: {self.test_option_.name}, test_id: {self.test_option_.id}")
                return

        print("No matching test option found.")

@app.on_event("startup")
async def startup_sdk():
    def _init():
        if len(sys.argv) > 1:
            ChannelFactoryInitialize(0, sys.argv[1])
        else:
            ChannelFactoryInitialize(0)
        global sport_client
        sport_client = SportClient()
        sport_client.SetTimeout(10.0)
        sport_client.Init()

    await asyncio.to_thread(_init)


async def _execute_option(opt_id: int):
    def sync_exec():
        if sport_client is None:
            raise RuntimeError('sport_client not initialized')
        ret = None
        if opt_id == 0:
            ret = sport_client.Damp()
        elif opt_id == 1:
            ret = sport_client.StandUp()
        elif opt_id == 2:
            ret = sport_client.StandDown()
        elif opt_id == 3:
            ret = sport_client.Move(0.3,0,0)
            print("ret: ",ret)
        elif opt_id == 4:
            ret = sport_client.Move(0,0.3,0)
        elif opt_id == 5:
            ret = sport_client.Move(0,0,0.5)
        elif opt_id == 6:
            ret = sport_client.StopMove()
        elif opt_id == 7:
            ret = sport_client.HandStand(True)
            time.sleep(4)
            ret = sport_client.HandStand(False)
        elif opt_id == 9:
            ret = sport_client.BalanceStand()
        elif opt_id == 10:
            ret = sport_client.RecoveryStand()
        elif opt_id == 11:
            ret = sport_client.LeftFlip()
            print("ret: ",ret)
        elif opt_id == 12:
            ret = sport_client.BackFlip()
            print("ret: ",ret)
        elif opt_id == 13:
            ret = sport_client.FreeWalk()
            print("ret: ",ret)
        elif opt_id == 14:
            ret = sport_client.FreeBound(True)
            print("ret: ",ret)
            time.sleep(2)
            ret = sport_client.FreeBound(False)
            print("ret: ",ret)
        elif opt_id == 15:
            ret = sport_client.FreeAvoid(True)
            print("ret: ",ret)
            time.sleep(2)
            ret = sport_client.FreeAvoid(False)
            print("ret: ",ret)
        elif opt_id == 17:
            ret = sport_client.WalkUpright(True)
            print("ret: ",ret)
            time.sleep(4)
            ret = sport_client.WalkUpright(False)
            print("ret: ",ret)
        elif opt_id == 18:
            ret = sport_client.CrossStep(True)
            print("ret: ",ret)
            time.sleep(4)
            ret = sport_client.CrossStep(False)
            print("ret: ",ret)
        elif opt_id == 19:
            ret = sport_client.FreeJump(True)
            print("ret: ",ret)
            time.sleep(4)
            ret = sport_client.FreeJump(False)
            print("ret: ",ret)
        # small delay to mimic original loop pacing
        time.sleep(1)
        return ret

    return await asyncio.to_thread(sync_exec)

@app.get('/api/options')
def api_options():
    return {"options": [{"name": o.name, "id": o.id} for o in option_list]}

class ExecRequest(BaseModel):
    id: int

@app.post('/api/execute')
async def api_execute(req: ExecRequest):
    opt_id = req.id
    if opt_id is None:
        raise HTTPException(status_code=400, detail='id required')
    try:
        ret = await _execute_option(opt_id)
        return {"ok": True, "ret": ret}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == '__main__':
    uvicorn.run('main:app', host='0.0.0.0', port=80, log_level='info')
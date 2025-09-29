FROM test-harbor.unitree.com/image/cpp_sdk2:latest

RUN apt update && apt install -y python3 pip && pip install fastapi uvicorn -i https://pypi.tuna.tsinghua.edu.cn/simple

CMD [ "bash" ]
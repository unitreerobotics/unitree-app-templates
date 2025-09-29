# Mobile Viewer

这是一个简单的 HTTP 服务器脚本，协助您快速调试移动端 UI 效果。它会递归搜索指定目录下的所有子目录，查找包含 `index.html` 文件的目录，并将这些目录作为可访问的应用根目录进行展示。

## 启动

执行以下命令启动服务器：
```bash
python3 server.py 
```

服务器默认监听 `8888` 端口，您可以通过浏览器访问 `http://localhost:8888/mobile_viewer/` 来查看可用的应用列表。
您也可以自行修改，方便您的调试工作。

## 调试
打开您需要调试的应用后端服务，并修改对应 UI 文件夹 `config.json` 文件中的 `appUrl` 字段为您的服务地址，即可进行联调。

比如当您需要调试 `g1_walk_demo` 示例时，当您启动了 `g1_walk_demo` 的后端服务：
```bash
cd g1_walk_demo
python3 server/cmd_server.py
```

服务将会在 80 端口启动，您需要修改 `g1_walk_demo/ui/config.json` 文件中的 `appUrl` 字段为：
```json
{
  "appUrl": "http://localhost:80"
}
```

保存后，刷新浏览器 `http://localhost:8888/mobile_viewer/` 页面，选择对应 UI，即可看到 UI 与后端服务联调的效果。
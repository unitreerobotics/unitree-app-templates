# CLI 工具

## 上传应用

压缩文件夹并上传应用包。

#### 参数
- `--folder FOLDER`      要压缩的文件夹路径
-  `--yaml YAML`          yaml配置文件路径
-  `--username USERNAME`  登录账号
-  `--password PASSWORD`  登录密码

#### 命令

```bash
python3 cli.py --folder FOLDER --yaml YAML --username USERNAME --password PASSWORD
```

示例：

```bash
python3 cli.py --folder ./python-control-demo/production --yaml ./python-control-demo/metadata.yaml --username test --password test
```
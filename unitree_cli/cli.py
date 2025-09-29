import os
import tarfile
import yaml
import requests
import argparse

def tar_gzip_folder(folder_path):
    base_name = os.path.basename(folder_path.rstrip('/\\'))
    tar_path = base_name + '.tar.gz'
    
    with tarfile.open(tar_path, 'w:gz') as tarf:
        tarf.add(folder_path, arcname=base_name)
        
    print(f"已压缩: {tar_path}")
    return tar_path

def parse_yaml(yaml_path):
    with open(yaml_path, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f)
    print(f"已解析yaml: {yaml_path}")
    return data

def get_token(username, password):
    resp = requests.post('https://test-harbor.unitree.com:8443/api/login', json={'username': username, 'password': password})
    if resp.status_code == 200:
        token = resp.json().get('access_token')
        print(f"登录成功，获取token: {token}")
        return token
    else:
        print(f"登录失败: {resp.status_code} {resp.text}")
        exit(1)

def upload_binary(tar_path):
    filename = os.path.basename(tar_path)
    
    resp = requests.post('https://test-harbor.unitree.com:8443/upload-url', json={'filename': filename})
    if resp.status_code != 200:
        print(f"获取上传URL失败: {resp.status_code} {resp.text}")
        exit(1)
        
    url = resp.json()['url']
    
    with open(tar_path, 'rb') as f:
        put_resp = requests.put(url, data=f)
        
    print(f"二进制包上传结果: {put_resp.status_code}")
    
    file_to_remove = tar_path 
    if os.path.exists(file_to_remove):
        os.remove(file_to_remove)
        print(f"已删除压缩包: {file_to_remove}")
    
    return filename

def upload_form(yaml_data, filename, token):
    form = {}
    fields = ['name', 'description', 'developer_id', 'version', 'supportedRobotModels', 'price', 'priceType', 'permissions', 'selectedImage', 'buildEnv', 'portMapping', 'icon', 'category', 'rating']
    for k in fields:
        if k in yaml_data:
            form[k] = yaml_data[k]
            
    form['package_file_name'] = filename
    form['package_file_url'] = filename
    
    headers = {
        'Authorization': f'Bearer {token}',
        'Content-Type': 'application/json'
    }
    
    resp = requests.post('https://test-harbor.unitree.com:8443/api/packages', json=form, headers=headers)
    print(f"表单上传结果: {resp.status_code}", resp.text)
    return resp

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='压缩文件夹并上传应用包 (tar.gz)')
    parser.add_argument('--folder', required=True, help='要压缩的文件夹路径')
    parser.add_argument('--yaml', required=True, help='yaml配置文件路径')
    parser.add_argument('--username', required=True, help='登录账号')
    parser.add_argument('--password', required=True, help='登录密码')
    args = parser.parse_args()
    
    tar_path = tar_gzip_folder(args.folder)
    
    yaml_data = parse_yaml(args.yaml)
    token = get_token(args.username, args.password)
    
    filename = upload_binary(tar_path)
    
    print(f"上传的文件名: {filename}")
    
    upload_form(yaml_data, filename, token)
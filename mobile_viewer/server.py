import http.server
import socketserver
import os
import json
from urllib.parse import urlparse, unquote

PORT = 8888
EXCLUDE_DIR_NAMES = {"mobile_viewer", "node_modules", ".git"}

class ReusableTCPServer(socketserver.TCPServer):
    allow_reuse_address = True

class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    
    def get_directories_with_index(self):
        search_root = self.directory 
        
        found_dirs = []
        
        def traverse_dirs(current_path, relative_path=""):
            try:
                for item in os.listdir(current_path):
                    item_path = os.path.join(current_path, item)
                    
                    if item in EXCLUDE_DIR_NAMES:
                        continue

                    if os.path.isdir(item_path):
                        new_relative_path = os.path.join(relative_path, item)
                        
                        if os.path.exists(os.path.join(item_path, 'index.html')):
                            posix_path = new_relative_path.replace(os.path.sep, '/')
                            found_dirs.append(posix_path)
                            
                        traverse_dirs(item_path, new_relative_path)
                            
            except Exception as e:
                print(f"Error accessing directory {current_path}: {e}")
                pass

        traverse_dirs(search_root)
        
        return found_dirs

    def do_GET(self):
        parsed_path = urlparse(self.path)
        path = unquote(parsed_path.path)

        if path == '/api/list_apps':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')

            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            app_list = self.get_directories_with_index()
            response = {"apps": app_list}
            self.wfile.write(json.dumps(response).encode('utf-8'))
            return
        
        else:
            super().do_GET()


if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    NEW_WEB_ROOT = os.path.abspath(os.path.join(script_dir, os.pardir))
    
    Handler = lambda *args, **kwargs: CustomHTTPRequestHandler(*args, directory=NEW_WEB_ROOT, **kwargs)

    with ReusableTCPServer(("", PORT), Handler) as httpd:
        print(f"Serving HTTP on port {PORT}...")
        print(f"Web Root (已设置为上一级目录): {NEW_WEB_ROOT}")
        print("-----------------------------------------------------")

        print(f"由于 Web Root 已更改，请访问主页的新路径: http://localhost:{PORT}/mobile_viewer/index.html")
        httpd.serve_forever()

import http.server
import socketserver

class MyHttpRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.send_header("Content-Length", 11)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write('Hello world'.encode("utf-8"))

# Create an object of the above class
handler_object = MyHttpRequestHandler

PORT = 80
my_server = socketserver.TCPServer(("", PORT), handler_object)

# Star the server
my_server.serve_forever()

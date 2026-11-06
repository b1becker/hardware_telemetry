"""Local HTTP fixtures exercise the real executable; no Raspberry Pi required."""
import http.server
import os
import socket
import subprocess
import sys
import threading
import time

endpoint = "/redfish/v1/Systems/1"
requests = []
status = 200
body = b'{"HostName":"fixture-only"}'
delay = 0


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        requests.append(self.path)
        time.sleep(delay)
        self.send_response(status)
        if status == 302:
            self.send_header("Location", "/redirect-must-not-be-followed")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        try:
            self.wfile.write(body)
        except (BrokenPipeError, ConnectionResetError):
            pass

    def log_message(self, *args):
        pass


server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
threading.Thread(target=server.serve_forever, daemon=True).start()
base = f"http://127.0.0.1:{server.server_port}"
env = dict(os.environ, NO_PROXY="*", no_proxy="*")
other_requests = []


class OtherHandler(Handler):
    def do_GET(self):
        other_requests.append(self.path)
        response = b'{"HostName":"second-fixture"}'
        self.send_response(200)
        self.send_header("Content-Length", str(len(response)))
        self.end_headers()
        self.wfile.write(response)


other_server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), OtherHandler)
threading.Thread(target=other_server.serve_forever, daemon=True).start()
other_base = f"http://127.0.0.1:{other_server.server_port}"


def run(inputs, target=base):
    result = subprocess.run([sys.argv[1], target], input=inputs, text=True,
                            capture_output=True, timeout=15, env=env)
    assert result.returncode == 0, result.stderr
    return result.stdout + result.stderr


try:
    output = run("\n/random/path\nhttp://example.com\n" + endpoint + "\n" + endpoint + "\nexit\n")
    assert output.count("Unsupported endpoint") == 3, output
    assert output.count("HTTP Status: 200") == 2, output
    assert body.decode() in output, output
    assert requests == [endpoint, endpoint], requests
    # Switch to a distinct server, reject malformed commands, then switch back.
    before = len(requests)
    invalid_commands = ["connect", "connect https://pi", "connect http://pi/path",
                        "connect http://user:pass@pi", "connect http://pi extra"]
    inputs = (endpoint + "\nconnect " + other_base + "\n" + endpoint + "\n"
              + "\n".join(invalid_commands) + "\n" + endpoint + "\n"
              + "connect " + base + "\n" + endpoint + "\nexit\n")
    output = run(inputs)
    assert output.count("HTTP Status: 200") == 4, output
    assert output.count("Target unchanged: " + other_base) == len(invalid_commands), output
    assert "second-fixture" in output, output
    assert len(requests) == before + 2, requests
    assert other_requests == [endpoint, endpoint], other_requests
    before = len(requests)
    # Selecting a target alone must not perform a network request.
    run("connect " + other_base + "\n")
    assert len(requests) == before and len(other_requests) == 2
    for code in (404, 500, 302):
        status = code
        before = len(requests)
        output = run(endpoint + "\nexit\n")
        assert f"HTTP Status: {code}" in output, output
        assert "Transport error" not in output, output
        assert ("HTTP error" in output) == (code >= 400), output
        assert len(requests) == before + 1, requests
    status, body = 200, b""
    assert "(empty response body)" in run(endpoint + "\nexit\n")
    before = len(requests)
    run("exit\n" + endpoint + "\n")
    run("")  # EOF before any input.
    run(endpoint + "\n")  # EOF after a completed request.
    assert len(requests) == before + 1, requests
    # Reserve a local port without listening: deterministic connection refusal.
    with socket.socket() as unused:
        unused.bind(("127.0.0.1", 0))
        output = run(endpoint + "\nexit\n", f"http://127.0.0.1:{unused.getsockname()[1]}")
        assert "Transport error" in output, output
        assert "HTTP Status:" not in output, output
        output = run(f"connect http://127.0.0.1:{unused.getsockname()[1]}\n"
                     + endpoint + "\nconnect " + base + "\n" + endpoint + "\nexit\n")
        assert "Transport error" in output and "HTTP Status: 200" in output, output
    delay = 11
    output = run(endpoint + "\nexit\n")
    assert "Transport error" in output and "timed out" in output.lower(), output
finally:
    other_server.shutdown()
    other_server.server_close()
    server.shutdown()
    server.server_close()

print("HTTP and interactive tests passed")

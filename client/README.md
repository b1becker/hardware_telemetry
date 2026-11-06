# HardwareTelemetry Client

Interactive C++17/libcurl GET client. Builds independently of the daemon and
does not read local hardware telemetry.

On Debian/Ubuntu install dependencies:

```bash
sudo apt install build-essential cmake libcurl4-openssl-dev
# Optional, for automated HTTP integration tests:
sudo apt install python3
```

Build and test from this directory:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/telemetry-client http://192.168.1.50:18080
```

Use the port configured by your daemon (the repository currently uses 18080).
The target must be `http://HOST[:PORT]`, using an ASCII DNS-style hostname or
IPv4 address, without credentials, trailing slash, path, query, or fragment.
Ports must be 1–65535. HTTPS and IPv6 literals are outside this initial slice.

Enter `/redfish/v1/Systems/1`, then another endpoint or `exit`. EOF also exits.
To switch devices without restarting, enter:

```text
connect http://192.168.1.60:18080
/redfish/v1/Systems/1
```

`connect` requires exactly one valid base URL, using the same rules as startup.
Spaces or tabs may separate the command and URL. Invalid commands preserve the
current target. A successful command prints the selected target but sends no HTTP
request; the next endpoint request checks connectivity. You can switch again
after connection failures. Selection does not confirm the daemon is reachable.

Only that exact endpoint is allowed; whitespace, queries, fragments, and other
paths are rejected. Extend the array in `src/EndpointValidator.cpp` when adding
supported resources. The daemon remains authoritative for resource availability
and access control.

Responses show the HTTP status and original body without JSON reformatting or
replacement values. Empty bodies are explicitly identified. HTTP 4xx/5xx remain
HTTP responses; connection failures and timeouts are reported as transport errors.
Redirects are displayed without following them. Each request has a five-second
connection timeout and ten-second overall timeout. Errors return to the prompt.
libcurl's standard proxy environment configuration applies.

## Files

- `src/main.cpp`: arguments, libcurl lifetime, interactive loop.
- `include/EndpointValidator.hpp`, `src/EndpointValidator.cpp`: endpoint allowlist
  and restricted base URL validation.
- `include/TelemetryClient.hpp`, `src/TelemetryClient.cpp`: GET transfer, body
  callback, timeouts, status and error reporting.
- `tests/ValidationTests.cpp`: automated positive and negative validation cases.
- `tests/integration.py`: optional Python standard-library HTTP fixture testing
  200, 404, 500, redirects, empty bodies, connection refusal, overall timeout,
  repeated requests, switching between two servers, invalid switch recovery,
  switching after transport errors, exit, and EOF. Its 404 is returned at
  the allowed path to exercise status handling without bypassing validation.

## Raspberry Pi integration checks

1. Start the existing daemon on the Pi and run this client on your laptop using
   the Pi's reachable address and configured port.
2. Enter the supported endpoint twice. Verify two 200 responses and actual Pi
   hostname/memory values. Optional identity properties may be absent.
3. Enter an empty line, `/random/path`, an external URL, and a path with `?x=1`.
   Each should be rejected locally; then enter the supported endpoint again.
4. Stop the daemon and request again. Verify a transport error and a new prompt.
5. Enter `exit`, or press Ctrl-D on an empty input line. Verify clean termination.

The automated delayed-server test checks the overall timeout. To test connection
timeout specifically, use a controlled network/firewall setup that drops connection
attempts to your test host; verify a transport timeout and continued prompting.
Do not depend on a random public address behaving as a silent timeout.

No authentication, TLS, discovery, polling, or write operations are implemented.

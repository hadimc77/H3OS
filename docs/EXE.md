# Running .exe on H3OS

H3OS can load and run **native H3OS PE32+** executables (`.exe`).

## What works now

```text
run /bin/hello.exe
hello.exe
run hello.exe          # also searches /bin
```

Sample output:

```text
Hello from hello.exe on H3OS!
PE32+ loader works.
run: exit code 0
```

## ABI (for developers)

Entry point receives `h3os_pe_api_t*` in `RDI` (System V):

| Offset | Field | Purpose |
|--------|--------|---------|
| 0 | magic | `H3OS_PE_MAGIC` |
| 8 | write | print string |
| 16 | writeln | print line |
| 24 | exec | run another exe |
| 32 | uptime_ms | milliseconds |
| 40 | exit | set exit code |

Build a sample with:

```bash
python tools/mkpe.py
```

## What is NOT Windows Win32 yet

Classic Windows programs that call `kernel32.dll` / GUI / WinAPI will not run until a Windows compatibility subsystem (Wine-like) is added. Those EXEs will be rejected with a clear probe error unless they are H3OS-native PE images.

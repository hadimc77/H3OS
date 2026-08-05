# H3OS package manager CLI scaffold (host-side tooling)
# Runtime in-OS command is handled by the Terminal (h3pkg).

import argparse
import sys

VERSION = "0.1.0"

def main():
    p = argparse.ArgumentParser(prog="h3pkg", description="H3OS Package Manager")
    p.add_argument("command", choices=[
        "install", "remove", "update", "upgrade", "search", "repair", "version"
    ])
    p.add_argument("package", nargs="?", default="")
    args = p.parse_args()

    if args.command == "version":
        print(f"h3pkg {VERSION} — H3OS Package Manager")
        return 0

    print(f"[h3pkg] {args.command} {args.package}".strip())
    print("[h3pkg] Repository not configured in host tooling yet.")
    print("[h3pkg] Boot H3OS and use the in-system terminal for early packages.")
    return 0

if __name__ == "__main__":
    sys.exit(main())

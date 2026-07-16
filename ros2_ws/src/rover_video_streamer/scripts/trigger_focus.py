#!/usr/bin/env python3

import sys
import socket
import argparse

def main():
    parser = argparse.ArgumentParser(description="Trigger remote autofocus on the Jetson Rover")
    parser.add_argument('ip', type=str, help='IP address of the Jetson (e.g. 192.168.1.88)')
    parser.add_argument('--port', type=int, default=5005, help='UDP trigger port (default: 5005)')
    args = parser.parse_args()
    
    # Create a raw UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # Send the "focus" trigger payload to the Jetson listener
        sock.sendto(b"focus", (args.ip, args.port))
        print(f"Successfully sent autofocus trigger to Jetson at {args.ip}:{args.port}")
    except Exception as e:
        print(f"ERROR: Failed to send trigger packet: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()

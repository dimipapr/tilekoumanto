#operator/project.py

import os
import argparse
from lib import certs

# --- Dynamic Path Resolution ---
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(CURRENT_DIR)
OUTPUT_DIR = os.path.join(REPO_ROOT, "certs")

# --- Configuration ---
DOMAIN = "mqtt-dev.tilekoumanto.gr"
DAYS_CA = 3650
DAYS_SERVER = 1095
DAYS_DEVICE = 365

def main():
    parser = argparse.ArgumentParser(description="Tilekoumanto Operator CLI")
    # Require a command to be passed
    subparsers = parser.add_subparsers(dest="command", help="Available commands", required=True)

    # Command: certs
    certs_parser = subparsers.add_parser("certs", help="Generate CA, server, and device certificates")
    
    # Mandatory positional argument for target device count
    certs_parser.add_argument(
        "target_count",
        type=int,
        help="The total number of device certificates required"
    )

    args = parser.parse_args()

    if args.command == "certs":
        print(f"Repository Root resolved to: {REPO_ROOT}")
        print(f"Output Directory set to: {OUTPUT_DIR}\n")
        
        certs.ensure_dir(OUTPUT_DIR)
        
        certs.generate_ca(OUTPUT_DIR, DAYS_CA)
        certs.generate_server_cert(OUTPUT_DIR, DOMAIN, DAYS_SERVER)
        
        # Pass the target count to the generator
        certs.generate_device_certs(OUTPUT_DIR, args.target_count, DAYS_DEVICE)
        
        print(f"✅ Certificate generation complete. Check the '{OUTPUT_DIR}' directory.")

if __name__ == "__main__":
    main()
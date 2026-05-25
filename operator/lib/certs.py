import os
import subprocess
import json
import uuid

def run_cmd(command):
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error executing command: {' '.join(command)}")
        print(result.stderr)
        exit(1)

def ensure_dir(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)

def generate_ca(output_dir, days):
    print("--- 1. Generating Root CA ---")
    ca_key = os.path.join(output_dir, "ca.key")
    ca_crt = os.path.join(output_dir, "ca.crt")
    
    if os.path.exists(ca_key) and os.path.exists(ca_crt):
        print("CA already exists. Skipping.\n")
        return

    run_cmd(["openssl", "genrsa", "-out", ca_key, "4096"])
    
    subj = "/C=GR/O=Tilekoumanto/CN=Tilekoumanto Root CA"
    run_cmd([
        "openssl", "req", "-new", "-x509", "-days", str(days),
        "-key", ca_key, "-out", ca_crt, "-subj", subj
    ])
    print("CA created successfully.\n")

def generate_server_cert(output_dir, domain, days):
    print("--- 2. Generating Mosquitto Server Certificate ---")
    server_key = os.path.join(output_dir, "server.key")
    server_csr = os.path.join(output_dir, "server.csr")
    server_crt = os.path.join(output_dir, "server.crt")
    san_cnf = os.path.join(output_dir, "san.cnf")
    
    if os.path.exists(server_crt):
        print("Server certificate already exists. Skipping.\n")
        return

    run_cmd(["openssl", "genrsa", "-out", server_key, "2048"])
    
    subj = f"/C=GR/O=Tilekoumanto/CN={domain}"
    run_cmd(["openssl", "req", "-new", "-key", server_key, "-out", server_csr, "-subj", subj])
    
    with open(san_cnf, "w") as f:
        f.write(f"subjectAltName = DNS:{domain},IP:127.0.0.1\n")
    
    run_cmd([
        "openssl", "x509", "-req", "-in", server_csr, 
        "-CA", os.path.join(output_dir, "ca.crt"), 
        "-CAkey", os.path.join(output_dir, "ca.key"), 
        "-CAcreateserial", 
        "-out", server_crt, "-days", str(days), "-extfile", san_cnf
    ])
    
    os.remove(server_csr)
    os.remove(san_cnf)
    print("Server certificate created successfully.\n")

def generate_device_certs(output_dir, target_count, days):
    print(f"--- 3. Checking Device Certificates (Target: {target_count}) ---")
    
    devices_dir = os.path.join(output_dir, "devices")
    ensure_dir(devices_dir)
    manifest_path = os.path.join(devices_dir, "manifest.json")

    # Load existing manifest
    if os.path.exists(manifest_path):
        with open(manifest_path, "r") as f:
            manifest = json.load(f)
    else:
        manifest = {"devices": []}

    current_count = len(manifest["devices"])
    needed = target_count - current_count

    # Check if target is already met
    if needed <= 0:
        print(f"Target of {target_count} devices is already met (currently {current_count}). Skipping.\n")
        return

    print(f"Found {current_count} devices. Generating {needed} new certificates...")

    ca_crt = os.path.join(output_dir, "ca.crt")
    ca_key = os.path.join(output_dir, "ca.key")

    for i in range(needed):
        device_uuid = str(uuid.uuid4())
        device_dir = os.path.join(devices_dir, device_uuid)
        ensure_dir(device_dir)

        device_key = os.path.join(device_dir, f"{device_uuid}.key")
        device_csr = os.path.join(device_dir, f"{device_uuid}.csr")
        device_crt = os.path.join(device_dir, f"{device_uuid}.crt")

        # Generate Private Key
        run_cmd(["openssl", "genrsa", "-out", device_key, "2048"])
        
        # Generate CSR
        subj = f"/C=GR/O=Tilekoumanto Devices/CN={device_uuid}"
        run_cmd(["openssl", "req", "-new", "-key", device_key, "-out", device_csr, "-subj", subj])
        
        # Generate Certificate
        run_cmd([
            "openssl", "x509", "-req", "-in", device_csr, 
            "-CA", ca_crt, "-CAkey", ca_key, "-CAcreateserial", 
            "-out", device_crt, "-days", str(days)
        ])
        
        # Cleanup CSR
        os.remove(device_csr)
        
        # Append the UUID string to the manifest array
        manifest["devices"].append(device_uuid)
        
        print(f"Created: {device_uuid} ({i + 1}/{needed})")

    # Save updated manifest
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=4)
    
    print(f"\nManifest updated at: {manifest_path}\n")
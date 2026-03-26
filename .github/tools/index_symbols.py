# Copyright 2026 The JBMod Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os, subprocess, json, sys, requests

SYMBOL_SERVER_URL = "https://symbols.jbmod.dev/api/index"
S3_BASE_PATH = os.environ.get("S3_SYMBOL_PATH", "") 
INDEXER_SECRET = os.environ.get("SYMBOL_SERVER_SECRET")

def get_pdb_hash(path):
    """Extracts GUID+Age from a PDB using llvm-pdbutil."""
    try:
        result = subprocess.check_output(["llvm-pdbutil", "dump", "--summary", path], stderr=subprocess.DEVNULL).decode()
        guid = ""
        age = ""
        for line in result.splitlines():
            if "GUID:" in line:
                guid = line.split(":")[1].strip().replace("{", "").replace("}", "").replace("-", "")
            if "Age:" in line:
                age = line.split(":")[1].strip()
        return guid.upper() + age if guid and age else None
    except: return None

def get_elf_hash(path):
    """Extracts Build ID from an ELF using readelf."""
    try:
        result = subprocess.check_output(["readelf", "-n", path], stderr=subprocess.DEVNULL).decode()
        for line in result.splitlines():
            if "Build ID" in line:
                return line.split(":")[1].strip().upper()
        return None
    except: return None

def main(root_dir):
    if not INDEXER_SECRET:
        print("Error: SYMBOL_SERVER_SECRET environment variable not set.")
        sys.exit(1)

    payload = []
    for root, dirs, files in os.walk(root_dir):
        for name in files:
            file_path = os.path.join(root, name)
            rel_path = os.path.relpath(file_path, root_dir).replace("\\", "/")
            s3_key = f"{S3_BASE_PATH}/{rel_path}".strip("/")

            hash_val = None
            if name.lower().endswith(".pdb"):
                hash_val = get_pdb_hash(file_path)
            elif name.lower().endswith(".dbg"):
                hash_val = get_elf_hash(file_path)

            if hash_val:
                payload.append({"hash": hash_val, "key": s3_key})

    if not payload:
        print("No indexable symbols found.")
        return

    print(f"Uploading {len(payload)} mappings to {SYMBOL_SERVER_URL}...")
    headers = {
        "Authorization": f"Bearer {INDEXER_SECRET}",
        "Content-Type": "application/json"
    }
    
    response = requests.post(SYMBOL_SERVER_URL, headers=headers, json=payload)
    if response.status_code == 200:
        print("Successfully indexed symbols!")
    else:
        print(f"Indexing failed [{response.status_code}]: {response.text}")

if __name__ == "__main__":
    scan_path = sys.argv[1] if len(sys.argv) > 1 else "."
    main(scan_path)

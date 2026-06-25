# Linux Notes

> Encoding: UTF-8  
> Purpose: Linux / Ubuntu basic command notes for daily development, Git, SSH, SCP, permissions, links, and Arduino setup.

---

## 1. Basic Terminal Usage

### Open Terminal

```bash
Ctrl + Alt + T
```

### Check current directory

```bash
pwd
```

Example:

```bash
/home/mcy02605
```

### List files

```bash
ls
ls -l
ls -la
ls -lah
```

Meaning:

- `-l`: long format
- `-a`: show hidden files
- `-h`: human-readable size

---

## 2. Directory Navigation

### Go to home directory

```bash
cd ~
```

### Go to a specific directory

```bash
cd ~/dd/maochengyu
```

### Go to parent directory

```bash
cd ..
```

### Important path symbols

| Symbol | Meaning |
|---|---|
| `/` | root directory |
| `~` | current user's home directory |
| `.` | current directory |
| `..` | parent directory |

---

## 3. Create Files and Directories

### Create an empty file

```bash
touch aa.txt
```

### Create a directory

```bash
mkdir testfolder
```

### Create nested directories

```bash
mkdir -p project/src/modules
```

---

## 4. View File Contents

### Show whole file

```bash
cat aa.txt
```

### Show file page by page

```bash
less aa.txt
```

Quit `less`:

```text
q
```

### Show first lines

```bash
head aa.txt
head -n 5 aa.txt
```

### Show last lines

```bash
tail aa.txt
tail -n 20 aa.txt
```

### Follow log output

```bash
tail -f logfile.txt
```

Quit:

```text
Ctrl + C
```

---

## 5. Copy Files: cp

### Basic syntax

```bash
cp source target
```

### Copy a file and rename it

```bash
cp aa.txt bb.txt
```

### Copy a file to another directory

```bash
cp aa.txt ~/dd/maochengyu/
```

### Copy a directory

```bash
cp -r testfolder backupfolder
```

### Archive copy: preserve attributes

```bash
cp -a project project_backup
```

`cp -a` preserves permissions, timestamps, ownership, and symbolic links as much as possible.

### Safe copy with confirmation

```bash
cp -i aa.txt bb.txt
```

---

## 6. Move and Rename Files: mv

### Rename a file

```bash
mv old.txt new.txt
```

### Move a file to another directory

```bash
mv aa.txt ~/dd/maochengyu/
```

### Move a directory

```bash
mv testfolder ~/dd/maochengyu/
```

### Important note

If you are already inside a directory, you cannot move that directory by using its own name from inside it.

Example:

```bash
# Current directory:
# ~/dd/maochengyu/testfolder/20230223

cd ..
mv 20230223 ~/dd/maochengyu/
```

---

## 7. Delete Files and Directories: rm

### Delete a file

```bash
rm aa.txt
```

### Delete with confirmation

```bash
rm -i aa.txt
```

### Delete a directory

```bash
rm -r testfolder
```

### Delete a directory with confirmation

```bash
rm -ri testfolder
```

### Force delete a directory

```bash
rm -rf testfolder
```

Warning:

```bash
rm -rf
```

is very dangerous. Always check your current directory first:

```bash
pwd
ls
```

---

## 8. File Permissions

Example:

```bash
-rw-rw-r-- 1 mcy02605 mcy02605 22 Feb 8 01:13 bb.txt
```

Permission part:

```text
rw- rw- r--
```

Groups:

```text
user group others
```

Permission letters:

| Letter | Meaning |
|---|---|
| `r` | read |
| `w` | write |
| `x` | execute |

---

## 9. chmod

`chmod` means change mode. It changes file or directory permissions.

### Numeric method

| Permission | Number |
|---|---|
| `r` | 4 |
| `w` | 2 |
| `x` | 1 |

Common permissions:

| Permission | Meaning |
|---|---|
| `777` | everyone can read, write, execute |
| `755` | owner can modify, others can read/execute |
| `644` | normal file permission |
| `600` | private file |

Examples:

```bash
chmod 755 script.sh
chmod 644 notes.txt
chmod 600 private_key
```

### Add execute permission

```bash
chmod +x script.sh
```

### Recursive permission change

```bash
chmod -R 755 folder
```

Be careful with `-R`.

---

## 10. Directory Permissions

For files:

| Permission | Meaning |
|---|---|
| `r` | read file content |
| `w` | modify file content |
| `x` | execute file |

For directories:

| Permission | Meaning |
|---|---|
| `r` | list directory names |
| `w` | create/delete/rename entries |
| `x` | enter/traverse directory |

Important:

- Directory `x` means you can enter or pass through it.
- Directory `w` means you can create or delete names inside it.
- To list a directory normally, you usually need both `r` and `x`.

---

## 11. sudo and root

### Run command as administrator

```bash
sudo command
```

Example:

```bash
sudo cp aa.txt /opt/
sudo rm /opt/aa.txt
```

### Why sudo is needed

System directories such as `/opt`, `/etc`, `/usr`, and `/bin` usually belong to `root`.

Check directory permission:

```bash
ls -ld /opt
```

Example:

```text
drwxr-xr-x 2 root root 4096 Sep 11 2024 /opt
```

Normal users can enter and read `/opt`, but cannot create files there without `sudo`.

---

## 12. /opt Directory

`/opt` means optional software.

It is used for third-party or manually installed system-level software.

Examples:

```text
/opt/google/
/opt/idea/
/opt/myapp/
```

Do not use `/opt` as a personal working directory. Use your home directory instead:

```bash
/home/mcy02605
```

or:

```bash
~
```

---

## 13. vi Basic Usage

Open or create a file:

```bash
vi hello.txt
```

### Basic workflow

1. Press `i` to enter insert mode.
2. Type text.
3. Press `Esc` to return to normal mode.
4. Type `:wq` and press Enter to save and quit.

### Quit without saving

```text
Esc
:q!
Enter
```

### Delete current line

```text
dd
```

### Delete 3 lines

```text
3dd
```

### Delete all blank lines

```vim
:g/^\s*$/d
```

### Swap file warning

If vi shows:

```text
E325: ATTENTION
Found a swap file
```

Common choices:

| Key | Meaning |
|---|---|
| `R` | recover file |
| `D` | delete swap file |
| `Q` | quit |

If you do not need recovery, press `D`.

---

## 14. SSH

SSH means Secure Shell.

It is used to log in to another Linux machine remotely.

### Login syntax

```bash
ssh username@host
```

Examples:

```bash
ssh mcy02605@localhost
ssh mcy02605@127.0.0.1
```

### First-time connection

If you see:

```text
Are you sure you want to continue connecting?
```

Type:

```text
yes
```

### Check whether current shell is SSH

```bash
echo $SSH_CONNECTION
```

If it prints connection information, you are inside an SSH session.

### Exit SSH

```bash
exit
```

or:

```text
Ctrl + D
```

---

## 15. SCP

SCP means secure copy.

It copies files through SSH.

### Local to remote

```bash
scp aa.txt username@host:/target/path
```

Example:

```bash
scp aa.txt mcy02605@localhost:/tmp
```

### Remote to local

```bash
scp username@host:/remote/file .
```

Example:

```bash
scp mcy02605@localhost:/tmp/aa.txt .
```

### Copy directory

```bash
scp -r folder username@host:/target/path
```

### Important note

The colon `:` is very important.

Correct:

```bash
scp aa.txt mcy02605@localhost:/tmp
```

Incorrect:

```bash
scp aa.txt mcy02605@localhost/tmp
```

---

## 16. Git Basic Configuration

Check Git user name and email:

```bash
git config --global user.name
git config --global user.email
```

Set Git user name and email:

```bash
git config --global user.name "CY-M02605"
git config --global user.email "mcy02605@gmail.com"
```

Check Git status:

```bash
git status
```

Basic Git workflow:

```bash
git add .
git commit -m "Commit message"
git push
```

---

## 17. GitHub SSH Setup

### Check SSH directory

```bash
ls -al ~/.ssh
```

### Generate SSH key

```bash
ssh-keygen -t ed25519 -C "mcy02605@gmail.com"
```

When asked for file location, press Enter.

When asked for passphrase, you can press Enter twice for no passphrase.

### Show public key

```bash
cat ~/.ssh/id_ed25519.pub
```

Copy the whole line beginning with:

```text
ssh-ed25519
```

Add it to GitHub:

```text
GitHub -> Settings -> SSH and GPG keys -> New SSH key
```

### Test GitHub SSH

```bash
ssh -T git@github.com
```

Success message:

```text
Hi CY-M02605! You've successfully authenticated, but GitHub does not provide shell access.
```

### Clone with SSH

Use:

```bash
git clone git@github.com:CY-M02605/Records.git
```

Do not use HTTPS if you want passwordless Git operations.

---

## 18. Symbolic Link and Hard Link

### Hard link

```bash
ln original.txt hardlink.txt
```

Hard links share the same inode.

Check inode:

```bash
ls -li
```

If inode numbers are the same, they are hard links to the same file data.

### Symbolic link

```bash
ln -s original.txt symlink.txt
```

A symbolic link stores a path to another file.

Example output:

```text
symlink.txt -> original.txt
```

### Broken symbolic link

If the target file is deleted, the symbolic link remains but becomes broken.

```bash
rm original.txt
cat symlink.txt
```

This may show:

```text
No such file or directory
```

### touch and symbolic links

Default behavior:

```bash
touch symlink.txt
```

This follows the link and touches the target file.

Do not follow symbolic link:

```bash
touch -h symlink.txt
```

---

## 19. Find Files

Search by name:

```bash
find ~ -name "aa.txt"
```

Search current directory:

```bash
find . -name "*.txt"
```

---

## 20. grep

Search text in a file:

```bash
grep "error" log.txt
```

Show line numbers:

```bash
grep -n "error" log.txt
```

Search recursively:

```bash
grep -r "FailureDetector" .
```

---

## 21. Network Basics

### Show network interfaces

```bash
ip a
```

Common interfaces:

| Name | Meaning |
|---|---|
| `lo` | loopback interface |
| `wlp...` | Wi-Fi interface |
| `enp...` | wired Ethernet interface |

### Test internet connection

```bash
ping -c 3 8.8.8.8
```

### Check NetworkManager devices

```bash
nmcli device status
```

---

## 22. Ubuntu Version

Check Ubuntu version:

```bash
lsb_release -a
```

Check OS release file:

```bash
cat /etc/os-release
```

Check kernel version:

```bash
uname -r
```

---

## 23. Terminal Shortcuts

| Shortcut | Meaning |
|---|---|
| `Ctrl + Alt + T` | open terminal |
| `Ctrl + Shift + C` | copy in terminal |
| `Ctrl + Shift + V` | paste in terminal |
| `Ctrl + C` | stop current command |
| `Ctrl + L` | clear screen |
| `Ctrl + R` | search command history |
| `Tab` | autocomplete |

---

## 24. Arduino IDE on Ubuntu

### AppImage execution

Go to Downloads directory:

```bash
cd ~/下载
```

Make AppImage executable:

```bash
chmod +x arduino-ide_2.3.10_Linux_64bit.AppImage
```

Run:

```bash
./arduino-ide_2.3.10_Linux_64bit.AppImage
```

### FUSE error

If you see:

```text
error loading libfuse.so.2
AppImages require FUSE to run.
```

Install:

```bash
sudo apt update
sudo apt install libfuse2
```

### Move AppImage to Applications directory

```bash
mkdir -p ~/Applications
mv ~/下载/arduino-ide_2.3.10_Linux_64bit.AppImage ~/Applications/
```

Run:

```bash
~/Applications/arduino-ide_2.3.10_Linux_64bit.AppImage
```

---

## 25. Useful Daily Practice

### Check where you are

```bash
pwd
```

### Check files

```bash
ls -lah
```

### Create a practice file

```bash
echo "hello linux" > hello.txt
```

### Copy it

```bash
cp hello.txt hello_copy.txt
```

### Move it

```bash
mv hello_copy.txt ~/dd/
```

### Delete it safely

```bash
rm -i hello.txt
```

---

## 26. Safety Rules

1. Before deleting, always run:

```bash
pwd
ls
```

2. Be careful with:

```bash
rm -rf
sudo rm
chmod -R
```

3. Do not casually run commands copied from the internet.

4. System directories usually require `sudo`:

```text
/opt
/etc
/usr
/bin
```

5. Personal work should usually be under:

```bash
~
~/workspace
~/dd
```

---

## 27. Recommended Workspace Structure

```text
~/workspace
├── Embedded-Control-Architecture
├── ArduinoProjects
├── cpp-learning
├── linux-practice
└── notes
```

Create it:

```bash
mkdir -p ~/workspace/{Embedded-Control-Architecture,ArduinoProjects,cpp-learning,linux-practice,notes}
```

---

## 28. Mental Model

Linux learning is not about memorizing every command.

Remember:

1. What problem the command solves.
2. What kind of object it operates on: file, directory, process, network, Git repository.
3. How to check help:

```bash
man command
command --help
```

Examples:

```bash
man cp
cp --help
```


# ISA / IMAP Client in C

C networking project implementing a command-line IMAP client for downloading email messages from a remote mail server. The project was created for the Brno University of Technology ISA course in 2024.

## Project overview

The client connects to an IMAP server, authenticates with credentials provided in a local file and downloads messages to a selected output directory. It supports both unencrypted IMAP and encrypted IMAPS connections using OpenSSL.

The implementation handles command-line argument parsing, hostname resolution, IMAP requests and responses, TLS configuration, mailbox selection and message retrieval.

## Main features

- IMAP client implemented in C
- TCP networking and hostname resolution
- optional SSL/TLS encryption with OpenSSL
- authentication from a local credentials file
- configurable server port
- selectable mailbox, with `INBOX` as the default
- option to download only new messages
- option to download only message headers
- certificate file or certificate directory configuration
- downloaded messages stored in a user-selected output directory

## Command-line options

```text
./imapcl server [-p port] [-T [-c certfile] [-C certaddr]] [-n] [-h] -a auth_file [-b MAILBOX] -o out_dir
```

Important options:

- `server` — IMAP server hostname or IP address
- `-p port` — custom server port
- `-T` — use encrypted IMAPS connection
- `-c certfile` — certificate file used for TLS verification
- `-C certaddr` — directory containing certificates
- `-n` — download only new messages
- `-h` — download message headers only
- `-a auth_file` — local file containing username and password
- `-b MAILBOX` — mailbox name; defaults to `INBOX`
- `-o out_dir` — directory where downloaded messages are stored

## Authentication file

Create a local authentication file based on `auth_file.example`:

```text
username = your_username
password = your_password
```

Do not commit real credentials. Local authentication files are excluded through `.gitignore`.

## Build

The project uses GCC and links against OpenSSL:

```bash
make
```

This produces the `imapcl` executable.

Clean generated files with:

```bash
make clean
```

## Example

```bash
cp auth_file.example auth_file
# edit auth_file with your local credentials
./imapcl mail.example.com -T -a auth_file -o out_dir
```

## Repository structure

```text
.
├── README.md
├── .gitignore
├── Makefile
├── imapcl.c
├── imapcl.h
├── auth_file.example
└── manual.pdf
```

- `imapcl.c` — main client implementation
- `imapcl.h` — program structures, networking and OpenSSL declarations
- `Makefile` — GCC build configuration
- `manual.pdf` — original project documentation

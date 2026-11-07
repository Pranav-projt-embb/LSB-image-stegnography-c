# LSB Image Steganography in C

Hide a secret text message inside a `.bmp` image and get it back later, using the **Least Significant Bit (LSB)** technique. The image looks the same to the human eye, but it carries a hidden message.

The project has two parts:

- **Encoding**: hides a secret text file inside a BMP image and creates a new (stego) image.
- **Decoding**: reads the stego image and recovers the hidden text file.

---

## Table of Contents

1. [How It Works](#how-it-works)
2. [What Is Stored in the Image](#what-is-stored-in-the-image)
3. [Project Structure](#project-structure)
4. [Requirements](#requirements)
5. [Clone the Repository](#clone-the-repository)
6. [Compile](#compile)
7. [Usage](#usage)
8. [Sample Output](#sample-output)
9. [Error Handling](#error-handling)
10. [Limitations](#limitations)
11. [Author](#author)

---

## How It Works

A BMP file has a **54-byte header** followed by the **pixel data** (RGB bytes).

- The 54-byte header is copied to the output image **unchanged**.
- Every pixel byte can hide **1 bit** of the secret data, by replacing its **least significant bit (LSB)**.
- Changing the LSB changes a colour value by at most 1 (out of 255), so the change cannot be seen.

**Hiding one character needs 8 image bytes** (one per bit, MSB first):

```c
image_byte = image_byte & (~1);              // clear the LSB
bit = data & (1 << (7 - i));                 // pick bit i of the data, MSB first
image_byte = image_byte | (bit >> (7 - i));  // put that bit into the LSB
```

**Decoding** reads the LSBs back in the same order:

```c
data = (data << 1) | (image_byte & 1);
```

Numbers (such as the file size) are stored the same way, but an `int` has 32 bits, so it uses **32 image bytes**.

---

## What Is Stored in the Image

The decoder only gets a stream of LSBs, so the encoder writes some extra information before the message. This is the order, right after the 54-byte header:

| Order | Data | Image bytes used |
|-------|------|------------------|
| 1 | Magic string (`#*`) | 2 x 8 = 16 |
| 2 | Secret file extension size (e.g. 4) | 32 |
| 3 | Secret file extension (e.g. `.txt`) | 4 x 8 = 32 |
| 4 | Secret file size (e.g. 27) | 32 |
| 5 | Secret file data (e.g. `hello we are hiding message`) | 27 x 8 = 216 |

For the sample `secret.txt` (`hello we are hiding message`, 27 characters), this uses 16 + 32 + 32 + 32 + 216 = **328 image bytes**. The rest of the image is copied as it is.

**Why each one is stored:**

- **Magic string**: tells the decoder whether the image was encoded by this program. If it does not match, decoding stops with an error.
- **Extension size**: the decoder needs to know how many characters of extension to read.
- **Extension**: the decoder uses it to create the output file with the right extension.
- **File size**: the decoder needs to know how many bytes of data to read, and where the message ends.
- **File data**: the secret message itself.

**Capacity check:** the image must have more bytes than `16 + 32 + (extension length x 8) + 32 + (file size x 8)`.

---

## Project Structure

```
LSB_image_stegnography/
├── Decode/
│   ├── Inc/
│   │   └── decoding.h
│   └── Src/
│       └── decoding.c
├── Encode/
│   ├── INC/
│   │   └── encode.h
│   └── SRC/
│       └── encode.c
├── main_c/
│   ├── main.c
│   ├── common.h
│   └── types.h
├── Source Image/
│   └── beautiful.bmp
├── Secret message text file/
│   └── secret.txt
├── Encoded image/
│   └── stego.bmp
├── Decoded secret message txt file/
│   └── decoded.txt
├── Result/
└── README.md
```

| File / Folder | Purpose |
|---------------|---------|
| `main_c/main.c` | Reads the command line and calls encoding or decoding |
| `main_c/common.h` | Defines the magic string |
| `main_c/types.h` | Common types (`Status`, `OperationType`) |
| `Encode/` | Encoding code: `encode.c` and `encode.h` |
| `Decode/` | Decoding code: `decoding.c` and `decoding.h` |
| `Source Image/` | The original `.bmp` image |
| `Secret message text file/` | The text file to hide |
| `Encoded image/` | The output stego image |
| `Decoded secret message txt file/` | The recovered text file |
| `Result/` | Terminal output of encoding and decoding |

---

## Requirements

- Linux (or any system with a terminal)
- GCC compiler
- Git (only needed to clone the repository)

---

## Clone the Repository

1. Open a terminal and clone the project:

```bash
git clone https://github.com/Pranav-projt-embb/LSB-image-stegnography-c.git
```

2. Go into the project folder:

```bash
cd LSB_image_stegnography
```

3. Continue with the **Compile** and **Usage** steps below. All commands must be run from inside this folder.

If you do not want to use Git, click **Code > Download ZIP** on the GitHub page, extract the ZIP, and open the extracted folder in the terminal.

---

## Compile

Open a terminal inside the `LSB_image_stegnography` folder and run:

```bash
gcc -I Encode/INC -I Decode/Inc -I main_c main_c/main.c Encode/SRC/encode.c Decode/Src/decoding.c -o a.out
```

---

## Usage

### Encode

```bash
./a.out -e <.bmp_file> <.text_file> [output_image.bmp]
```

```bash
./a.out -e "Source Image/beautiful.bmp" "Secret message text file/secret.txt" "Encoded image/stego.bmp"
```

If the output file is not given, `stego.bmp` is created in the current folder.

### Decode

```bash
./a.out -d <stego_image.bmp> [output_file]
```

```bash
./a.out -d "Encoded image/stego.bmp" "Decoded secret message txt file/decoded.txt"
```

If the output file is not given, `decoded` plus the original extension (for example `decoded.txt`) is created in the current folder.

### View the decoded message

```bash
cat "Decoded secret message txt file/decoded.txt"
```

Output:

```
hello we are hiding message
```

### Important notes

- Folder names with spaces **must be inside quotes**.
- The output folders must **already exist**. The program creates files, not folders.
- Folder and file names are case-sensitive on Linux (`INC` and `Inc` are different).

---

## Sample Output

The terminal output of encoding and decoding is saved in the **`Result`** folder.

**Encoding**

```
INFO: Read and validate encode arguments is successful
INFO: Opening required files
INFO: Opened all required files
INFO: Checking for secret.txt size
width = 1024
height = 768
INFO: Image has enough capacity to store the secret data
INFO: Copying image header
INFO: Header copied successfully
INFO: Encoding magic string signature
INFO: Done
INFO: Encoding secret file extension size
INFO: Done
INFO: Encoding secret file extension
INFO: Done
INFO: Encoding secret file size
INFO: Done
INFO: Encoding secret file data
INFO: Done
INFO: Copying left over data
INFO: Done
INFO: Encoding Done Successfully
```

**Decoding**

```
INFO: Read and validate decode arguments is successful
INFO: Opening required files
INFO: Opened stego.bmp
INFO: Decoding magic string signature
INFO: Done
INFO: Decoding output file extension size
INFO: Done
INFO: Decoding output file extension
INFO: Done
INFO: Opening output file
INFO: Opened decoded.txt
INFO: Decoding file size
INFO: Done
INFO: Decoding file data
INFO: Done
INFO: Decoding Done Successfully
```

---

## Error Handling

The program prints a clear message for these cases:

- No arguments or wrong arguments (prints the usage)
- The source or output image is not a `.bmp` file
- The secret file has no extension, or the extension is too long
- The secret file is empty
- The image does not have enough capacity
- A file cannot be opened
- The magic string does not match (the image is not a stego image)

---

## Limitations

- Works only with `.bmp` images (24-bit, 54-byte header).
- The secret file can be at most **100 bytes**.
- The secret file extension can have at most **4 characters** including the dot (for example `.txt`, `.c`).
-Here only the default magic string (`#*`) is supported.

---

## Author

**More Pranav Shivaji**

- GitHub: [More Pranav Shivaji](https://github.com/Pranav-projt-embb)
- LinkedIn: [More Pranav](https://www.linkedin.com/in/more-pranav-b98290381)

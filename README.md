# Nibble CPU
# Dateien
- [Dokumentation_Nibble_CPU.pdf](https://github.com/Knoch1/NibbleCPU/blob/main/Dokumentation_Nibble_CPU.pdf) is the Dokumentation of the hole Projekt
- [Eagle_circuits.zip](https://github.com/Knoch1/NibbleCPU/blob/main/Eagle_circuits.zip) are the 3 Circuits (shematic and board) of this Project
- [nalo.cpp](https://github.com/Knoch1/NibbleCPU/blob/main/nalo.cpp) is the source code of the assambler.
- [nalo.1](https://github.com/Knoch1/NibbleCPU/blob/main/nalo.1) is the manual, which can be opened with the command `man nalo`.
- [nal.vim](https://github.com/Knoch1/NibbleCPU/blob/main/nal.vim) is for syntax highlighting in Neovim.
- [example1.nal](https://github.com/Knoch1/NibbleCPU/blob/main/example1.nal) is a example of a programm in the Nibble Assambly Language(NAL).
- [nalo](https://github.com/Knoch1/NibbleCPU/blob/main/nalo) is the compiled version of the assambler. 




## Installation

First, download the required software from GitHub:  
[https://github.com/Knoch1/NibbleCPU/](https://github.com/Knoch1/NibbleCPU/)

Then navigate to the downloaded folder and execute the following commands:

```bash
sudo cp nalo /usr/local/bin/nalo
cp nalo.1 /usr/share/man/man1
cp nalo.vim ~/.config/nvim/syntax
```
## Programming
After downloading and installing nalo, you can enter the following commands in the terminal to display the available commands for programming:

```bash

nalo -h
```
or

```bash

man nalo
```
These commands will output a list of available commands and options that can be used during programming.
To get the correct syntax highlighting, enter this command in the running Neovim session.
```vim
:set syntax=nal
```
## Assembling and Uploading
To upload the program, connect the CPU to the PC via a cable and then use the following command:

```bash

nalo file.nal usbport
```
Replace file with the name of your file and usbport with the corresponding port where you want to upload the program.
The assembler may refuse to upload if the port is not found or if there is a programming error.
If you want to upload the program despite errors, add -f (force upload) after the file name, like this:

```bash

nalo file.nal -f usbport
```
It is also possible to get a binary view of the uploaded file by simply adding -d (debug):

```bash

nalo file.nal -f -d usbport
```
You can also assemble multiple files simultaneously by simply appending them one after the other:

```bash

nalo file1.nal file2.nal -d -f usbport
```
After uploading, you need to press the reset button once so that the program actually starts at the first line.

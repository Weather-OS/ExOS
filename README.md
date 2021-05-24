# ExOS
A Hardware-based Operating system where you have kernel access to every hardware piece of your computer.  
### To Build the code:  
  
  #### Required : 
  - Linux Operating system
  - qemu (Sudo apt install qemu)
  - GCC Compiler (Sudo apt install gcc-default)
  - NASM Compiler (Sudo apt install nasm)
1. Copy the repository to a folder in your computer.
2. Execute the file named : run.sh  
  
  
### To run the ISO File :  
  
  #### Required : 
  - Any operating system  
  - A copy of VMware or VirtualBox  
  - Optional : CPU Emulator.  
  
  #### In VirtualBox :
1. Make a new Virtual machine.
2. Give it around 4 MB of RAM with 1 CPU core
3. Give it around 16 MB of HARD DISK DRIVE
4. After creation go to settings
5. Go to Drives
6. Go to disk
7. Press "Add CD"
8. Add the ISO image
9. run the virtual machine.

## ExOs Operating System usage.
ExOS is mostly editkernel based operating system where you can add custom commands if you have a Brief and extended C Knowledge.

  #### Commonly used Commands:
  - PRINT : Print a word
  - SCRIPT : Execute a .S file ("default hello.s")
  - EDIT : Edit a Shell/Kernel File.
  - ERASE : To clear the shell

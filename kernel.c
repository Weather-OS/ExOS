#include "kernel.h"
#include "utils.h"
#include "char.h"
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include </home/office/Desktop/KERNEL/ext/ata.h>
#include </home/office/Desktop/KERNEL/ext/ext2.h>
#define WIDTH 80
#define HEIGHT 25
#define KERNEL_LOAD (void* )

uint32 vga_index;
static uint32 next_line_index = 1;
uint8 g_fore_color = WHITE, g_back_color = BLUE;
int digit_ascii_codes[10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

inode_t* const inode = (inode_t*) 0xBEEF0;
static void* const scratch_buff = (void*)0x80000;
superblock_t* const superblock = (superblock_t*)0x10000;
blockgroup_desctable_t* bgdt = (blockgroup_desctable_t*) (0x10000 + 1024);
void ata_setup();



uint16_t* vga = (uint16_t*)0xb8000;



/*
this is same as we did in our assembly code for vga_print_char

vga_print_char:
  mov di, word[VGA_INDEX]
  mov al, byte[VGA_CHAR]

  mov ah, byte[VGA_BACK_COLOR]
  sal ah, 4
  or ah, byte[VGA_FORE_COLOR]

  mov [es:di], ax

  ret

*/
uint16 vga_entry(unsigned char ch, uint8 fore_color, uint8 back_color) 
{
  uint16 ax = 0;
  uint8 ah = 0, al = 0;

  ah = back_color;
  ah <<= 4;
  ah |= fore_color;
  ax = ah;
  ax <<= 8;
  al = ch;
  ax |= al;

  return ax;
}

static void read_block(void* dst, uint64_t lba, uint32_t cnt) {
  int status;
}


static inline void zero(uint8_t* mem,  size_t len) {
  while(len--)
    *(mem++) = 0;
}

static void putchar(uint8_t ch) {
  static size_t idx = 0;
  static uint16_t* const VGA = (uint16_t*) 0xb8002;
  
  if(idx >= WIDTH*HEIGHT) {
    idx = 0;
    zero((uint8_t*)VGA, WIDTH*HEIGHT*2);
  }

  if(ch == '\n') {
    idx /= WIDTH;
    idx++;
    idx *= WIDTH;
  } else {
    VGA[idx++] = 0x0D00 | ch;
  }
}

void read_inode(void* buff, uint32_t inode) {
  uint32_t bg = (inode-1) / superblock->n_inodes_per_group;

  uint32_t idx = (inode-1) % superblock->n_inodes_per_group;

  uint32_t blk = bgdt[bg].lba_inode_table + ((idx * superblock->sizeof_inode) >> (10+superblock->block_size));
  uint32_t offset = (idx * superblock->sizeof_inode) & ((1 << (10+superblock->block_size)) - 1);

  read_block(buff, blk, 2);

  uint8_t* b = (uint8_t*)(buff + offset);
  uint8_t* d = (uint8_t*)buff;
  for(size_t i = 0; i < superblock->sizeof_inode; i++) {
    d[i] = b[i];
  }
}
static void print_u8(uint8_t x) {
  static const char hexstr[] = "0123456789ABCDEF";
  putchar(hexstr[x >> 4]);
  putchar(hexstr[x & 0xF]);
}

static void print_u16(uint16_t x) {
  print_u8(x >> 8);
  print_u8(x >> 0);
}
static bool streq(char* un, char* dos, size_t cnt) {
  while(cnt--) {
    if(*un != *dos)
      return false;
    un++;
    dos++;
  }
  return true;
}


static uint32_t find_entry(uint32_t parent_inode, const char* name) {

  uint32_t ret = 0;

  read_inode(inode, parent_inode);
  
  dirent_t* dirent = (dirent_t*) (scratch_buff + 4096);

  size_t n = 0;
  while(inode->dbptr[n]) {
    read_block(dirent, inode->dbptr[n], 1);
    while(dirent->inode) {
      if(streq(dirent->name, name, dirent->namelen)) {
        ret = dirent->inode;
        goto full_break;
      }
      dirent = (dirent_t*)(((void*)dirent) + dirent->sz);
    }

    n++;
  }

  full_break:

  return ret;
}

static void print_u32(uint32_t x) {
  print_u16(x >> 16);
  print_u16(x >> 0);
}


void clear_vga_buffer(uint16 **buffer, uint8 fore_color, uint8 back_color)
{
  uint32 i;
  for(i = 0; i < BUFSIZE; i++){
    (*buffer)[i] = vga_entry(NULL, fore_color, back_color);
  }
  next_line_index = 1;
  vga_index = 0;
}

void init_vga(uint8 fore_color, uint8 back_color)
{
  vga_buffer = (uint16*)VGA_ADDRESS;
  clear_vga_buffer(&vga_buffer, fore_color, back_color);
  g_fore_color = fore_color;
  g_back_color = back_color;
}

void print_new_line()
{
  if(next_line_index >= 26){
    next_line_index = 0;
    clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
  }
  vga_index = 80*next_line_index;
  next_line_index++;
}
void goto_previous_line(){
	next_line_index--;
	vga_index = 80*next_line_index;
	
}
void clear_screen(){
	next_line_index = 0;
    clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
}
void print_char(char ch)
{
  vga_buffer[vga_index] = vga_entry(ch, g_fore_color, g_back_color);
  vga_index++;
}

void print_string(char *str)
{
  uint32 index = 0;
  while(str[index]){
    print_char(str[index]);
    index++;
  }
}

void print_int(int num)
{
  char str_num[digit_count(num)+1];
  itoa(num, str_num);
  print_string(str_num);
}

uint8 inb(uint16 port)
{
  uint8 ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
  return ret;
}

void outb(uint16 port, uint8 data)
{
  asm volatile("outb %0, %1" : "=a"(data) : "d"(port));
}

char get_input_keycode()
{
  char ch = 0;
  while((ch = inb(KEYBOARD_PORT)) != 0){
    if(ch > 0)
      return ch;
  }
  return ch;
}

/*
keep the cpu busy for doing nothing(nop)
so that io port will not be processed by cpu
here timer can also be used, but lets do this in looping counter
*/
void wait_for_io(uint32 timer_count)
{
  while(1){
    asm volatile("nop");
    timer_count--;
    if(timer_count <= 0)
      break;
    }
}

void sleep(uint32 timer_count)
{
  wait_for_io(timer_count);
}
void edit( char FILE_NAME[100] ){
char keyedit = 0;
char addedit[100];
int Super = 1;
int sas;
int NEWFILEM = 1;
char Scripts[] = "HELLO.S";

    int count1 = 0, count2 = 0, i, j, flag;
    
	init_vga(WHITE, BLACK);
	 while (Scripts[count1] != '\0')
        count1++;
    while (FILE_NAME[count2] != '\0')
        count2++;
    for (i = 0; i <= count1 - count2; i++)
    {
        for (j = i; j < i + count2; j++)
            flag = 1;
            if (Scripts[j] != FILE_NAME[j - i])
            {
                flag = 0;
                break;
            }
        
        if (flag == 1)
            break;
}
    if (FILE_NAME[1] == Scripts[1] && FILE_NAME[2] == Scripts[2] && FILE_NAME[3] == Scripts[3] && FILE_NAME[4] == Scripts[4] && FILE_NAME[5] == Scripts[5]){
    	print_new_line();
	print_string("Editing : ");
	uint32_t sys = find_entry(2, "sys");
	uint32_t stage2bin = find_entry(sys, "hello.txt");
	print_u32(stage2bin);
	int be = 1;
	do { 
		be++;
	} while (FILE_NAME[be] != '\0');
	for (int j = 1; j <= be; j++){
    	print_char(FILE_NAME[j]);
	}
	} else {
		print_new_line();
	print_string("New file: ");
	uint32_t sys = find_entry(2, "sys");
	uint32_t stage2bin = find_entry(sys, "hello.txt");
	print_u32(stage2bin);
	int be = 1;
	do { 
		be++;
	} while (FILE_NAME[be] != '\0');
	for (int j = 0; j <= be; j++){
    	print_char(FILE_NAME[j]);
	}
    print_new_line();
    print_string("Press TAB to exit. Press UP and DOWN arrow keys to move UP or DOWN.");
	}
	print_new_line();

	for (int i = 0; i <= 9000000; i++){
		EDIT:
			keyedit = get_input_keycode();	
			do{
				keyedit = get_input_keycode();	
				if (keyedit == KEY_ENTER){
					print_new_line();
					addedit[Super] = ' ';
					Super++;
				} else if (keyedit == KEY_TAB) {
					install();
				} else if (keyedit == KEY_UP){
					goto_previous_line();
				} else if (keyedit == KEY_DOWN){
					print_new_line();
				} else {
				addedit[Super] = get_ascii_char(keyedit);
			char s = addedit[Super];
   			print_char(s);
   			 Super++;
   			 
			}  for(i=0;i<700000000;i++)
   				 {
 
    				sas++;
						} sleep(0x02FFFFFF);
       				 } 	while(keyedit > 0);
				}
			}
	
			
			
void install(){
    init_vga(WHITE, BLUE); 
	 int chas = 1;
	 char keycommand = 0;
  char addthis[100];
   char command;
   char printvalue[212];
   char scriptvalue[20];
   char editvalue[20];
   int NEXTPG = 0;
   int PRINTS = 1;
    	int sa;
for (int i = 0; i <= 9000000; i++){

        
  print_string("SHELL* ");
 
  	HOME:
  keycommand = get_input_keycode();
  do{
  		keycommand = get_input_keycode();
    if (keycommand == KEY_ENTER){
    	NEXTPG++;
    	if (NEXTPG == 7){
    		NEXTPG = 1;
    		goto HOME;
		}
	char s[] = "PRINT";
    if (addthis[1] == 'P' && addthis[2] == 'R' && addthis[3] == 'I' && addthis[4] == 'N' && addthis[5] == 'T' && addthis[6] == '\0'){
    if (addthis[7] == '\0'){
	print_new_line();
    	print_string("NO PRINT VALUE ERROR");
      print_new_line();
        print_string("SHELL* ");
	} else {
        print_new_line();
		int printvalueex = 7;
		do {
			printvalueex++;
		} while (addthis[printvalueex] != '\0');
		for (int k = 0; k <= printvalueex; k++){
			print_char(addthis[k+7]);
		}			
		print_new_line();
		  print_string("SHELL* ");
			for (int j = 7; j <= PRINTS; j++){
    			printvalue[j] = '\0';
			}
			int c = 1;
	do { 
		c++;
	} while (addthis[c] != '\0');
	for (int j = 1; j <= c+10; j++){
    	addthis[j] = '\0';
	}
	chas = 1;
	}
	int c = 1;
	do { 
		c++;
	} while (addthis[c] != '\0');
	for (int j = 1; j <= c; j++){
    	addthis[j] = '\0';
	}
	chas = 1;
    } else if (addthis[1] == 'S' && addthis[2] == 'C' && addthis[3] == 'R' && addthis[4] == 'I' && addthis[5] == 'P' && addthis[6] == 'T' && addthis[7] == '\0'){
    	if (addthis[7] == '\0'){
	print_new_line();
    	print_string("NO SCRIPT WAS FOUND ERROR");
      print_new_line();
        print_string("SHELL* ");
	} else {
        print_new_line();
        
		int sd = 8;
		do {
			sd++;
		} while (addthis[sd] != '\0');
		for (int k = 8; k <= sd; k++){
			scriptvalue[k-7] = (addthis[k]);
		}			
		if (scriptvalue[1] == 'H' && scriptvalue[2] == 'E' && scriptvalue[3] == 'L' && scriptvalue[4] == 'L' && scriptvalue[5] == 'O' && scriptvalue[6] == '.' && scriptvalue[7] == 'S' && addthis[8] == '\0'){
			print_string("HELLO WORLD!");
		} else {
		print_string("NO KNOWN SCRIPT FILE KNOWN AS ");
			int sds = 1;
		do {
			sds++;
		} while (scriptvalue[sds] != '\0');
		for (int k = 1; k <= sd; k++){
			print_char(scriptvalue[k]);
		}
		
		print_string(" WAS FOUND ERROR.");
	}
		print_new_line();
		  print_string("SHELL* ");
			for (int j = 7; j <= PRINTS; j++){
    			printvalue[j] = '\0';
			}
			int c = 1;
	do { 
		c++;
	} while (addthis[c] != '\0');
	for (int j = 1; j <= c+10; j++){
    	addthis[j] = '\0';
	}
	chas = 1;
	}
	for (int j = 1; j <= 20; j++){
    	scriptvalue[j] = '\0';
		}
	int c = 1;
	do { 
		c++;
	} while (addthis[c] != '\0');
	for (int j = 1; j <= c; j++){
    	addthis[j] = '\0';
	}
	chas = 1;
    	
	} else if (addthis[1] == 'E' && addthis[2] == 'D' && addthis[3] == 'I' && addthis[4] == 'T' && addthis[5] == '\0'){
    if (addthis[6] == '\0'){
	print_new_line();
    	print_string("NO EDIT INPUT ERROR");
      print_new_line();
        print_string("SHELL* ");
	} else {
        print_new_line();
		int editex = 6;
		do {
			editex++;
		} while (addthis[editex] != '\0');
		for (int k = 0; k <= editex; k++){
			editvalue[k+1] = addthis[k+5];
			
		}	
		edit( editvalue );		
		print_new_line();
		  print_string("SHELL* ");
			for (int j = 7; j <= PRINTS; j++){
    			addthis[j] = '\0';
			}
			int c = 1;
	do { 
		c++;
	} while (addthis[c] != '\0');
	for (int j = 1; j <= c+10; j++){
    	addthis[j] = '\0';
	}
	chas = 1;
	}
	int c = 1;
	do { 
		c++;
	} while (addthis[c] != '\0');
	for (int j = 1; j <= c; j++){
    	addthis[j] = '\0';
	}
	chas = 1;
		
	} else if (addthis[1] == 'E' && addthis[2] == 'R' && addthis[3] == 'A' && addthis[4] == 'S' && addthis[5] == 'E' && addthis[6] == '\0') {
		clear_screen();
		print_string("SHELL* ");
		int c = 1;
	do { 
		c++;
	} while (addthis[c] != '\0');
	for (int j = 1; j <= c+10; j++){
    	addthis[j] = '\0';
	}
	chas = 1;
	} else {
	
          print_new_line();
    print_string("INVALID COMMAND: \"");
    int ca = 1;
	do { 
		ca++;
	} while (addthis[ca] != '\0');
	for (int j = 1; j <= ca-1; j++){
    	print_char(addthis[j]);
	}
		print_string("\"");
    
      print_new_line();
        print_string("SHELL* ");
    
	int c = 1;
	do { 
		c++;
	} while (addthis[c] != '\0');
	for (int j = 1; j <= c+10; j++){
    	addthis[j] = '\0';
	}
	chas = 1;
    	
	
	}
	} else {
	addthis[chas] = get_ascii_char(keycommand);
	char n = addthis[chas];
    print_char(n);
    chas++;
    PRINTS++;
	}  for(i=0;i<700000000;i++)
    {
 
    	sa++;
	} sleep(0x02FFFFFF);
        } while(keycommand > 0);
}

  
  
}
void test_input()
{
  char ch = 0;
  char keycode = 0;
  char myString[] = "1";
  char installnode = 0;
    keycode = get_input_keycode();
  do{

    if(keycode == KEY_1){
        install();
      print_new_line();
    } else {
	
		ch = get_ascii_char(keycode);
      print_char(ch);
	
    	

    } sleep(0x02FFFFFF);
  }while(keycode > 0);
}

void kernel_entry()
{

		
  init_vga(WHITE, BLACK);
  print_string("Welcome to Custom made kernel!");
  print_new_line();
  print_string("Which option do you want to choose?");
  print_new_line();
  print_string("1: livecd");
  print_new_line();
  install();
  test_input();
    test_input();
      test_input();
        test_input();
          test_input();
            test_input();
              test_input();
                test_input();
  

}


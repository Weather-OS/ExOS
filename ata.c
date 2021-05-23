#include </home/office/Desktop/KERNEL/ext/ata.h>
#include <stdint.h>
#include <stddef.h>
#include </home/office/Desktop/KERNEL/common/def.h>
#include </home/office/Desktop/KERNEL/common/io.h>

#define CONTROL_BASE 0x3F6
#define CONTROL_REG (CONTROL_BASE + 0)

#define IO_BASE 0x1F0
#define STATUS_REG (IO_BASE + 7)
#define COMMAND_REG (IO_BASE + 7)

#define ATA_BUSY (1 << 7)
#define ATA_NEIN (1 << 1)
#define ATA_DRQ (1 << 3)
#define ATA_RDY (1 << 6)
#define ATA_LBA (1 << 6)

static inline uint8_t ata_getstatus() {
  return inb(STATUS_REG);
}

static inline uint8_t ata_wait() {
  uint8_t status;
  while((status = ata_getstatus()) & ATA_BUSY)
    ;

  return status;
}

void ata_setup() {
  /* set nEIN bit to disable IRQs */
  outb(CONTROL_REG, ATA_NEIN);

}

void ata_reset() {
  outb(CONTROL_REG, 4);
  outb(CONTROL_REG, 0);
  inb(CONTROL_REG);
  inb(CONTROL_REG);
  inb(CONTROL_REG);
  inb(CONTROL_REG);
  while(!(ata_wait() & ATA_RDY))
    ;
}

int ata_read(void* _dst, uint64_t lba, uint16_t cnt) {
  uint16_t* dst = (uint16_t*) _dst;

  while(!(ata_wait() & ATA_RDY))
    ;


#ifdef PIO_48
  /* select master drive (default for now)*/
  outb(IO_BASE + 6, ATA_LBA);

  outb(IO_BASE + 2, cnt >> 8);
  outb(IO_BASE + 3, lba >> 24);
  outb(IO_BASE + 4, lba >> 32);
  outb(IO_BASE + 5, 0);

  outb(IO_BASE + 2, cnt >> 0);
  outb(IO_BASE + 3, lba >> 0);
  outb(IO_BASE + 4, lba >> 8);
  outb(IO_BASE + 5, lba >> 16);

  /* say we want to read ext (0x24 to command) */
  outb(COMMAND_REG, 0x24);

#else
  outb(IO_BASE + 6, ATA_LBA | ((lba >> 24) & 0xF));
  outb(IO_BASE + 2, cnt);
  outb(IO_BASE + 3, lba >> 0);
  outb(IO_BASE + 4, lba >> 8);
  outb(IO_BASE + 5, lba >> 16);

  outb(COMMAND_REG, 0x20);

#endif /* PIO_48 */
  uint8_t status;

  for(size_t i = 0; i < 5; i++) {
    if((status = ata_getstatus()) & ATA_BUSY)
      continue;
    if(status & 8)
      goto data_ready;
  }

  while(cnt--) {
    status = ata_wait();
    if(status & 0x21) {
      return status;
    }
    
    data_ready:

    if(status & ATA_DRQ) {
      for(size_t i = 0; i < 256; i++) {
        *(dst++) = inw(IO_BASE);
      }

      /* delay 400ns */
      inb(0x1f0);
      inb(0x1f0);
      inb(0x1f0);
      inb(0x1f0);

    } else {
      return ata_getstatus();
    }
  }
  return 0;
}


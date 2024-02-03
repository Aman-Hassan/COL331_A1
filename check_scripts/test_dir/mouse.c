#include "types.h"
#include "defs.h"
#include "x86.h"
#include "mouse.h"
#include "traps.h"

// Wait until the mouse controller is ready for us to send a packet
void 
mousewait_send(void) 
{
    while (inb(MSSTATP) & 0x02) 
        ;
}

// Wait until the mouse controller has data for us to receive
void 
mousewait_recv(void) 
{
    while(!(inb(MSSTATP) & 0x01)) 
        ;
}

// Send a one-byte command to the mouse controller, and wait for it
// to be properly acknowledged
void 
mousecmd(uchar cmd) 
{
    mousewait_send(); //Wait for mouse controller to be ready for packet to be sent
    outb(MSSTATP, PS2MS); //Tell the mouse controller we're sending a command to address the mouse
    mousewait_send(); //Wait for mouse controller to be ready for packet to be sent
    outb(MSDATAP, cmd); //Send the command
    mousewait_recv(); //Wait for mouse controller to have data to be read
    if (inb(MSDATAP) != MSACK) 
        panic("Mouse did not acknowledge command");
}

void
mouseinit(void)
{
    // Enabling the mouse
    mousewait_send(); //Wait for mouse controller to be ready for packet to be sent
    outb(MSSTATP, MSEN); //Enable the auxiliary mouse device

    //Changing the Compaq Status Byte
    mousewait_send(); //Wait for mouse controller to be ready for packet to be sent
    outb(MSSTATP, 0x20); //Tell the mouse controller we're selecting the Compaq Status Byte
    
    mousewait_recv(); //Wait for mouse controller to have data to be read
    uchar status = (inb(MSDATAP) | 0x02); //Set the correct bit of the status byte -> Enable interrupts
    
    mousewait_send(); //Wait for mouse controller to be ready for packet to be sent
    outb(MSSTATP, 0x60); //Tell the mouse controller we're setting the updated Compaq Status Byte
    mousewait_send(); //Wait for mouse controller to be ready for packet to be sent
    outb(MSDATAP, status); //Send the updated status byte

    //Select default settings for mouse -> 0xF6
    mousecmd(0xF6);

    //Activate mouse to recieve interrupts -> 0xF4
    mousecmd(0xF4);

    //Tell interrupt controller to enable IRQ_MOUSE (IRQ12) on CPU 0
    ioapicenable(IRQ_MOUSE, 0);

    //Print message (for autograder)
    cprintf("Mouse has been initialized\n");
}

void
mouseintr(void)
{
    // Read from mouse buffer 1 byte at a time -> 3 bytes per packet
    // 1st Byte -> Status Byte 
    // 2nd Byte -> X Movement
    // 3rd Byte -> Y Movement

    // Drain buffer - Read until there are no more bytes to read
    while (inb(MSSTATP) & 0x01){
        
        //Read the mouse packet (3 bytes per packet), 1 packet at a time
        // mousewait_recv();
        uchar byte1 = inb(MSDATAP);

        // We are right now only going to print whether mouse was clicked (to be improved later)
        if (byte1 & 0x01){
            cprintf("LEFT\n");
        }
        if (byte1 & 0x02){
            cprintf("RIGHT\n");
        }
        if (byte1 & 0x04){
            cprintf("MID\n");
        }

        // mousewait_recv();
        inb(MSDATAP); //Read and ignore byte 2 of packet
        // mousewait_recv();
        inb(MSDATAP); //Read and ignore byte 3 of packet
    }
}
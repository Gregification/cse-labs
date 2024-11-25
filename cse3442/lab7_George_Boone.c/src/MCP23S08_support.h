/*
 * MCP23S08_support.h
 *
 *  Created on: Nov 24, 2024
 *      Author: George Boone
 *
 * MCP23S08 datasheet : https://ww1.microchip.com/downloads/en/devicedoc/21919e.pdf
 */

#ifndef SRC_MCP23S08_SUPPORT_H_
#define SRC_MCP23S08_SUPPORT_H_

//----------------------------opcode-------------------------------
// see figure 1-5
#define MCP23S08_OPC            0x40            // base op code
#define MCP23S08_OPC_HADDR_M    0x06            // hardware address mask
#define MCP23S08_OPC_RW_M       0x01            // read / write mask (read=1)

//-----------------configuration & control registers---------------
// see table 1-3
#define MCP23S08_ADDR_IODIR     0x00
#define MCP23S08_ADDR_IPOL      0x01
#define MCP23S08_ADDR_GPINTEN   0x02
#define MCP23S08_ADDR_DEFVAL    0x03
#define MCP23S08_ADDR_INTCON    0x04
#define MCP23S08_ADDR_IOCON     0x05
#define MCP23S08_ADDR_GPPU      0x06
#define MCP23S08_ADDR_INTF      0x07
#define MCP23S08_ADDR_INTCAP    0x08
#define MCP23S08_ADDR_GPIO      0x09
#define MCP23S08_ADDR_OLAT      0x0A


#endif /* SRC_MCP23S08_SUPPORT_H_ */

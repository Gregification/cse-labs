#include "MCP23008.h"

void MCP23008_write(const MCP23008_t * mcp,MCP23008_REG_t reg, uint8_t data) {
	if(!mcp || !mcp->spi)
		return;
	
	uint8_t tx[] = {
			0x40 | (mcp->addr_ext << 1),
			reg,
			data
	};
	
	SPI_transfer8(mcp->spi, tx, 0, 3, &mcp->cs);
}

uint8_t MCP23008_read(const MCP23008_t * mcp, MCP23008_REG_t reg) {
	if(!mcp || !mcp->spi)
		return 0;
	
	uint8_t tx[] = {
			0x40 | (mcp->addr_ext << 1) | 1,
			reg,
			0
	};
	
	SPI_transfer8(mcp->spi, tx, tx, 3, &mcp->cs);
	
	return tx[2];
}
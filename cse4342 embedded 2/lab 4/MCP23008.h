#include <stdint.h>
#include "common.h"

typedef enum _MCP23008_REG : uint8_t {
	REG_IODIR,
	REG_IPOL,
	REG_GPINTEN,
	REG_DEFVAL,
	REG_INTCON,
	REG_IOCON,
	REG_GPPU,
	REG_INTF,
	REG_INTCAP,
	REG_GPIO,
	REG_OLAT,
} MCP23008_REG_t;
_Static_assert(REG_OLAT == 0xA, "messed up register addresses");

typedef struct _MCP23008 {
	GPIO_Pin_t cs;
	SPI_TypeDef * spi;
	
	uint8_t addr_ext : 2;
} MCP23008_t;

void MCP23008_write(const MCP23008_t *,MCP23008_REG_t reg, uint8_t data);
uint8_t MCP23008_read(const MCP23008_t *, MCP23008_REG_t reg);

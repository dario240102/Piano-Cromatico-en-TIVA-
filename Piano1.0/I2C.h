#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

/**************************************************************************************************
 *  Variables de diagnostico
 */

extern volatile uint8_t i2c_error_debug;
extern volatile uint32_t i2c_timeout_debug;
extern volatile uint8_t i2c_last_mcs_debug;
extern volatile uint8_t i2c_probe_3c_debug;
extern volatile uint8_t i2c_probe_3d_debug;

/**************************************************************************************************
 *  Prototipos
 */

void I2C0_Init(void);

uint8_t I2C0_WriteByte(uint8_t slave_address, uint8_t data);
uint8_t I2C0_Write2Bytes(uint8_t slave_address, uint8_t byte1, uint8_t byte2);
uint8_t I2C0_Probe(uint8_t slave_address);

#endif /* I2C_H_ */

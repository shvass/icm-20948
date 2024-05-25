
#ifndef ICM_HPP
#define ICM_HPP


#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>

#define ICM_SPI_CLK_FREQ  (7 * 1000 * 1000)

#define ICM_SPI_ID SPI3_HOST 
#define ICM_CLK_DEFAULT GPIO_NUM_25
#define ICM_CS_DEFAULT GPIO_NUM_26
#define ICM_MISO_DEFAULT GPIO_NUM_33
#define ICM_MOSI_DEFAULT GPIO_NUM_32


/**
 * @brief icm class manages I/O and configuration for ICM-20948
 * 
 */
class icm {

public:

    icm(gpio_num_t MISO = ICM_MISO_DEFAULT, gpio_num_t MOSI = ICM_MOSI_DEFAULT, gpio_num_t CLK = ICM_CLK_DEFAULT, spi_host_device_t devId = ICM_SPI_ID);


    /**
     * @brief burst read 'size' data starting from given register 
     * 
     * @param bank address bank of ICM  
     * @param address to read from
     * @param buffer buffer to fill data into
     * @param size number of bytes to read
     */
    void read (char bank, char address, void* buffer, size_t size = 1);

    /**
     * @brief number of bytes to write 
     * 
     * @param bank address bank of ICM  
     * @param address to write to
     * @param buffer buffer to fill data from
     * @param size number of bytes to write
     */
    void write(char bank, char address, void* buffer, size_t size = 1);



// private:

    /**
     * @brief SPI I/O command to burst read number of bytes  
     * 
     * @param bank address bank of ICM  
     * @param address to read from
     * @param buffer buffer to fill data into
     * @param size number of bytes to read
     */
    void spiRead(char address, void* buffer, size_t size = 1);

    /**
     * @brief SPI I/O command to burst write number of bytes 
     * 
     * @param bank address bank of ICM  
     * @param address to write to
     * @param buffer buffer to fill data from
     * @param size number of bytes to write
     */
    void spiWrite(char address, void* buffer, size_t size = 1);
    

    spi_device_handle_t handle;

};




#endif //  ICM_HPP
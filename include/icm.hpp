
#ifndef ICM_HPP
#define ICM_HPP


#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>

#define ICM_SPI_CLK_FREQ  (7 * 1000 * 1000)

#define ICM_SPI_ID SPI3_HOST 


#define ICM_RX_BUF_SIZE 130

/**
 * @brief icm class manages I/O and configuration for ICM-20948
 * 
 */
class icm {

public:

    struct sensorData{
        float ax, ay, az;
        float gx, gy, gz;
        float temp;

    } data;
    

    
    icm(gpio_num_t MISO, gpio_num_t MOSI, gpio_num_t CLK, gpio_num_t CS, spi_host_device_t devId = ICM_SPI_ID);


    void update();


    void reset();
    void enable();

// private:

    void dump();
    
    void setBank(uint8_t bank);
    /**
     * @brief SPI I/O command to burst read number of bytes  
     * 
     * @param address to read from
     * @param buffer buffer to fill data into
     * @param size number of bytes to read
     */
    uint8_t* spiRead(char address, size_t size = 1);

    /**
     * @brief SPI I/O command to burst write number of bytes 
     * 
     * @param address to write to
     * @param buffer buffer to fill data from
     * @param size number of bytes to write
     */
    void spiWrite(char address, uint8_t* buffer, size_t size = 1);
    
    /**
     * @brief SPI I/O command to single byte read  
     * 
     * @param address to read from
     * @param byte byte to fill data into
     */
    uint8_t read(char address);

    /**
     * @brief SPI I/O command to single byte write 
     * 
     * @param address to write to
     * @param byte byte to fill data from
     */
    inline void write(char address, uint8_t byte) { spiWrite(address, &byte); };

    struct sensorDataRaw {
        int16_t ax, ay, az;
        int16_t gx, gy, gz;
        int16_t temp;
    } rawData;
    

    spi_device_handle_t handle;
    uint8_t bankId = 0;

    uint8_t rxBuffer[ICM_RX_BUF_SIZE];
    float accSens = m_accSens[0], gyroSens = m_gyroSens[0];
    static float m_accSens[4], m_gyroSens[4];
};




#endif //  ICM_HPP
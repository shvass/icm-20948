#include <stdio.h>

#include <cstring>

#include <driver/spi_master.h>

#include "icmRegisterMap.hpp"
#include "icm.hpp"




icm::icm(gpio_num_t MISO, gpio_num_t MOSI, gpio_num_t CLK , spi_host_device_t devId){

    // initialize SPI bus
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .mosi_io_num = MOSI,
        .miso_io_num = MISO,
        .sclk_io_num = CLK,
        .max_transfer_sz =  32,
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };

    spi_device_interface_config_t devcfg = {
        .mode = 0,                              
        .clock_speed_hz = ICM_SPI_CLK_FREQ, 
        .spics_io_num =  ICM_CS_DEFAULT,             
        .queue_size = 7,
    };


    ESP_ERROR_CHECK(ret = spi_bus_initialize(devId, &buscfg, SPI_DMA_CH_AUTO));

    // add SPI device
    ESP_ERROR_CHECK(ret = spi_bus_add_device(devId, &devcfg, &handle));


}

void icm::spiRead(char address, void *buffer, size_t size){

    spi_transaction_t trans;
    // trans.addr =  address;f
    trans.rx_buffer = buffer;
    trans.rxlength = (size) * 8;
    

    uint8_t txBuffer = ICM_READ_MASK | address;
    trans.tx_buffer = &txBuffer;
    trans.length = (size + 1) * 8;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &trans));
}

void icm::spiWrite(char address, void *buffer, size_t size){

    uint8_t txBuffer[size + 1];
    memcpy(txBuffer + 1, buffer, size);
    txBuffer[0] = ICM_WRITE_MASK & address;


    spi_transaction_t trans;
    trans.length = (size + 1) * 8;
    trans.rxlength = 0;
    trans.rx_buffer = 0;
    trans.tx_buffer = txBuffer;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &trans));
}

#include <stdio.h>

#include <cstring>

#include <driver/spi_master.h>

#include <esp_log.h>
#include "icmRegisterMap.hpp"
#include "icm.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG "ICM"


float icm::m_accSens[4] = {
    16384.0f,
    8192.0f,
    4096.0f,
    2048.0f    
};

float icm::m_gyroSens[4] = {
    131.0f,
    65.5f,
    32.8f,
    16.4f    
};


icm::icm(gpio_num_t MISO, gpio_num_t MOSI, gpio_num_t CLK , spi_host_device_t devId){

    // initialize SPI bus
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .mosi_io_num = MOSI,
        .miso_io_num = MISO,
        .sclk_io_num = CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz =  32,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = 0
    };


    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .clock_source = SPI_CLK_SRC_DEFAULT,
        .duty_cycle_pos = 128,
        .cs_ena_pretrans = 8,
        .cs_ena_posttrans = 8,
        .clock_speed_hz = ICM_SPI_CLK_FREQ, 
        .input_delay_ns = 59,
        .spics_io_num =  ICM_CS_DEFAULT,
        .flags = 0,
        .queue_size = 7,
        .pre_cb = 0,
        .post_cb = 0
    };

    ESP_ERROR_CHECK(ret = spi_bus_initialize(devId, &buscfg, SPI_DMA_CH_AUTO));

    // add SPI device
    ESP_ERROR_CHECK(ret = spi_bus_add_device(devId, &devcfg, &handle));


    // initialized ICM
    // check device id
    if(read(AGB0_REG_WHO_AM_I) != DEVICE_ID) ESP_LOGE(LOG, "ICM-20948 device Id does not match !!");

    reset();
    vTaskDelay(pdMS_TO_TICKS(50));
    enable();


    // start magnetometer here

    // set sample mode to continuous
    write(AGB0_REG_LP_CONFIG, 0x00);

    setBank(REG_BANK_2);
    // set full scale deflection
    write(AGB2_REG_ACCEL_CONFIG_1, 0x38);
    // write(AGB2_REG_ACCEL_CONFIG_2, 0x02);
    write(AGB2_REG_ACCEL_CONFIG_2, 0x00);

    // gyro
    write(AGB2_REG_GYRO_CONFIG_1, 0x38);
    // write(AGB2_REG_GYRO_CONFIG_2, 0x02);
    write(AGB2_REG_GYRO_CONFIG_2, 0x00);

    setBank(REG_BANK_0);

    dump();
}

void icm::update(){
    uint8_t* buffer = spiRead(AGB0_REG_ACCEL_XOUT_H, sizeof(sensorDataRaw));
    memcpy(&rawData, buffer, sizeof(sensorDataRaw));

    rawData.ax = read(AGB0_REG_ACCEL_XOUT_H) << 8 | read(AGB0_REG_ACCEL_XOUT_L);
    rawData.ay = read(AGB0_REG_ACCEL_YOUT_H) << 8 | read(AGB0_REG_ACCEL_YOUT_L);
    rawData.az = read(AGB0_REG_ACCEL_ZOUT_H) << 8 | read(AGB0_REG_ACCEL_ZOUT_L);

    rawData.gx = read(AGB0_REG_GYRO_XOUT_H) << 8 | read(AGB0_REG_GYRO_XOUT_L);
    rawData.gy = read(AGB0_REG_GYRO_YOUT_H) << 8 | read(AGB0_REG_GYRO_YOUT_L);
    rawData.gz = read(AGB0_REG_GYRO_ZOUT_H) << 8 | read(AGB0_REG_GYRO_ZOUT_L);

    rawData.temp = read(AGB0_REG_TEMP_OUT_H) << 8 | read(AGB0_REG_TEMP_OUT_L);

    data.ax = (float(~(rawData.ax + 1))) / accSens;
    data.ay = (float(~(rawData.ay + 1))) / accSens;
    data.az = (float(~(rawData.az + 1))) / accSens;

    data.gx = (float(rawData.gx)) / gyroSens;
    data.gy = (float(rawData.gy)) / gyroSens;
    data.gz = (float(rawData.gz)) / gyroSens;
    data.temp = (rawData.temp) / 333.87f + 21.0f;

}


void icm::reset(){
    uint8_t val = *spiRead(AGB0_REG_PWR_MGMT_1, 1);
    val |= (1 << 7);
    spiWrite(AGB0_REG_PWR_MGMT_1, &val);
}

void icm::enable(){
    // uint8_t val = *spiRead(AGB0_REG_PWR_MGMT_1, 1);
    // val &= 0x80;
    uint8_t val = 0;
    spiWrite(AGB0_REG_PWR_MGMT_1, &val);
}


void icm::dump(){
    for(char i = 0; i < 4; i++){
        setBank(i);
        uint8_t* buf = spiRead(0x00, 128);
        ESP_LOGI("ICM", "dumping register bank %d", i);
        ESP_LOG_BUFFER_HEX("ICM", buf, 128);
    }

    setBank(REG_BANK_0);
}

void icm::setBank(uint8_t bank)
{
    if(bank != bankId) {
        bank <<= 4;
        spiWrite(REG_BANK_SEL, &bank);
        bankId = bank;
    };
}

uint8_t *icm::spiRead(char address, size_t size)
{

    uint8_t txBuffer = ICM_READ_MASK | address;


    spi_transaction_t trans = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = (size + 1) * 8,
        .rxlength = (size + 1) * 8,
        .user = 0,
        .tx_buffer = &txBuffer,
        .rx_buffer = rxBuffer
    };
    
    ESP_ERROR_CHECK(spi_device_transmit(handle, &trans));

    return rxBuffer + 1;
}

void icm::spiWrite(char address, uint8_t *buffer, size_t size)
{

    uint8_t txBuffer[size + 1];
    memcpy(txBuffer + 1, buffer, size);
    txBuffer[0] = ICM_WRITE_MASK & address;


    spi_transaction_t trans = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = (size + 1) * 8,
        .rxlength = 0,
        .user = 0,
        .tx_buffer = txBuffer,
        .rx_buffer = 0
    };

    ESP_ERROR_CHECK(spi_device_transmit(handle, &trans));
}

uint8_t icm::read(char address)
{
    uint8_t ret = *spiRead(address);
    return ret;
}

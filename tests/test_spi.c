#include <hwmocker/hwmocker.h>
#include <hwmocker/irq.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SOC_SPI_IDX 4
#define HOST_SPI_IDX 0
#define XFER_SIZE 256
#define SOC_TX_PATTERN 0xaa
#define HOST_TX_PATTERN 0x55

struct async_spi_ctx {
    unsigned char *rxbuf;
    size_t size;
};

void check_rxbuf(unsigned char *rxbuf, unsigned char pattern, size_t size) {
    for (size_t idx = 0; idx < size; idx++) {
        if (rxbuf[idx] != pattern) {
            printf("%s failed at index %zu - got %x but expected %x\n", __func__, idx,
                   rxbuf[idx] & 0xff, pattern & 0xff);
            return;
        }
    }
    printf("%s(%zu bytes with %x) succeeded\n", __func__, size, pattern & 0xff);
}

int async_spi_callback(void *ctx) {
    struct async_spi_ctx *async_ctx = (struct async_spi_ctx *)ctx;
    check_rxbuf(async_ctx->rxbuf, HOST_TX_PATTERN, async_ctx->size);
    return 0;
}

int soc_main(void *priv) {
    struct hwmocker *mocker = *((struct hwmocker **)priv);
    void *soc = hwmocker_get_soc(mocker);
    void *spi_dev = hwmocker_get_spi_device(soc, SOC_SPI_IDX);
    unsigned char txbuf[XFER_SIZE];
    unsigned char rxbuf[XFER_SIZE] = {0};
    struct async_spi_ctx async_ctx = {.rxbuf = rxbuf, .size = XFER_SIZE};

    memset(txbuf, SOC_TX_PATTERN, XFER_SIZE);

    if (!spi_dev) {
        printf("%s - SPI device %d not found\n", __func__, SOC_SPI_IDX);
        return -1;
    }

    hwmocker_set_soc_ready(mocker);
    hwmocker_wait_host_ready(mocker);

    int rc = hwmocker_spi_xfer(spi_dev, txbuf, rxbuf, XFER_SIZE);
    printf("%s - hwmocker_spi_xfer returned %d\n", __func__, rc);
    check_rxbuf(rxbuf, HOST_TX_PATTERN, XFER_SIZE);

    memset(rxbuf, 0, XFER_SIZE);
    rc = hwmocker_spi_xfer_async(spi_dev, txbuf, rxbuf, XFER_SIZE, async_spi_callback, &async_ctx);
    printf("%s - hwmocker_spi_xfer returned %d\n", __func__, rc);
    check_rxbuf(rxbuf, HOST_TX_PATTERN, XFER_SIZE);

    /* wait 10ms here to let the signals to be handled */
    usleep(100000);
    return 0;
}

int host_main(void *priv) {
    struct hwmocker *mocker = *((struct hwmocker **)priv);
    void *host = hwmocker_get_host(mocker);
    void *spi_dev = hwmocker_get_spi_device(host, HOST_SPI_IDX);
    unsigned char txbuf[XFER_SIZE];
    unsigned char rxbuf[XFER_SIZE] = {0};

    memset(txbuf, HOST_TX_PATTERN, XFER_SIZE);

    if (!spi_dev) {
        printf("%s - SPI device %d not found\n", __func__, HOST_SPI_IDX);
        return -1;
    }

    hwmocker_set_host_ready(mocker);
    hwmocker_wait_soc_ready(mocker);

    /* wait 100ms so the soc is ready to xfer */
    usleep(10000);

    int rc = hwmocker_spi_xfer(spi_dev, txbuf, rxbuf, XFER_SIZE);
    printf("%s - hwmocker_spi_xfer returned %d\n", __func__, rc);
    check_rxbuf(rxbuf, SOC_TX_PATTERN, XFER_SIZE);

    usleep(100000);
    memset(rxbuf, 0, XFER_SIZE);
    rc = hwmocker_spi_xfer(spi_dev, txbuf, rxbuf, XFER_SIZE);
    printf("%s - hwmocker_spi_xfer returned %d\n", __func__, rc);
    check_rxbuf(rxbuf, SOC_TX_PATTERN, XFER_SIZE);

    /* wait 10ms here to let the signals to be handled */
    usleep(10000);
    return 0;
}

int main(int argc, char **argv) {
    struct hwmocker *mocker;
    int rc;

    if (argc != 2) {
        fprintf(stderr, "No config file given\n");
        fprintf(stderr, "Usage: %s <config JSON file>\n", argv[0]);
        return -EINVAL;
    }

    mocker = hwmocker_create(argv[1], host_main, &mocker, soc_main, &mocker);
    if (!mocker)
        return -ENOMEM;

    printf("hwmocker_create returned %p\n", (void *)mocker);

    rc = hwmocker_start(mocker);
    printf("hwmocker_start returned %d\n", rc);

    printf("hwmocker waiting for system to exit...\n");
    hwmocker_wait(mocker);

    printf("hwmocker cleaning up...\n");
    hwmocker_destroy(mocker);

    printf("That's all folks!!!\n");
    return 0;
}
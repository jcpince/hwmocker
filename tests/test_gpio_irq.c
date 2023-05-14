#include <hwmocker/hwmocker.h>
#include <hwmocker/irq.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define DEVICE_IRQ_NUMBER 17
#define HOST_IRQ_NUMBER 42

#if 0
struct device_data {
    struct hwmocker_device *device;
    struct hwmocker_irq_desc *irq_desc0;
    struct hwmocker *mocker;
};

int device_irq_handler(int irq_number, void *priv) {
    struct device_data *device_data = priv;
    puts("device_irq_handler called\n");
    return 0;
}

int device_main(void *priv) {
    struct device_data *device_data = priv;
    struct hwmocker_irq_desc *desc;

    int rc = hwmocker_irq_handler_register_handler(device_data->device->irq_handler,
                                                   DEVICE_IRQ_NUMBER, device_irq_handler,
                                                   device_data, HWMOCKER_IRQ_RISING_EDGE, 0);
    if (rc)
        return rc;

    desc = hwmocker_irq_handler_get_irq_desc(device_data->device->irq_handler, DEVICE_IRQ_NUMBER);

    if (!desc)
        return -ENOENT;

    return hwmocker_irq_handler_trigger_irq(desc);
}

struct host_data {
    struct hwmocker_host *host;
    struct hwmocker_irq_desc *irq_desc0;
    struct hwmocker *mocker;
};

int host_irq_handler(int irq_number, void *priv) {
    struct host_data *host_data = priv;
    puts("host_irq_handler called\n");
    return 0;
}

int host_main(void *priv) {
    struct host_data *host_data = priv;
    int rc = hwmocker_irq_handler_register_handler(host_data->host->irq_handler, HOST_IRQ_NUMBER,
                                                   host_irq_handler, host_data,
                                                   HWMOCKER_IRQ_RISING_EDGE, 0);
    if (rc)
        return rc;

    return 0;
}
#endif

int soc_main(void *priv) {
    printf("%s(%p) called\n", __func__, priv);
    sleep(1);
    return 0;
}

int host_main(void *priv) {
    printf("%s(%p) called\n", __func__, priv);
    sleep(1);
    return 0;
}

int main(int argc, char **argv) {
    // struct device_data device_data;
    // struct host_data host_data;
    struct hwmocker *mocker;
    int rc;

    if (argc != 2) {
        fprintf(stderr, "No config file given\n");
        fprintf(stderr, "Usage: %s <config JSON file>\n", argv[0]);
        return -EINVAL;
    }

    mocker = hwmocker_create(argv[1], host_main, NULL, soc_main, NULL);
    if (!mocker)
        return -ENOMEM;

    rc = hwmocker_start(mocker);
    printf("hwmocker_start returned %d\n", rc);

    printf("hwmocker waiting for system to exit...\n");
    hwmocker_wait(mocker);

    printf("hwmocker cleaning up...\n");
    hwmocker_destroy(mocker);

    printf("That's all folks!!!\n");
    return 0;
}
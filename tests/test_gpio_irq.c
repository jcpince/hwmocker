#include <hwmocker/hwmocker.h>
#include <hwmocker/irq.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define SOC_IRQ_NUMBER 5
#define HOST_IRQ_NUMBER 102
#define INVALID_PIN_NUMBER 1555

#define HOST2SOC_PIN(x) (x - 100)
#define SOC2HOST_PIN(x) (x + 100)

int soc_irq_triggered;
int host_irq_triggered;

int soc_irq_handler(void) {
    printf("%s() called\n", __func__);
    soc_irq_triggered++;
    return 0;
}

int soc_main(void *priv) {
    struct hwmocker *mocker = *((struct hwmocker **)priv);
    void *soc = hwmocker_get_soc(mocker);

    printf("%s(%p) called\n", __func__, (void *)mocker);
    /* Invalid gpio should fail with -EINVAL */
    int rc = hwmocker_set_gpio_irq_handler(soc, INVALID_PIN_NUMBER, soc_irq_handler);
    printf("%s: hwmocker_set_gpio_irq_handler(%d) returned %d\n", __func__, INVALID_PIN_NUMBER,
           rc);
    assert(rc == -EINVAL);

    rc = hwmocker_set_gpio_irq_handler(soc, SOC_IRQ_NUMBER, soc_irq_handler);
    printf("%s: hwmocker_set_gpio_irq_handler(%d) returned %d\n", __func__, SOC_IRQ_NUMBER, rc);

    // usleep(10000);
    hwmocker_set_soc_ready(mocker);
    hwmocker_wait_host_ready(mocker);

    hwmocker_set_gpio_level(soc, HOST2SOC_PIN(HOST_IRQ_NUMBER), 1);
    /* Invalid gpio should have no effect */
    hwmocker_set_gpio_level(soc, INVALID_PIN_NUMBER, 1);

    /* wait 10ms here to let the signals to be handled */
    usleep(10000);
    return 0;
}

int host_irq_handler(void) {
    printf("%s() called\n", __func__);
    host_irq_triggered++;
    return 0;
}

int host_main(void *priv) {
    struct hwmocker *mocker = *((struct hwmocker **)priv);
    void *host = hwmocker_get_host(mocker);

    printf("%s(%p) called\n", __func__, (void *)mocker);

    int rc = hwmocker_set_gpio_irq_handler(host, HOST_IRQ_NUMBER, host_irq_handler);
    printf("%s: hwmocker_set_gpio_irq_handler(%d) returned %d\n", __func__, HOST_IRQ_NUMBER, rc);

    // usleep(10000);
    hwmocker_set_host_ready(mocker);
    hwmocker_wait_soc_ready(mocker);

    hwmocker_set_gpio_level(host, SOC2HOST_PIN(SOC_IRQ_NUMBER), 1);
    /* Invalid gpio should have no effect */
    hwmocker_set_gpio_level(host, INVALID_PIN_NUMBER, 1);

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

    assert(soc_irq_triggered == 1);
    assert(host_irq_triggered == 1);

    printf("That's all folks!!!\n");
    return 0;
}
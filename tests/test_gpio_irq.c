#include <hwmocker/hwmocker.h>
#include <hwmocker/irq.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define SOC_IRQ_NUMBER 17
#define HOST_IRQ_NUMBER 42

int soc_irq_handler(void) {
    printf("%s() called\n", __func__);
    return 0;
}

int soc_main(void *priv) {
    struct hwmocker *mocker = *((struct hwmocker **)priv);
    void *soc = hwmocker_get_soc(mocker);

    printf("%s(%p) called\n", __func__, (void *)mocker);
    int rc = hwmocker_set_gpio_irq_handler(soc, SOC_IRQ_NUMBER, soc_irq_handler);
    printf("%s: hwmocker_set_gpio_irq_handler(%d) returned %d\n", __func__, SOC_IRQ_NUMBER, rc);

    rc = hwmocker_set_gpio_irq_handler(soc, 5, soc_irq_handler);
    printf("%s: hwmocker_set_gpio_irq_handler(%d) returned %d\n", __func__, 5, rc);

    sleep(1);
    return 0;
}

int host_irq_handler(void) { return 0; }

int host_main(void *priv) {
    struct hwmocker *mocker = *((struct hwmocker **)priv);
    void *host = hwmocker_get_host(mocker);

    printf("%s(%p) called\n", __func__, (void *)mocker);

    sleep(1);
    hwmocker_set_gpio_level(host, 105, 1);
    sleep(1);
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
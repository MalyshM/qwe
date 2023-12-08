#include <stdio.h>
#include <mosquitto.h>
#include <sys/time.h>
#include <string.h>
#define TOPIC       "test/t1"
#define QOS         0
int main()
{
    int rc;
    struct mosquitto *mosq;

    mosquitto_lib_init();

    mosq = mosquitto_new("publisher-test", true, NULL);

    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
    if (rc != 0)
    {
        printf("Client could not connect to broker! Error Code: %d\n", rc);
        mosquitto_destroy(mosq);
        return -1;
    }
    printf("We are now connected to the broker!\n");
    for (int i = 0; i < 100; i++) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        char payload[100];
        snprintf(payload, sizeof(payload), "%ld.%06ld", tv.tv_sec, tv.tv_usec);
        mosquitto_publish(mosq, NULL, TOPIC, strlen(payload), payload, QOS, false);
    }

    // mosquitto_publish(mosq, NULL, "test/t1", 6, "Hello", 0, false);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();
    return 0;
}
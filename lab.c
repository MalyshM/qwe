#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mosquitto.h>

#define ADDRESS     "tcp://broker.hivemq.com:1883"
#define CLIENTID_PUB    "Publisher"
#define CLIENTID_SUB    "Subscriber"
#define TOPIC       "mytopic"
#define QOS         1
#define TIMEOUT     10000L

volatile int delivered = 0;

void message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    time_t now;
    char payload[100];

    now = time(NULL);
    strftime(payload, sizeof(payload), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("Message arrived at %s: %s\n", payload, (char*)message->payload);
}

void connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    if (result == 0) {
        printf("Publisher connected to broker successfully.\n");
    } else {
        fprintf(stderr, "Publisher connection failed.\n");
        exit(EXIT_FAILURE);
    }
}

void publish_callback(struct mosquitto *mosq, void *userdata, int mid)
{
    printf("Message with ID %d delivered.\n", mid);
    delivered = 1;
}

int main(int argc, char* argv[])
{
    struct mosquitto *mosq_pub, *mosq_sub;
    int rc_pub, rc_sub;

    mosquitto_lib_init();

    mosq_pub = mosquitto_new(CLIENTID_PUB, true, NULL);
    mosq_sub = mosquitto_new(CLIENTID_SUB, true, NULL);

    if (!mosq_pub || !mosq_sub) {
        fprintf(stderr, "Error: Out of memory.\n");
        return EXIT_FAILURE;
    }

    mosquitto_connect_callback_set(mosq_pub, connect_callback);
    mosquitto_connect_callback_set(mosq_sub, connect_callback);
    mosquitto_message_callback_set(mosq_sub, message_callback);
    mosquitto_publish_callback_set(mosq_pub, publish_callback);

    rc_pub = mosquitto_connect(mosq_pub, ADDRESS, 1883, 20);
    rc_sub = mosquitto_connect(mosq_sub, ADDRESS, 1883, 20);

    if (rc_pub == MOSQ_ERR_SUCCESS && rc_sub == MOSQ_ERR_SUCCESS)
    {
        mosquitto_subscribe(mosq_sub, NULL, TOPIC, QOS);
        
        char input[100];
        while (1) {
            printf("Enter a message to publish (or 'q' to quit): ");
            fgets(input, sizeof(input), stdin);

            if (input[0] == 'q') {
                break;
            }

            delivered = 0;
            rc_pub = mosquitto_publish(mosq_pub, NULL, TOPIC, strlen(input), input, QOS, false);

            if (rc_pub != MOSQ_ERR_SUCCESS) {
                fprintf(stderr, "Error publishing message: %s\n", mosquitto_strerror(rc_pub));
                exit(EXIT_FAILURE);
            }

            while (!delivered) {
                rc_pub = mosquitto_loop(mosq_pub, 100, 1);
                if (rc_pub != MOSQ_ERR_SUCCESS) {
                    fprintf(stderr, "Error in the publisher loop: %s\n", mosquitto_strerror(rc_pub));
                    exit(EXIT_FAILURE);
                }
            }

        }

        mosquitto_unsubscribe(mosq_sub, NULL, TOPIC);
    }
    else
    {
        fprintf(stderr, "Failed to connect to broker: %s\n", mosquitto_strerror(rc_pub));
        exit(EXIT_FAILURE);
    }

    mosquitto_disconnect(mosq_pub);
    mosquitto_destroy(mosq_pub);
    mosquitto_disconnect(mosq_sub);
    mosquitto_destroy(mosq_sub);
    mosquitto_lib_cleanup();

    return EXIT_SUCCESS;
}
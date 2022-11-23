#include <iostream>
#include "stdio.h"
#include "zenohcpp.h"

using namespace zenoh;

int main(int argc, char **argv) {
    const char *keyexpr = "demo/example/zenoh-c-put";
    const char *value = "Put from CPP!";

    if (argc > 1) keyexpr = argv[1];
    if (argc > 2) value = argv[2];

    Config config;
    if (argc > 3) {
        if (config.insert_json(Z_CONFIG_CONNECT_KEY, argv[3]) < 0) {
            printf(
                "Couldn't insert value `%s` in configuration at `%s`. This is likely because `%s` expects a "
                "JSON-serialized list of strings\n",
                argv[3], Z_CONFIG_CONNECT_KEY, Z_CONFIG_CONNECT_KEY);
            exit(-1);
        }
    }

    printf("Opening session...\n");
    auto session = std::get<Session>(open(std::move(config)));

    printf("Putting Data ('%s': '%s')...\n", keyexpr, value);
    PutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);

    if (!session.put(keyexpr, value, options)) {
        printf("Put failed...\n");
    }

    return 0;
}
